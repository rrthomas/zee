-- Redisplay engine
--
-- Copyright (c) 2009-2012 Free Software Foundation, Inc.
--
-- This file is part of Zee.
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

-- Tidy up the term ready to exit (temporarily or permanently!).
function term_tidy ()
  term_move (term_height () - 1, 0)
  term_clrtoeol ()
  term_attrset (display.normal)
  term_refresh ()
end

-- Tidy and close the terminal ready to exit.
function term_finish ()
  term_tidy ()
  term_close ()
end

local function make_char_printable (c, x, cur_tab_width)
  assert (c ~= "")
  if c == '\t' then
    return string.rep (" ", cur_tab_width - x % cur_tab_width)
  elseif string.byte (c) > 0 and string.byte (c) <= 27 then
    return string.format ("^%c", string.byte ("@") + string.byte (c))
  else
    return string.format ("\\%o", string.byte (c))
  end
end

local function draw_line (line, startcol, wp, o, rp, highlight, cur_tab_width)
  term_move (line, 0)

  -- Draw body of line.
  local x = 0
  local line_len = buffer_line_len (cur_bp, o)
  for i = startcol, math.huge do
    term_attrset ((highlight and in_region (o, i, rp)) and display.reverse or display.normal)
    if i >= line_len or x >= wp.ewidth then
      break
    end
    local c = get_buffer_char (cur_bp, o + i)
    if posix.isprint (c) then
      term_addstr (c)
      x = x + 1
    else
      local s = make_char_printable (c, x, cur_tab_width)
      term_addstr (s)
      x = x + #s
    end
  end

  -- Draw end of line.
  if x >= term_width () then
    term_move (line, term_width () - 1)
    term_attrset (display.normal)
    term_addstr ('$')
  else
    term_addstr (string.rep (" ", wp.ewidth - x))
  end
  term_attrset (display.normal)
end

local function calculate_highlight_region ()
  if cur_bp.mark == nil or not cur_bp.mark_active then
    return false
  end

  return true, region_new (get_buffer_pt (cur_bp), cur_bp.mark.o)
end

function make_modeline_flags ()
  if cur_bp.modified and cur_bp.readonly then
    return "%*"
  elseif cur_bp.modified then
    return "**"
  elseif cur_bp.readonly then
    return "%%"
  end
  return "--"
end

local function make_screen_pos (wp)
  local tv = window_top_visible (wp)
  local bv = window_bottom_visible (wp)

  if tv and bv then
    return "All"
  elseif tv then
    return "Top"
  elseif bv then
    return "Bot"
  end
  return string.format ("%2d%%", (get_buffer_pt (cur_bp) / get_buffer_size (cur_bp)) * 100)
end

local function draw_border ()
  term_attrset (display.reverse)
  term_addstr (string.rep ('-', cur_wp.ewidth))
  term_attrset (display.normal)
end

local function draw_status_line (line, wp)
  term_move (line, 0)
  draw_border()

  term_attrset (display.reverse)
  term_move (line, 0)
  local n = offset_to_line (cur_bp, get_buffer_pt (cur_bp))
  local as = string.format ("--%2s  %-15s   %s %-9s (",
                            make_modeline_flags (), cur_bp.name, make_screen_pos (wp),
                            string.format ("(%d,%d)", n + 1, get_goalc ()))

  if cur_bp.autofill then
    as = as .. " Fill"
  end
  if thisflag.defining_macro then
    as = as .. " Def"
  end
  if cur_bp.isearch then
    as = as .. " Isearch"
  end
  as = as .. ")"

  term_addstr (string.sub (as, 1, term_width ()))
  term_attrset (display.normal)
end

local function draw_window (topline, wp)
  local highlight, rp = calculate_highlight_region ()

  -- Find the first line to display on the first screen line.
  local o = buffer_start_of_line (cur_bp, get_buffer_pt (cur_bp))
  local i = wp.topdelta
  while i > 0 and o > 1 do
    o = buffer_prev_line (cur_bp, o)
    assert (o)
    i = i - 1
  end

  -- Draw the window lines.
  local cur_tab_width = tab_width ()
  for i = topline, wp.eheight + topline do
    -- Clear the line.
    term_move (i, 0)
    term_clrtoeol ()

    -- If at the end of the buffer, don't write any text.
    if o ~= nil then
      draw_line (i, wp.start_column, wp, o, rp, highlight, cur_tab_width)

      if wp.start_column > 0 then
        term_move (i, 0)
        term_addstr ('$')
      end

      o = buffer_next_line (cur_bp, o)
    end
  end

  wp.all_displayed = o == nil or o == get_buffer_size (cur_bp)

  -- Draw the status line only if there is available space after the
  -- buffer text space.
  if wp.fheight > wp.eheight then
    draw_status_line (topline + wp.eheight, wp)
  end
end

local cur_topline, col = 0, 0



-- Popup

-- Contents of popup window.
local popup_text
local popup_line = 0

-- Set the popup string to `s'.
function popup_set (s)
  popup_text = s and AStr (s:chomp ())
  popup_line = 0
end

-- Clear the popup string.
function popup_clear ()
  popup_set ()
end

-- Scroll the popup text and loop having reached the bottom.
function popup_scroll_down_and_loop ()
  popup_line = popup_line + cur_wp.fheight - 3
  if popup_line > popup_text:lines () then
    popup_line = 0
  end
  term_redisplay ()
end

-- Scroll down the popup text.
function popup_scroll_down ()
  local h = cur_wp.fheight - 3
  popup_line = math.min (popup_line + h, popup_text:lines () + 1 - h)
  term_display ()
end

-- Scroll up the popup text.
function popup_scroll_up ()
  popup_line = popup_line - math.min (win.fheight - 3, popup_line)
  term_display ()
end

-- Draw the popup window.
local function draw_popup ()
  assert (popup_text)

  -- Number of lines of popup_text that will fit on the terminal.
  -- Allow 3 for the border above, and minibuffer and status line below.
  local h = cur_wp.fheight - 3
  -- Number of lines.
  local l = popup_text:lines ()
  -- Position of top of popup == number of lines not to use.
  local y = math.max (h - l, 0)

  term_move (y, 0)
  draw_border ()

  -- Draw popup text, and blank lines to bottom of window.
  local o = 1
  for l = 0, popup_line do
    o = popup_text:next_line (o)
  end
  for i = 1, h - y + 1 do
    if o then
      term_addstr (tostring (popup_text:sub (o, popup_text:end_of_line (o)))) -- FIXME
      o = popup_text:next_line (o)
    end
    term_clrtoeol ()
  end
end

function term_redisplay ()
  -- Calculate the start column if the line at point has to be truncated.
  local lastcol, t = 0, tab_width ()
  local o = get_buffer_pt (cur_bp)
  local lineo = o - get_buffer_line_o (cur_bp)

  col = 0
  o = o - lineo
  cur_wp.start_column = 0

  local ew = cur_wp.ewidth
  for lp = lineo, 0, -1 do
    col = 0
    for p = lp, lineo - 1 do
      local c = get_buffer_char (cur_bp, o + p)
      if posix.isprint (c) then
        col = col + 1
      else
        col = col + #make_char_printable (c, col, t)
      end
    end

    if col >= ew - 1 or (lp / (ew / 3) + 2 < lineo / (ew / 3)) then
      cur_wp.start_column = lp + 1
      col = lastcol
      break
    end

    lastcol = col
  end

  -- Draw the window.
  local topline = 0
  cur_topline = topline
  draw_window (topline, cur_wp)
  topline = topline + cur_wp.fheight

  -- Draw the popup window.
  if popup_text then
    draw_popup ()
  end

  term_redraw_cursor ()
end

function term_redraw_cursor ()
  term_move (cur_topline + cur_wp.topdelta, col)
end

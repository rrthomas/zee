-- Redisplay engine
--
-- Copyright (c) 2009-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

-- Tidy up the term ready to leave Zile (temporarily or permanently!).
function term_tidy ()
  term_move (term_height () - 1, 0)
  term_clrtoeol ()
  term_attrset (display.normal)
  term_refresh ()
end

-- Tidy and close the terminal ready to leave Zile.
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
  local line_len = buffer_line_len (wp.bp, o)
  for i = startcol, math.huge do
    term_attrset ((highlight and in_region (o, i, rp)) and display.reverse or display.normal)
    if i >= line_len or x >= wp.ewidth then
      break
    end
    local c = get_buffer_char (wp.bp, o + i)
    if posix.isprint (c) then
      term_addch (string.byte (c))
      x = x + 1
    else
      local s = make_char_printable (get_buffer_char (wp.bp, o + i), x, cur_tab_width)
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

local function calculate_highlight_region (wp)
  if (wp ~= cur_wp and not get_variable_bool ("highlight-nonselected-windows"))
    or wp.bp.mark == nil
    or not wp.bp.mark_active then
    return false
  end

  return true, region_new (window_o (wp), wp.bp.mark.o)
end

function make_modeline_flags (wp)
  if wp.bp.modified and wp.bp.readonly then
    return "%*"
  elseif wp.bp.modified then
    return "**"
  elseif wp.bp.readonly then
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
  return string.format ("%2d%%", (window_o (wp) / get_buffer_size (wp.bp)) * 100)
end

local function draw_status_line (line, wp)
  local n = offset_to_line (wp.bp, window_o (wp))
  term_attrset (display.reverse)
  term_move (line, 0)
  term_addstr (string.rep ('-', wp.ewidth))

  local eol_type
  if get_buffer_eol (cur_bp) == coding_eol_cr then
    eol_type = "(Mac)"
  elseif get_buffer_eol (cur_bp) == coding_eol_crlf then
    eol_type = "(DOS)"
  else
    eol_type = ":"
  end

  term_move (line, 0)

  local as = string.format ("--%s%2s  %-15s   %s %-9s (Fundamental",
                            eol_type, make_modeline_flags (wp), wp.bp.name, make_screen_pos (wp),
                            string.format ("(%d,%d)", n + 1, get_goalc_bp (wp.bp, window_o (wp))))

  if wp.bp.autofill then
    as = as .. " Fill"
  end
  if thisflag.defining_macro then
    as = as .. " Def"
  end
  if wp.bp.isearch then
    as = as .. " Isearch"
  end
  as = as .. ")"

  term_addstr (string.sub (as, 1, term_width ()))
  term_attrset (display.normal)
end

local function draw_window (topline, wp)
  local highlight, rp = calculate_highlight_region (wp)

  -- Find the first line to display on the first screen line.
  local o = buffer_start_of_line (wp.bp, window_o (wp))
  local i = wp.topdelta
  while i > 0 and o > 0 do
    o = buffer_prev_line (wp.bp, o)
    assert (o)
    i = i - 1
  end

  -- Draw the window lines.
  local cur_tab_width = tab_width (wp.bp)
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

      o = buffer_next_line (wp.bp, o)
    end
  end

  wp.all_displayed = o == nil or o == get_buffer_size (wp.bp)

  -- Draw the status line only if there is available space after the
  -- buffer text space.
  if wp.fheight > wp.eheight then
    draw_status_line (topline + wp.eheight, wp)
  end
end

local cur_topline, col = 0, 0

function term_redisplay ()
-- Calculate the start column if the line at point has to be truncated.
  local lastcol, t = 0, tab_width (cur_wp.bp)
  local o = window_o (cur_wp)
  local lineo = o - get_buffer_line_o (cur_wp.bp)

  col = 0
  o = o - lineo
  cur_wp.start_column = 0

  local ew = cur_wp.ewidth
  for lp = lineo, 0, -1 do
    col = 0
    for p = lp, lineo - 1 do
      local c = get_buffer_char (cur_wp.bp, o + p)
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

  -- Draw the windows.
  local topline = 0
  cur_topline = topline
  for _, wp in ipairs (windows) do
    if wp == cur_wp then
      cur_topline = topline
    end

    draw_window (topline, wp)

    topline = topline + wp.fheight
  end

  term_redraw_cursor ()
end

function term_redraw_cursor ()
  term_move (cur_topline + cur_wp.topdelta, col)
end

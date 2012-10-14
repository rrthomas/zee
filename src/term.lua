-- Display engine
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

-- Window table:
-- {
--   topdelta: The top line delta from point.
--   lastpointn: The last point line number.
--   fwidth, fheight: The formal width and height of the window.
--   ewidth, eheight: The effective width and height of the window.
-- }

function window_top_visible (wp)
  return offset_to_line (buf, get_buffer_pt (buf)) == wp.topdelta
end

function window_bottom_visible (wp)
  return wp.all_displayed
end

function window_resync (wp)
  local n = offset_to_line (buf, get_buffer_pt (buf))
  local delta = n - wp.lastpointn

  if delta ~= 0 then
    if (delta > 0 and wp.topdelta + delta < wp.eheight) or (delta < 0 and wp.topdelta >= -delta) then
      wp.topdelta = wp.topdelta + delta
    elseif n > wp.eheight / 2 then
      wp.topdelta = math.floor (wp.eheight / 2)
    else
      wp.topdelta = n
    end
    wp.lastpointn = n
  end
end

-- FIXME: rename
Define ("move-redraw",
[[
Redraw the display.
]],
  function ()
    term_clear ()
    term_display ()
    term_refresh ()
  end
)

function resize_window ()
  -- Resize window horizontally.
  win.fwidth = term_width ()
  win.ewidth = win.fwidth

  -- Resize window vertically.
  win.fheight = term_height ()
  win.eheight = win.fheight
  win.eheight = win.eheight - math.min (win.eheight, 2)
end

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
  elseif c:byte () > 0 and c:byte () <= 27 then
    return string.format ("^%c", string.byte ("@") + c:byte ())
  else
    return string.format ("\\%o", c:byte ())
  end
end

-- Prints a line on the terminal.
--
-- line - the line number within the buffer
-- startcol - the horizontal scroll offset
-- o - the starting offset of the line
-- rp - the highlight rectangle, or nil if none
-- cur_tab_width - the display width of a tab character
--
-- If any part of the line is off the left-hand side of the screen,
-- prints a `$' character in the left-hand column. If any part is off
-- the right, prints a `$' character in the right-hand column. Any
-- part that is within the highlight region is highlighted. If the
-- final position is within the highlight region then the remainder of
-- the line is also highlighted.
local function draw_line (line, startcol, o, rp, cur_tab_width)
  term_move (line, 0)

  -- Draw body of line.
  local x = 0
  local line_len = buffer_line_len (buf, o)
  for i = startcol, math.huge do
    if i >= line_len or x >= win.ewidth then
      break
    end
    term_attrset ((rp and in_region (o, i, rp)) and display.reverse or display.normal)
    local c = get_buffer_char (buf, o + i)
    if posix.isprint (c) then
      term_addstr (c)
      x = x + 1
    else
      local s = make_char_printable (c, x, cur_tab_width)
      term_addstr (s:sub (1, math.min (#s, win.fwidth - x)))
      x = x + #s
    end
  end

  term_addstr (string.rep (" ", win.fwidth - x))
  term_attrset (display.normal)

  -- Draw end of line.
  if x >= win.fwidth then
    term_move (line, win.fwidth - 1)
    term_addstr ('$')
  end
end

local function calculate_highlight_region ()
  if buf.mark == nil then
    return nil
  end
  return region_new (get_buffer_pt (buf), buf.mark.o)
end

local function draw_border ()
  term_attrset (display.reverse)
  term_addstr (string.rep ('-', win.ewidth))
  term_attrset (display.normal)
end

local function draw_status_line (line, wp)
  term_move (line, 0)
  draw_border()

  term_attrset (display.reverse)
  term_move (line, 0)
  local n = offset_to_line (buf, get_buffer_pt (buf))

  local as = "--"

  -- Buffer state flags
  if buf.modified and buf.readonly then
    as = as .. "%*"
  elseif buf.modified then
    as = as .. "**"
  elseif buf.readonly then
    as = as .. "%%"
  else
    as = as .. "--"
  end

  -- File name
  as = as .. string.format ("  %-15s   ", buf.filename)

  -- Percentage of the way through the file
  as = as .. string.format ("%3d%%", (get_buffer_pt (buf) / get_buffer_size (buf)) * 100)

  -- Coordinates
  as = as .. string.format (" %-9s (", string.format ("(%d,%d)", n + 1, get_goalc ()))

  -- Mode flags
  local flags = {}
  if thisflag.defining_macro then
    table.insert (flags, "Def")
  end
  if buf.search then
    table.insert (flags, "Search")
  end
  as = as .. table.concat (flags, " ") .. ")"

  -- Display status line
  term_addstr (as:sub (1, term_width ()))
  term_attrset (display.normal)
end

local start_column = 0 -- start column of the window (>0 if scrolled sideways).
local cursor_screen_column = 0 -- screen column of cursor

local function draw_window (topline, wp)
  local rp = calculate_highlight_region ()

  -- Find the first line to display on the first screen line.
  local o = buffer_start_of_line (buf, get_buffer_pt (buf))
  local i = wp.topdelta
  while i > 0 and o > 1 do
    o = buffer_prev_line (buf, o)
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
      draw_line (i, start_column, o, rp, cur_tab_width)

      if start_column > 0 then
        term_move (i, 0)
        term_addstr ('$')
      end

      o = buffer_next_line (buf, o)
    end
  end

  wp.all_displayed = o == nil or o == get_buffer_size (buf)

  -- Draw the status line only if there is available space after the
  -- buffer text space.
  if wp.fheight > wp.eheight then
    draw_status_line (topline + wp.eheight, wp)
  end
end


-- Popup

-- Contents of popup window.
local popup_text
local popup_line = 0

-- Set the popup string to `s'.
function popup_set (s)
  popup_text = s and AStr (s:chomp ()) -- FIXME: stop needing chomp for docstrings
  popup_line = 0
end

-- Clear the popup string.
function popup_clear ()
  popup_set ()
end

-- Scroll the popup text and loop having reached the bottom.
function popup_scroll_down_and_loop ()
  popup_line = popup_line + win.fheight - 3
  if popup_line > popup_text:lines () then
    popup_line = 0
  end
  term_display ()
end

-- Scroll down the popup text.
function popup_scroll_down ()
  local h = win.fheight - 3
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
  local h = win.fheight - 3
  -- Number of lines.
  local l = popup_text:lines ()
  -- Position of top of popup == number of lines not to use.
  local y = math.max (h - l - 1, 0)

  term_move (y, 0)
  draw_border ()

  -- Draw popup text, and blank lines to bottom of window.
  local o = 1
  for l = 0, popup_line - 1 do
    o = popup_text:next_line (o)
  end
  for i = 1, h - y + 1 do
    if o then
      term_addstr (tostring (popup_text:sub (o, popup_text:end_of_line (o))))
      o = popup_text:next_line (o)
    end
    term_clrtoeol ()
  end
end

-- Scans `s' and replaces each character with a string of one or
-- more printable characters.
--
-- Scanning stops when the screen column reaches or exceeds `goal',
-- or when `s' is exhausted. The number of input characters
-- scanned is returned as a second return value. If you don't
-- want a `goal', just pass `math.huge'.
--
-- Characters that are already printable expand to themselves.
-- Characters from 0 to 26 are replaced with strings from `^@' to
-- `^Z'. Tab characters are replaced with enough spaces (but always
-- at least one) to reach a screen column that is a multiple of `tab'.
-- Newline characters must not occur in `s'. Other characters are
-- replaced with a backslash followed by their hex character code.
function make_string_printable (s, goal)
  goal = goal or math.huge

  local ctrls = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  local tab = tab_width ()
  local ret, pos = "", 0
  for i = 1, #s do
    local c = s[i]
    assert (c ~= '\n')

    local x = #ret
    if x >= goal then
      break
    end

    if c == '\t' then
      ret = ret .. string.rep (' ', tab - (x % tab))
    elseif c < #ctrls then
      ret = ret .. '^' .. ctrls[c + 1]
    elseif posix.isprint (string.char (c)) then
      ret = ret .. string.char (c)
      -- FIXME: For double-width characters add a '\0' too so the length of
      -- 'ret' matches the display width.
    else
      ret = ret .. '\\' ..  string.format ("%x", c)
    end

    pos = pos + 1
  end

  return ret
end

-- Calculate start_column and cursor_screen_column.
--
-- `start_column' is always a multiple of a third of a screen width. It is
-- chosen so as to put the cursor in the middle third, unless the cursor is near
-- one or other end of the line, in which case it is chosen to show as much of
-- the line as possible.
local function calculate_start_column ()
  local width = win.ewidth
  local third_width = math.max (1, math.floor (width / 3))

  -- Calculate absolute columns of cursor and end of line.
  local x = get_goalc ()
  local length = #make_string_printable (get_line ())

  -- Choose start_column.
  if x < third_width or length < width then
    -- No-brainer cases: show left-hand end of line.
    start_column = 0
  else
    -- Put cursor in the middle third.
    start_column = x - (x % third_width) - third_width
    -- But scroll left if the right-hand end of the line stays on the screen.
    while start_column + width >= length + third_width do
      start_column = start_column - third_width
    end
  end

  -- Consequently, calculate screen-relative column.
  cursor_screen_column = x - start_column
end

function term_display ()
  -- Calculate the start column if the line at point has to be truncated.
  local lastcol, t = 0, tab_width ()
  local o = get_buffer_pt (buf)
  local lineo = o - get_buffer_line_o (buf)

  calculate_start_column ()

  draw_window (0, win)

  if popup_text then
    draw_popup ()
  end

  term_redraw_cursor ()
end

function term_redraw_cursor ()
  term_move (win.topdelta, cursor_screen_column)
end

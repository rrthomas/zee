-- Window handling functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
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

-- The window list
windows = {}

-- Window table:
-- {
--   next: The next window in window list.
--   bp: The buffer displayed in window.
--   topdelta: The top line delta from point.
--   lastpointn: The last point line number.
--   start_column: The start column of the window (>0 if scrolled sideways).
--   saved_pt: The point line pointer, line number and offset
--             used to hold the point in non-current windows).
--   fwidth, fheight: The formal width and height of the window.
--   ewidth, eheight: The effective width and height of the window.
-- }

local function window_new ()
  return {topdelta = 0, start_column = 0, lastpointn = 0}
end

-- Set the current window and its buffer as the current buffer.
function set_current_window (wp)
  -- Save buffer's point in a new marker.
  if cur_wp.saved_pt then
    unchain_marker (cur_wp.saved_pt)
  end

  cur_wp.saved_pt = point_marker ()
  cur_wp = wp
  cur_bp = wp.bp

  -- Update the buffer point with the window's saved point marker.
  if cur_wp.saved_pt then
    goto_offset (cur_wp.saved_pt.o)
    unchain_marker (cur_wp.saved_pt)
    cur_wp.saved_pt = nil
  end
end

function find_window (name)
  for _, wp in ipairs (windows) do
    if wp.bp.name == name then
      return wp
    end
  end
end

function window_o (wp)
  -- The current window uses the current buffer point; all other
  -- windows have a saved point, except that if a window has just been
  -- killed, it needs to use its new buffer's current point.

  assert (wp ~= nil)
  if wp == cur_wp then
    assert (wp.bp == cur_bp)
    assert (wp.saved_pt == nil)
    assert (cur_bp ~= nil)
    return get_buffer_pt (cur_bp)
  else
    if wp.saved_pt then
      return wp.saved_pt.o
    else
      return get_buffer_pt (wp.bp)
    end
  end
end

local function window_prev (this_wp)
  for i, wp in ipairs (windows) do
    if wp == this_wp then
      if i < #windows then
        return windows[i + 1]
      elseif i > 1 then
        return windows[i - 1]
      end
      break
    end
  end
  return windows[1]
end

local function window_next (this_wp)
  for i, wp in ipairs (windows) do
    if wp == this_wp then
      if i > 1 then
        return windows[i - 1]
      elseif i < #windows then
        return windows[i + 1]
      end
      break
    end
  end
  return windows[1]
end

function delete_window (del_wp)
  for i = 1, #windows do
    local wp = windows[i]
    if wp == del_wp then
      local next_wp = window_prev (wp)
      table.remove (windows, i)
      if next_wp then
        next_wp.fheight = next_wp.fheight + del_wp.fheight
        next_wp.eheight = next_wp.eheight + del_wp.eheight + 1
        set_current_window (next_wp)
      end
      break
    end
  end

  if del_wp.saved_pt then
    unchain_marker (del_wp.saved_pt)
  end
end

Defun ("delete-window",
       {},
[[
Remove the current window from the screen.
]],
  true,
  function ()
    if #windows == 1 then
      minibuf_error ("Attempt to delete sole ordinary window")
      return false
    end

    delete_window (cur_wp)
  end
)

Defun ("enlarge-window",
       {},
[[
Make current window one line bigger.
]],
  true,
  function ()
    if #windows == 1 then
      return false
    end

    local wp = cur_wp.next
    if not wp or wp.fheight < 3 then
      for _, wp in ipairs (windows) do
        if wp.next == cur_wp then
          if wp.fheight < 3 then
            return false
          end
          break
        end
      end

      if cur_wp == windows[#windows] and cur_wp.next.fheight < 3 then
        return false
      end

      wp.fheight = wp.fheight - 1
      wp.eheight = wp.eheight - 1
      if wp.topdelta >= wp.eheight then
        recenter (wp)
      end
      cur_wp.fheight = cur_wp.fheight + 1
      cur_wp.eheight = cur_wp.eheight + 1
    end
  end
)

Defun ("shrink-window",
       {},
[[
Make current window one line smaller.
]],
  true,
  function ()
    if #windows == 1 or cur_wp.fheight < 3 then
      return false
    end

    local next_wp = window_next (cur_wp)
    next_wp.fheight = next_wp.fheight + 1
    next_wp.eheight = next_wp.eheight + 1
    cur_wp.fheight = cur_wp.fheight - 1
    cur_wp.eheight = cur_wp.eheight - 1
    if cur_wp.topdelta >= cur_wp.eheight then
      recenter (next_wp)
    end
  end
)

Defun ("delete-other-windows",
       {},
[[
Make the selected window fill the screen.
]],
  true,
  function ()
    for _, wp in ipairs (table.clone (windows)) do
      if wp ~= cur_wp then
        delete_window (wp)
      end
    end
  end
)

Defun ("other-window",
       {},
[[
Select the first different window on the screen.
All windows are arranged in a cyclic order.
This command selects the window one step away in that order.
]],
  true,
  function ()
    set_current_window (window_next (cur_wp))
  end
)


-- Scroll completions up.
function completion_scroll_up ()
  local old_wp = cur_wp
  local wp = find_window ("*Completions*")
  assert (wp)
  set_current_window (wp)
  if not execute_function ("scroll-up") then
    execute_function ("beginning-of-buffer")
  end
  set_current_window (old_wp)

  term_redisplay ()
end

-- Scroll completions down.
function completion_scroll_down ()
  local old_wp = cur_wp

  local wp = find_window ("*Completions*")
  assert (wp)
  set_current_window (wp)
  if not execute_function ("scroll-down") then
    gotoeob ()
    window_resync (cur_wp)
  end
  set_current_window (old_wp)

  term_redisplay ()
end

function window_top_visible (wp)
  return offset_to_line (wp.bp, window_o (wp)) == wp.topdelta
end

function window_bottom_visible (wp)
  return wp.all_displayed
end

function popup_window ()
  if #windows == 1 then
    -- There is only one window on the screen, so split it.
    execute_function ("split-window")
  end

  return window_next (cur_wp)
end

Defun ("split-window",
       {},
[[
Split current window into two windows, one above the other.
Both windows display the same buffer now current.
]],
  true,
  function ()
    -- Windows smaller than 4 lines cannot be split.
    if cur_wp.fheight < 4 then
      minibuf_error (string.format ("Window height %d too small (after splitting)", cur_wp.fheight))
      return false
    end

    local newwp = table.clone (cur_wp)
    newwp.fheight = cur_wp.fheight / 2 + cur_wp.fheight % 2
    newwp.eheight = newwp.fheight - 1
    newwp.saved_pt = point_marker ()
    table.insert (windows, newwp)

    cur_wp.next = newwp
    cur_wp.fheight = math.floor (cur_wp.fheight / 2)
    cur_wp.eheight = cur_wp.fheight - 1
    if cur_wp.topdelta >= cur_wp.eheight then
      recenter (cur_wp)
    end

    return true
  end
)

-- This function creates the scratch buffer and window when there are
-- no other windows (and possibly no other buffers).
function create_scratch_window ()
  local bp = create_scratch_buffer ()
  local w, h = term_width (), term_height ()
  local wp = window_new ()
  cur_wp = wp
  table.insert (windows, wp)
  wp.fwidth = w
  wp.ewidth = w
  -- Save space for minibuffer.
  wp.fheight = h - 1
  -- Save space for status line.
  wp.eheight = wp.fheight - 1
  cur_bp = bp
  wp.bp = cur_bp
end

function window_resync (wp)
  local n = offset_to_line (wp.bp, get_buffer_pt (wp.bp))
  local delta = n - wp.lastpointn

  if delta ~= 0 then
    if (delta > 0 and wp.topdelta + delta < wp.eheight) or (delta < 0 and wp.topdelta >= -delta) then
      wp.topdelta = wp.topdelta + delta
    elseif n > wp.eheight / 2 then
      wp.topdelta = math.floor (wp.eheight / 2)
    else
      wp.topdelta = n
    end
    wp.redisplay = true
  end
  wp.lastpointn = n
end

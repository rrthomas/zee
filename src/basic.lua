-- Basic movement functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
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

Defun ("beginning-of-line",
       {},
[[
Move point to beginning of current line.
]],
  true,
  function ()
    goto_offset (get_buffer_line_o (cur_bp))
    cur_bp.goalc = 0
  end
)

Defun ("end-of-line",
       {},
[[
Move point to end of current line.
]],
  true,
  function ()
    goto_offset (get_buffer_line_o (cur_bp) + get_buffer_line_len (cur_bp))
    cur_bp.goalc = math.huge
  end
)

function backward_char ()
  return move_char (-1)
end

function forward_char ()
  return move_char (1)
end

Defun ("backward-char",
       {"number"},
[[
Move point left N characters (right if N is negative).
On attempt to pass beginning or end of buffer, stop and signal error.
]],
  true,
  function (n)
    local ok = move_char (-(n or 1))
    if not ok then
      minibuf_error ("Beginning of buffer")
    end
    return ok
  end
)

Defun ("forward-char",
       {"number"},
[[
Move point right N characters (left if N is negative).
On reaching end of buffer, stop and signal error.
]],
  true,
  function (n)
    local ok = move_char (n or 1)
    if not ok then
      minibuf_error ("End of buffer")
    end
    return ok
  end
)

-- Get the goal column, expanding tabs.
-- FIXME: Get start of line from o, not from get_buffer_line_o.
function get_goalc_bp (bp, o)
  local col = 0
  local t = tab_width (bp)
  for i = 1, o - get_buffer_line_o (bp) do
    if get_buffer_char (bp, get_buffer_line_o (bp) + i) == '\t' then
      col = bit.bor (col, t - 1)
    end
    col = col + 1
  end

  return col
end

function get_goalc ()
  return get_goalc_bp (cur_bp, get_buffer_o (cur_bp))
end

Defun ("goto-char",
       {"number"},
[[
Set point to @i{position}, a number.
Beginning of buffer is position 1.
]],
  true,
  function (n)
    if not n then
      n = minibuf_read_number ("Goto char: ", "")
    end

    if type (n) == "number" then
      goto_offset (math.max (n, 1) - 1)
    else
      return false
    end
  end
)

Defun ("goto-line",
       {"number"},
[[
Goto @i{line}, counting from line 1 at beginning of buffer.
]],
  true,
  function (n)
    n = n or current_prefix_arg
    if not n and _interactive then
      n = minibuf_read_number ("Goto line: ")
    end

    if type (n) == "number" then
      move_line ((math.max (n, 1) - 1) - get_buffer_pt (cur_bp).n)
      execute_function ("beginning-of-line")
    else
      return false
    end
  end
)

function previous_line ()
  return move_line (-1)
end

function next_line ()
  return move_line (1)
end

Defun ("previous-line",
       {"number"},
[[
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
]],
  true,
  function (n)
    return move_line (-(n or current_prefix_arg or 1))
  end
)

Defun ("next-line",
       {"number"},
[[
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
]],
  true,
  function (n)
    return move_line (n or current_prefix_arg or 1)
  end
)

Defun ("beginning-of-buffer",
       {},
[[
Move point to the beginning of the buffer; leave mark at previous position.
]],
  true,
  function ()
    goto_offset (0)
  end
)

Defun ("end-of-buffer",
       {},
[[
Move point to the end of the buffer; leave mark at previous position.
]],
  true,
  function ()
    goto_offset (get_buffer_size (cur_bp))
  end
)

local function scroll_down ()
  if not window_top_visible (cur_wp) then
    return move_line (-cur_wp.eheight)
  end

  minibuf_error ("Beginning of buffer")
  return false
end

local function scroll_up ()
  if not window_bottom_visible (cur_wp) then
    return move_line (cur_wp.eheight)
  end

  minibuf_error ("End of buffer")
  return false
end

Defun ("scroll-down",
       {"number"},
[[
Scroll text of current window downward near full screen.
]],
  true,
  function (n)
    return execute_with_uniarg (false, n or 1, scroll_down, scroll_up)
  end
)

Defun ("scroll-up",
       {"number"},
[[
Scroll text of current window upward near full screen.
]],
  true,
  function (n)
    return execute_with_uniarg (false, n or 1, scroll_up, scroll_down)
  end
)

-- Basic movement functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
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

Defun ("move-start-line",
[[
Move point to beginning of current line.
]],
  function ()
    goto_offset (get_buffer_line_o (cur_bp))
    cur_bp.goalc = 0
  end
)

Defun ("move-end-line",
[[
Move point to end of current line.
]],
  function ()
    goto_offset (get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp))
    cur_bp.goalc = math.huge
  end
)

Defun ("move-previous-character",
[[
Move point left N characters (right if N is negative).
On attempt to pass beginning or end of buffer, stop and signal error.
]],
  function (n)
    local ok = move_char (-(n or 1))
    if not ok then
      minibuf_error ("Beginning of buffer")
    end
    return ok
  end
)

Defun ("move-next-character",
[[
Move point right N characters (left if N is negative).
On reaching end of buffer, stop and signal error.
]],
  function (n)
    local ok = move_char (tonumber (n) or 1)
    if not ok then
      minibuf_error ("End of buffer")
    end
    return ok
  end
)

-- Get the goal column, expanding tabs.
function get_goalc ()
  local o = get_buffer_pt (cur_bp)
  local col = 0
  local t = tab_width ()
  local start = buffer_start_of_line (cur_bp, o)
  for i = 1, o - start do
    if get_buffer_char (cur_bp, start + i) == '\t' then
      col = bit32.bor (col, t - 1)
    end
    col = col + 1
  end

  return col
end

-- FIXME: cope with out-of-range arg
Defun ("goto-char",
[[
Set point to @i{position}, a number.
Beginning of buffer is position 1.
]],
  function (n)
    if not n then
      n = minibuf_read_number ("Goto char: ")
    end
    n = tonumber (n)

    return type (n) == "number" and goto_offset (math.max (n, 1))
  end
)

Defun ("edit-goto-line",
[[
Goto @i{line}, counting from line 1 at beginning of buffer.
]],
  function (n)
    n = tonumber (n)
    if not n and _interactive then
      n = minibuf_read_number ("Goto line: ")
    end

    if type (n) == "number" then
      move_line ((math.max (n, 1) - 1) - offset_to_line (cur_bp, get_buffer_pt (cur_bp)))
      execute_function ("move-start-line")
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

Defun ("move-previous-line",
[[
Move cursor vertically up one line.
If there is no character in the target line exactly over the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
]],
  function ()
    return move_line (-1)
  end
)

Defun ("move-next-line",
[[
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
]],
  function (n)
    return move_line (1)
  end
)

Defun ("move-start-file",
[[
Move point to the beginning of the buffer; leave mark at previous position.
]],
  function ()
    goto_offset (1)
  end
)

Defun ("move-end-file",
[[
Move point to the end of the buffer; leave mark at previous position.
]],
  function ()
    goto_offset (get_buffer_size (cur_bp) + 1)
  end
)

Defun ("move-previous-page",
[[
Scroll text of current window downward near full screen.
]],
  function ()
    if not window_top_visible (cur_wp) then
      return move_line (-cur_wp.eheight)
    end

    return minibuf_error ("Beginning of buffer")
  end
)

Defun ("move-next-page",
[[
Scroll text of current window upward near full screen.
]],
  function ()
    if not window_bottom_visible (cur_wp) then
      return move_line (cur_wp.eheight)
    end

    return minibuf_error ("End of buffer")
  end
)

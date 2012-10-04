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

Command ("move-start-line",
[[
Move point to beginning of current line.
]],
  function ()
    goto_offset (get_buffer_line_o (buf))
    buf.goalc = 0
  end
)

Command ("move-end-line",
[[
Move point to end of current line.
]],
  function ()
    goto_offset (get_buffer_line_o (buf) + buffer_line_len (buf))
    buf.goalc = math.huge
  end
)

Command ("move-previous-character",
[[
Move point left one character.
On attempt to pass beginning or end of buffer, stop and signal error.
]],
  function ()
    local ok = move_char (-1)
    if not ok then
      minibuf_error ("Beginning of buffer")
    end
    return ok
  end
)

Command ("move-next-character",
[[
Move point right one characters.
On reaching end of buffer, stop and signal error.
]],
  function ()
    local ok = move_char (1)
    if not ok then
      minibuf_error ("End of buffer")
    end
    return ok
  end
)

-- Get the goal column, expanding tabs.
function get_goalc ()
  local o = get_buffer_pt (buf)
  local col = 0
  local t = tab_width ()
  local start = buffer_start_of_line (buf, o)
  for i = 1, o - start do
    if get_buffer_char (buf, start + i) == '\t' then
      col = bit32.bor (col, t - 1)
    end
    col = col + 1
  end

  return col
end

Command ("edit-goto-character",
[[
Set point to @i{position}, a number.
Beginning of buffer is position 1.
]],
  function (n)
    if not n then
      n = minibuf_read_number ("Goto char: ")
    end
    n = tonumber (n)

    return type (n) == "number" and goto_offset (math.min (get_buffer_size (buf) + 1, math.max (n, 1)))
  end
)

Command ("edit-goto-line",
[[
Goto @i{line}, counting from line 1 at beginning of buffer.
]],
  function (n)
    n = tonumber (n)
    if not n and _interactive then
      n = minibuf_read_number ("Goto line: ")
    end

    if type (n) == "number" then
      move_line ((math.max (n, 1) - 1) - offset_to_line (buf, get_buffer_pt (buf)))
      execute_command ("move-start-line")
    else
      return false
    end
  end
)

Command ("edit-goto-column",
[[
Goto @i{column}, counting from column 1 at the start of the line.
]],
  function (n)
    n = tonumber (n)
    if not n and _interactive then
      n = minibuf_read_number ("Goto column: ")
    end

    if type (n) == "number" then
      goto_offset (math.min (math.max (n, 1), buffer_line_len (buf)) - 1 + get_buffer_line_o (buf))
    else
      return false
    end
  end
)

Command ("move-previous-line",
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

Command ("move-next-line",
[[
Move cursor vertically down one line.
If there is no character in the target line exactly under the current column,
the cursor is positioned after the character in that line which spans this
column, or at the end of the line if it is not long enough.
]],
  function ()
    return move_line (1)
  end
)

Command ("move-start-file",
[[
Move point to the beginning of the buffer; leave mark at previous position.
]],
  function ()
    goto_offset (1)
  end
)

Command ("move-end-file",
[[
Move point to the end of the buffer; leave mark at previous position.
]],
  function ()
    goto_offset (get_buffer_size (buf) + 1)
  end
)

Command ("move-previous-page",
[[
Scroll text of current window downward near full screen.
]],
  function ()
    if not window_top_visible (win) then
      return move_line (-win.eheight)
    end

    return minibuf_error ("Beginning of buffer")
  end
)

Command ("move-next-page",
[[
Scroll text of current window upward near full screen.
]],
  function ()
    if not window_bottom_visible (win) then
      return move_line (win.eheight)
    end

    return minibuf_error ("End of buffer")
  end
)

-- Movement commands
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

Define ("move-start-line",
[[
Move the cursor to the beginning of the line.
]],
  function ()
    goto_offset (get_buffer_line_o (buf))
    set_goalc (0)
  end
)

Define ("move-start-line-text",
[[
Move the cursor to the first non-whitespace character on this line.
]],
  function ()
    goto_offset (get_buffer_line_o (buf))
    while not end_of_line () and following_char ():match ("%s") do
      move_char (1)
    end
  end
)

Define ("move-end-line",
[[
Move the cursor to the end of the line.
]],
  function ()
    goto_offset (get_buffer_line_o (buf) + buffer_line_len (buf))
    set_goalc (math.huge)
  end
)

Define ("move-previous-character",
[[
Move the cursor left one character.
]],
  function ()
    return move_char (-1)
  end
)

Define ("move-next-character",
[[
Move the cursor right one character.
]],
  function ()
    return move_char (1)
  end
)

Define ("move-previous-line",
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

Define ("move-next-line",
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

Define ("move-start-file",
[[
Move the cursor to the beginning of the file.
]],
  function ()
    goto_offset (1)
  end
)

Define ("move-end-file",
[[
Move the cursor to the end of the file.
]],
  function ()
    goto_offset (get_buffer_size (buf) + 1)
  end
)

Define ("move-previous-page",
[[
Scroll text of current window downward near full screen.
]],
  function ()
    return move_line (-win.eheight)
  end
)

Define ("move-next-page",
[[
Scroll text of current window upward near full screen.
]],
  function ()
    return move_line (win.eheight)
  end
)

-- Move through words
local function move_word (dir)
  local gotword = false
  repeat
    while not (dir > 0 and end_of_line or beginning_of_line) () do
      if get_buffer_char (buf, get_buffer_pt (buf) - (dir < 0 and 1 or 0)):match ("%w") then
        gotword = true
      elseif gotword then
        break
      end
      move_char (dir)
    end
  until gotword or move_char (dir)
  return gotword
end

Define ("move-next-word",
[[
Move the cursor forward one word.
]],
  function ()
    return not move_word (1)
  end
)

Define ("move-previous-word",
[[
Move the cursor backwards one word.
]],
  function ()
    return not move_word (-1)
  end
)

local function move_paragraph (dir, line_extremum)
  repeat until not is_empty_line () or move_line (dir)
  repeat until is_empty_line () or move_line (dir)

  if is_empty_line () then
    execute_command ("move-start-line")
  else
    execute_command (line_extremum)
  end
end

Define ("move-previous-paragraph",
[[
Move the cursor backward to the start of the paragraph.
]],
  function ()
    move_paragraph (-1, "move-start-line")
  end
)

Define ("move-next-paragraph",
[[
Move the cursor forward to the end of the paragraph.
]],
  function ()
    move_paragraph (1, "move-end-line")
  end
)

Define ("move-goto-character",
[[
Move to character @i{position}. Beginning of buffer is character 1.
]],
  function (n)
    n = tonumber (n)
    if not n and interactive () then
      n = minibuf_read_number ("Goto char: ")
    end
    if type (n) ~= "number" then
      return true
    end

    goto_offset (math.min (get_buffer_size (buf) + 1, math.max (n, 1)))
  end
)

Define ("move-goto-line",
[[
Move the cursor to the given line.
Line 1 is the beginning of the buffer.
]],
  function (n)
    n = tonumber (n)
    if not n and interactive () then
      n = minibuf_read_number ("Goto line: ")
    end
    if type (n) ~= "number" then
      return true
    end

    move_line ((math.max (n, 1) - 1) - get_buffer_line (buf))
    execute_command ("move-start-line")
  end
)

-- FIXME: Abstract out the interactive input of arguments à la C Zee
Define ("move-goto-column",
[[
Move the cursor to the given column.
]],
  function (n)
    n = tonumber (n)
    if not n and interactive () then
      n = minibuf_read_number ("Goto column: ")
    end
    if type (n) ~= "number" then
      return true
    end

    goto_offset (math.min (math.max (n, 1), buffer_line_len (buf)) - 1 + get_buffer_line_o (buf))
  end
)

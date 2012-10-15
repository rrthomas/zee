-- Line-oriented editing functions
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
-- along with this program; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

function insert_string (s)
  return insert_astr (AStr (s))
end

-- Indentation command
-- Go to cur_goalc () in the previous non-blank line.
local function previous_nonblank_goalc ()
  local cur_goalc = get_goalc ()
  local ok = true

  -- Find previous non-blank line, if any.
  repeat
    ok = not move_line (-1)
  until not ok or not is_blank_line ()

  -- Go to `cur_goalc' in that non-blank line.
  while ok and not eolp () and get_goalc () < cur_goalc do
    ok = move_char (1)
  end
end

Define ("edit-indent-relative",
[[
Indent line or insert a tab.
]],
  function ()
    local cur_goalc = get_goalc ()
    local target_goalc = 0
    local m = point_marker ()
    local ok

    execute_command ("edit-select-off")
    previous_nonblank_goalc ()

    -- Now find the next blank char.
    if preceding_char () ~= '\t' or get_goalc () <= cur_goalc then
      while not eolp () and not following_char ():match ("%s") do
        move_char (1)
      end
    end

    -- Find next non-blank char.
    while not eolp () and following_char ():match ("%s") do
      move_char (1)
    end

    -- Target column.
    if not eolp () then
      target_goalc = get_goalc ()
    end
    goto_offset (m.o)

    -- Indent.
    undo_start_sequence ()
    if target_goalc > 0 then
      -- If not at EOL on target line, insert spaces & tabs up to
      -- target_goalc.
      while get_goalc () < target_goalc do
        ok = insert_char (' ')
      end
    else
      -- if already at EOL on target line, insert a tab.
      ok = call_command ("insert-tab")
    end
    undo_end_sequence ()

    return ok
  end
)

Define ("edit-insert-newline-and-indent",
[[
Insert a newline, then indent.
Indentation is done using the `edit-indent-relative' function, except
that if there is a character in the first column of the line above,
no indenting is performed.
]],
  function ()
    execute_command ("edit-select-off")

    undo_start_sequence ()
    if insert_string ("\n") then
      local m = point_marker ()
      local pos

      -- Check where last non-blank goalc is.
      previous_nonblank_goalc ()
      pos = get_goalc ()
      local indent = pos > 0 or (not eolp () and following_char ():match ("%s"))
      goto_offset (m.o)
      -- Only indent if we're in column > 0 or we're in column 0 and
      -- there is a space character there in the last non-blank line.
      if indent then
        execute_command ("edit-indent-relative")
      end
    end
    undo_end_sequence ()
  end
)


Define ("edit-delete-next-character",
[[
Delete the following character.
Join lines if the character is a newline.
]],
  function ()
    delete_char ()
  end
)

Define ("edit-delete-previous-character",
[[
Delete the previous character.
Join lines if the character is a newline.
]],
  function ()
    if move_char (-1) then
      minibuf_error ("Beginning of buffer")
      return true
    end

    delete_char ()
  end
)

Define ("edit-delete-horizontal-space",
[[
Delete all spaces and tabs around point.
]],
  function ()
    undo_start_sequence ()

    while not eolp () and following_char ():match ("%s") do
      delete_char ()
    end

    while not bolp () and preceding_char ():match ("%s") do
      execute_command ("edit-delete-previous-character")
    end

    undo_end_sequence ()
  end
)

Define ("edit-insert-tab",
[[
Indent to next multiple of `indent-width'.
]],
  function ()
    local t = indent_width ()
    return not insert_string (string.rep (' ', t - get_goalc () % t))
  end
)

Define ("edit-insert-newline",
[[
Insert a newline.
]],
  function ()
    return not insert_string ("\n")
  end
)

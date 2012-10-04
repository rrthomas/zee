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

-- If point is greater than wrap-column, then split the line at the
-- right-most space character at or before wrap-column, if there is
-- one, or at the left-most at or after wrap-column, if not. If the
-- line contains no spaces, no break is made.
--
-- Return flag indicating whether break was made.
function wrap_break_line ()
  local i, old_col
  local break_col = 0
  local wrapcol = tonumber (get_variable ("wrap-column"))
  local break_made = false

  -- Only break if we're beyond wrap-column.
  if get_goalc () > wrapcol then
    -- Save point.
    local m = point_marker ()

    -- Move cursor back to wrap column
    old_col = get_buffer_pt (buf) - get_buffer_line_o (buf)
    while get_goalc () > wrapcol + 1 do
      move_char (-1)
    end

    -- Find break point moving left from wrap-column.
    for i = get_buffer_pt (buf) - get_buffer_line_o (buf), 1, -1 do
      if get_buffer_char (buf, get_buffer_line_o (buf) + i - 1):match ("%s") then
        break_col = i
        break
      end
    end

    -- If no break point moving left from wrap-column, find first
    -- possible moving right.
    if break_col == 0 then
      for i = get_buffer_pt (buf) + 1, buffer_end_of_line (buf, get_buffer_line_o (buf)) do
        if get_buffer_char (buf, i - 1):match ("%s") then
          break_col = i - get_buffer_line_o (buf)
          break
        end
      end
    end

    if break_col >= 1 then -- Break line.
      goto_offset (get_buffer_line_o (buf) + break_col)
      execute_command ("delete-horizontal-space")
      insert_string ("\n")
      goto_offset (m.o)
      break_made = true
    else -- Undo fiddling with point.
      goto_offset (get_buffer_line_o (buf) + old_col)
    end

    unchain_marker (m)
  end

  return break_made
end

local function insert_expanded_tab ()
  local t = tab_width ()
  insert_string (string.rep (' ', t - get_goalc () % t))
end

local function insert_tab ()
  if warn_if_readonly_buffer () then
    return false
  end

  if get_variable ("indent-tabs-mode") then
    insert_char ('\t')
  else
    insert_expanded_tab ()
  end

  return true
end

-- Indentation command
-- Go to cur_goalc () in the previous non-blank line.
local function previous_nonblank_goalc ()
  local cur_goalc = get_goalc ()

  -- Find previous non-blank line.
  execute_command ("move-start-line")
  while move_line (-1) and is_blank_line () do end

  -- Go to `cur_goalc' in that non-blank line.
  while not eolp () and get_goalc () < cur_goalc do
    move_char (1)
  end
end

Command ("indent-relative",
[[
Indent line or insert a tab.
]],
  function ()
    local target_goalc = 0
    local cur_goalc = get_goalc ()
    local t = tab_width ()
    local ok = false

    if warn_if_readonly_buffer () then
      return false
    end

    deactivate_mark ()

    -- If we're on the first line, set target to 0.
    if get_buffer_line_o (buf) == 0 then
      target_goalc = 0
    else
      -- Find goalc in previous non-blank line.
      local m = point_marker ()

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
      unchain_marker (m)
    end

    -- Insert indentation.
    undo_start_sequence ()
    if target_goalc > 0 then
      -- If not at EOL on target line, insert spaces & tabs up to
      -- target_goalc; if already at EOL on target line, insert a tab.
      cur_goalc = get_goalc ()
      if cur_goalc < target_goalc then
        repeat
          if cur_goalc % t == 0 and cur_goalc + t <= target_goalc then
            ok = insert_tab ()
          else
            ok = insert_char (' ')
          end
          cur_goalc = get_goalc ()
        until not ok or cur_goalc >= target_goalc
      else
        ok = insert_tab ()
      end
    else
      ok = insert_tab ()
    end
    undo_end_sequence ()

    return ok
  end
)

Command ("edit-insert-newline-and-indent",
[[
Insert a newline, then indent.
Indentation is done using the `indent-relative' function, except
that if there is a character in the first column of the line above,
no indenting is performed.
]],
  function ()
    local ok = false

    if warn_if_readonly_buffer () then
      return false
    end

    deactivate_mark ()

    undo_start_sequence ()
    if insert_string ("\n") then
      local m = point_marker ()
      local pos

      -- Check where last non-blank goalc is.
      previous_nonblank_goalc ()
      pos = get_goalc ()
      local indent = pos > 0 or (not eolp () and following_char ():match ("%s"))
      goto_offset (m.o)
      unchain_marker (m)
      -- Only indent if we're in column > 0 or we're in column 0 and
      -- there is a space character there in the last non-blank line.
      if indent then
        execute_command ("indent-relative")
      end
      ok = true
    end
    undo_end_sequence ()

    return ok
  end
)


Command ("edit-delete-next-character",
[[
Delete the following character.
Join lines if the character is a newline.
]],
  function ()
    delete_char ()
  end
)

Command ("edit-delete-previous-character",
[[
Delete the previous character.
Join lines if the character is a newline.
]],
  function ()
    deactivate_mark ()

    if not move_char (-1) then
      minibuf_error ("Beginning of buffer")
      return false
    end

    delete_char ()
    return true
  end
)

Command ("delete-horizontal-space",
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

Command ("edit-insert-tab",
[[
Indent to next multiple of `indent_width'.
]],
  function ()
    return insert_tab ()
  end
)

Command ("edit-insert-newline",
[[
Insert a newline, wrapping if in Wrap mode.
]],
  function ()
    if buf.wrap and get_goalc () > tonumber (get_variable ("wrap-column")) then
      wrap_break_line ()
    end
    return insert_string ("\n")
  end
)

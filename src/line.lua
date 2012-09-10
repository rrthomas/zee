-- Line-oriented editing functions
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
-- along with this program; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

function insert_string (s)
  return insert_astr (AStr (s))
end

-- If point is greater than fill-column, then split the line at the
-- right-most space character at or before fill-column, if there is
-- one, or at the left-most at or after fill-column, if not. If the
-- line contains no spaces, no break is made.
--
-- Return flag indicating whether break was made.
function fill_break_line ()
  local i, old_col
  local break_col = 0
  local fillcol = get_variable_number ("fill-column")
  local break_made = false

  -- Only break if we're beyond fill-column.
  if get_goalc () > fillcol then
    -- Save point.
    local m = point_marker ()

    -- Move cursor back to fill column
    old_col = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp)
    while get_goalc () > fillcol + 1 do
      move_char (-1)
    end

    -- Find break point moving left from fill-column.
    for i = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp), 1, -1 do
      if get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i - 1):match ("%s") then
        break_col = i
        break
      end
    end

    -- If no break point moving left from fill-column, find first
    -- possible moving right.
    if break_col == 0 then
      for i = get_buffer_pt (cur_bp) + 1, buffer_end_of_line (cur_bp, get_buffer_line_o (cur_bp)) do
        if get_buffer_char (cur_bp, i - 1):match ("%s") then
          break_col = i - get_buffer_line_o (cur_bp)
          break
        end
      end
    end

    if break_col >= 1 then -- Break line.
      goto_offset (get_buffer_line_o (cur_bp) + break_col)
      execute_function ("delete-horizontal-space")
      insert_newline ()
      goto_offset (m.o)
      break_made = true
    else -- Undo fiddling with point.
      goto_offset (get_buffer_line_o (cur_bp) + old_col)
    end

    unchain_marker (m)
  end

  return break_made
end

function insert_newline ()
  return insert_string ("\n")
end

-- Insert a newline at the current position without moving the cursor.
function intercalate_newline ()
  return insert_newline () and move_char (-1)
end

local function insert_expanded_tab ()
  local t = tab_width (cur_bp)
  insert_string (string.rep (' ', t - get_goalc () % t))
end

local function insert_tab ()
  if warn_if_readonly_buffer () then
    return false
  end

  if get_variable_bool ("indent-tabs-mode") then
    insert_char ('\t')
  else
    insert_expanded_tab ()
  end

  return true
end

local function backward_delete_char ()
  deactivate_mark ()

  if not move_char (-1) then
    minibuf_error ("Beginning of buffer")
    return false
  end

  delete_char ()
  return true
end

-- Indentation command
-- Go to cur_goalc () in the previous non-blank line.
local function previous_nonblank_goalc ()
  local cur_goalc = get_goalc ()

  -- Find previous non-blank line.
  while execute_function ("forward-line", -1) and is_blank_line () do
  end

  -- Go to `cur_goalc' in that non-blank line.
  while not eolp () and get_goalc () < cur_goalc do
    move_char (1)
  end
end

local function previous_line_indent ()
  local cur_indent
  local m = point_marker ()

  execute_function ("move-previous-line")
  execute_function ("move-start-line")

  -- Find first non-blank char.
  while not eolp () and following_char ():match ("%s") do
    move_char (1)
  end

  cur_indent = get_goalc ()

  -- Restore point.
  goto_offset (m.o)
  unchain_marker (m)

  return cur_indent
end

Defun ("indent-for-tab-command",
       {},
[[
Indent line or insert a tab.
Depending on `tab-always-indent', either insert a tab or indent.
If initial point was within line's indentation, position after
the indentation.  Else stay at same point in text.
]],
  true,
  function ()
    if get_variable_bool ("tab-always-indent") then
      return insert_tab ()
    elseif (get_goalc () < previous_line_indent ()) then
      return execute_function ("indent-relative")
    end
  end
)

Defun ("indent-relative",
       {},
[[
Space out to under next indent point in previous nonblank line.
An indent point is a non-whitespace character following whitespace.
The following line shows the indentation points in this line.
    ^         ^    ^     ^   ^           ^      ^  ^    ^
If the previous nonblank line has no indent points beyond the
column point starts at, `edit-insert-tab' is done instead, unless
this command is invoked with a numeric argument, in which case it
does nothing.
]],
  true,
  function ()
    local target_goalc = 0
    local cur_goalc = get_goalc ()
    local t = tab_width (cur_bp)
    local ok = false

    if warn_if_readonly_buffer () then
      return false
    end

    deactivate_mark ()

    -- If we're on the first line, set target to 0.
    if get_buffer_line_o (cur_bp) == 0 then
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

Defun ("edit-insert-newline-and-indent",
       {},
[[
Insert a newline, then indent.
Indentation is done using the `indent-for-tab-command' function.
]],
  true,
  function ()
    local ok = false

    if warn_if_readonly_buffer () then
      return false
    end

    deactivate_mark ()

    undo_start_sequence ()
    if insert_newline () then
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
        execute_function ("indent-for-tab-command")
      end
      ok = true
    end
    undo_end_sequence ()

    return ok
  end
)


Defun ("edit-delete-next-character",
       {"number"},
[[
Delete the following @i{n} characters (previous if @i{n} is negative).
]],
  true,
  function (n)
    return execute_with_uniarg (true, n, delete_char, backward_delete_char)
  end
)

Defun ("backward-edit-delete-next-character",
       {"number"},
[[
Delete the previous @i{n} characters (following if @i{n} is negative).
]],
  true,
  function (n)
    return execute_with_uniarg (true, n, backward_delete_char, delete_char)
  end
)

Defun ("delete-horizontal-space",
       {},
[[
Delete all spaces and tabs around point.
]],
  true,
  function ()
    undo_start_sequence ()

    while not eolp () and following_char ():match ("%s") do
      delete_char ()
    end

    while not bolp () and preceding_char ():match ("%s") do
      backward_delete_char ()
    end

    undo_end_sequence ()
  end
)

Defun ("edit-insert-tab",
       {"number"},
[[
Insert a tabulation at the current point position into the current
buffer.
]],
  true,
  function (n)
    return execute_with_uniarg (true, n, insert_tab)
  end
)

local function newline ()
  if cur_bp.autofill and get_goalc () > get_variable_number ("fill-column") then
    fill_break_line ()
  end
  return insert_newline ()
end

Defun ("edit-insert-newline",
       {"number"},
[[
Insert a newline at the current point position into
the current buffer.
]],
  true,
  function (n)
    return execute_with_uniarg (true, n, newline)
  end
)

Defun ("open-line",
       {"number"},
[[
Insert a newline and leave point before it.
]],
  true,
  function (n)
    return execute_with_uniarg (true, n, intercalate_newline)
  end
)

Defun ("insert",
       {"string"},
[[
Insert the argument at point.
]],
  false,
  function (arg)
    insert_string (arg)
  end
)

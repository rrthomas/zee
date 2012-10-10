-- Delete buffer facility functions
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

local delete_buffer_text

local function maybe_free_delete_buffer ()
  if _last_command ~= "edit-delete-selection" then
    delete_buffer_text = nil
  end
end

local function copy_or_delete_the_region (delete)
  local rp = calculate_the_region ()

  if rp then
    maybe_free_delete_buffer ()
    delete_buffer_text = (delete_buffer_text or AStr ("")):cat (get_buffer_region (buf, rp))

    if delete then
      if buf.readonly then
        minibuf_error ("Read only text copied to delete buffer")
      else
        assert (delete_region (rp))
      end
    end

    _this_command = "edit-delete-selection"
    deactivate_mark ()

    return true
  end

  return false
end

local function delete_text (mark_func)
  maybe_free_delete_buffer ()

  if warn_if_readonly_buffer () then
    return false
  end

  local m = point_marker ()
  undo_start_sequence ()
  set_mark ()
  execute_command (mark_func)
  execute_command ("edit-delete-selection")
  undo_end_sequence ()
  set_mark ()
  unchain_marker (m)

  _this_command = "edit-delete-selection"
  minibuf_write ("") -- Erase "Set mark" message.
  return true
end

Define ("edit-delete-word",
[[
Kill characters forward until encountering the end of a word.
]],
  function ()
    return delete_text ("move-next-word")
  end
)

Define ("edit-delete-word-backward",
[[
Kill characters backward until encountering the end of a word.
]],
  function ()
    return delete_text ("move-previous-word")
  end
)

Define ("edit-paste",
[[
Reinsert the stretch of deleted text most recently deleted.
Set mark at beginning, and put point at end.
]],
  function ()
    if not delete_buffer_text then
      minibuf_error ("Delete buffer is empty")
      return false
    end

    if warn_if_readonly_buffer () then
      return false
    end

    insert_astr (delete_buffer_text)
    deactivate_mark ()
  end
)

-- FIXME: Rename
Define ("edit-delete-selection",
[[
Delete the selection.
The text is deleted, unless the buffer is read-only, and saved in the
delete buffer; the `edit-paste' command retrieves it.
If the previous command was also a delete command,
the text deleted this time appends to the text deleted last time.
]],
  function ()
    return copy_or_delete_the_region (true)
  end
)

Define ("edit-copy",
[[
Copy the selection to the delete buffer.
]],
  function ()
    return copy_or_delete_the_region (false)
  end
)

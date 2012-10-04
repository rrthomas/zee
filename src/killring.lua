-- Kill ring facility functions
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

local kill_ring_text

local function maybe_free_kill_ring ()
  if _last_command ~= "edit-kill-selection" then
    kill_ring_text = nil
  end
end

local function copy_or_kill_the_region (kill)
  local rp = calculate_the_region ()

  if rp then
    maybe_free_kill_ring ()
    kill_ring_text = (kill_ring_text or AStr ("")):cat (get_buffer_region (buf, rp))

    if kill then
      if buf.readonly then
        minibuf_error ("Read only text copied to kill ring")
      else
        assert (delete_region (rp))
      end
    end

    _this_command = "edit-kill-selection"
    deactivate_mark ()

    return true
  end

  return false
end

local function kill_text (mark_func)
  maybe_free_kill_ring ()

  if warn_if_readonly_buffer () then
    return false
  end

  local m = point_marker ()
  undo_start_sequence ()
  select_on ()
  execute_command (mark_func)
  execute_command ("edit-kill-selection")
  undo_end_sequence ()
  set_mark (m)
  unchain_marker (m)

  _this_command = "edit-kill-selection"
  minibuf_write ("") -- Erase "Set mark" message.
  return true
end

Command ("edit-kill-word",
[[
Kill characters forward until encountering the end of a word.
]],
  function ()
    return kill_text ("move-next-word")
  end
)

Command ("edit-kill-word-backward",
[[
Kill characters backward until encountering the end of a word.
]],
  function ()
    return kill_text ("move-previous-word")
  end
)

Command ("edit-paste",
[[
Reinsert the stretch of killed text most recently killed.
Set mark at beginning, and put point at end.
]],
  function ()
    if not kill_ring_text then
      minibuf_error ("Kill ring is empty")
      return false
    end

    if warn_if_readonly_buffer () then
      return false
    end

    insert_astr (kill_ring_text)
    deactivate_mark ()
  end
)

-- FIXME: Rename
Command ("edit-kill-selection",
[[
Delete the selection.
The text is deleted, unless the buffer is read-only, and saved in the
kill buffer; the `edit_paste' command retrieves it.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time.
]],
  function ()
    return copy_or_kill_the_region (true)
  end
)

Command ("edit-copy",
[[
Copy the selection to the kill buffer.
]],
  function ()
    return copy_or_kill_the_region (false)
  end
)

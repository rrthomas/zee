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

local function kill_ring_push (as)
  kill_ring_text = (kill_ring_text or AStr ("")):cat (as)
end

local function copy_or_kill_region (kill, rp)
  kill_ring_push (get_buffer_region (cur_bp, rp))

  if kill then
    if cur_bp.readonly then
      minibuf_error ("Read only text copied to kill ring")
    else
      assert (delete_region (rp))
    end
  end

  _this_command = "edit-kill-selection"
  deactivate_mark ()

  return true
end

local function copy_or_kill_the_region (kill)
  local rp = calculate_the_region ()

  if rp then
    maybe_free_kill_ring ()
    copy_or_kill_region (kill, rp)
    return true
  end

  return false
end

local function kill_text (uniarg, mark_func)
  maybe_free_kill_ring ()

  if warn_if_readonly_buffer () then
    return false
  end

  push_mark ()
  undo_start_sequence ()
  execute_function ("edit-select-on")
  execute_function (mark_func, uniarg)
  execute_function ("edit-kill-selection")
  undo_end_sequence ()
  pop_mark ()

  _this_command = "edit-kill-selection"
  minibuf_write ("") -- Erase "Set mark" message.
  return true
end

Defun ("edit-kill-word",
       {"number"},
[[
Kill characters forward until encountering the end of a word.
With argument @i{arg}, do this that many times.
]],
  true,
  function (arg)
    return kill_text (arg, "move-next-word")
  end
)

Defun ("edit-kill-word-backward",
       {"number"},
[[
Kill characters backward until encountering the end of a word.
With argument @i{arg}, do this that many times.
]],
  true,
  function (arg)
    return kill_text (-(arg or 1), "move-next-word")
  end
)

Defun ("edit-paste",
       {},
[[
Reinsert the last stretch of killed text.
More precisely, reinsert the stretch of killed text most recently
killed @i{or} pasted.  Put point at end, and set mark at beginning.
]],
  true,
  function ()
    if not kill_ring_text then
      minibuf_error ("Kill ring is empty")
      return false
    end

    if warn_if_readonly_buffer () then
      return false
    end

    execute_function ("set-mark-command")
    insert_astr (kill_ring_text)
    deactivate_mark ()
  end
)

Defun ("edit-kill-selection",
       {},
[[
Kill between point and mark.
The text is deleted but saved in the kill ring.
The command @kbd{C-y} (edit-paste) can retrieve it from there.
If the buffer is read-only, beep and refrain from deleting the text,
but put the text in the kill ring anyway.  This means that you can
use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time
to make one entry in the kill ring.
]],
  true,
  function ()
    return copy_or_kill_the_region (true)
  end
)

Defun ("edit-copy",
       {},
[[
Save the region as if killed, but don't kill it.
]],
  true,
  function ()
    return copy_or_kill_the_region (false)
  end
)

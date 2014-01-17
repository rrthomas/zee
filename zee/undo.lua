-- Undo facility functions
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

-- Save a reverse delta for doing undo.
local function undo_save (ty, o, osize, size)
  local u = {type = ty, next = buf.last_undo, o = o}

  if ty == "replace block" then
    u.size = size
    u.text = get_buffer_region (buf, region_new (o, o + osize))
    u.unchanged = not buf.modified
  end

  buf.last_undo = u
end


function undo_start_sequence ()
  undo_save ("start sequence", get_buffer_pt (buf))
end

function undo_end_sequence ()
  undo_save ("end sequence")
end

function undo_save_block (o, osize, size)
  undo_save ("replace block", o, osize, size)
end

-- Set unchanged flags to false.
function undo_set_unchanged (u)
  while u do
    u.unchanged = false
    u = u.next
  end
end

-- Revert an action.
-- Return the next undo entry.
local function revert_action (up)
  if up.type == "end sequence" then
    undo_start_sequence ()
    up = up.next
    while up.type ~= "start sequence" do
      up = revert_action (up)
    end
    undo_end_sequence ()
  end

  goto_offset (up.o)
  if up.type == "replace block" then
    replace_string (up.size, up.text)
    goto_offset (up.o)
    if up.unchanged then
      buf.modified = false
    end
  end

  return up.next
end

Define ("edit-undo",
[[
Undo some previous changes.
Repeat this command to undo more changes.
]],
  function ()
    if not buf.next_undo then
      minibuf_error ("No further undo information")
      buf.next_undop = buf.last_undo
      return true
    end

    buf.next_undo = revert_action (buf.next_undo)
    minibuf_write ("Undo!")
  end
)

Define ("edit-revert",
[[
Undo until buffer is unmodified.
]],
  function ()
    while buf.modified do
      execute_command ("edit-undo")
    end
  end
)

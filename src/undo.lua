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
  local up = {type = ty, next = buf.last_undop, o = o}

  if ty == "replace block" then
    up.size = size
    up.text = get_buffer_region (buf, region_new (o, o + osize))
    up.unchanged = not buf.modified
  end

  buf.last_undop = up
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
function undo_set_unchanged (up)
  while up do
    up.unchanged = false
    up = up.next
  end
end

-- Revert an action.  Return the next undo entry.
local function revert_action (up)
  if up.type == "end sequence" then
    undo_start_sequence ()
    up = up.next
    while up.type ~= "start sequence" do
      up = revert_action (up)
    end
    undo_end_sequence ()
  end

  if up.type ~= "end sequence" then
    goto_offset (up.o)
  end
  if up.type == "replace block" then
    replace_astr (up.size, up.text)
  end

  if up.unchanged then
    buf.modified = false
  end

  return up.next
end

Define ("edit-undo",
[[
Undo some previous changes.
Repeat this command to undo more changes.
]],
  function ()
    if warn_if_readonly_buffer () then
      return false
    end

    if not buf.next_undop then
      minibuf_error ("No further undo information")
      buf.next_undop = buf.last_undop
      return false
    end

    buf.next_undop = revert_action (buf.next_undop)
    minibuf_write ("Undo!")
  end
)

Define ("edit-revert",
[[
Undo until buffer is unmodified.
]],
  function ()
    -- FIXME: save pointer to current undo action and abort if we get
    -- back to it.
    while buf.modified do
      execute_command ("edit-undo")
    end
  end
)

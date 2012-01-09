-- Undo facility functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

-- Undo delta types.
UNDO_REPLACE_BLOCK = 0  -- Replace a block of characters.
UNDO_START_SEQUENCE = 1 -- Start a multi operation sequence.
UNDO_END_SEQUENCE = 2   -- End a multi operation sequence.

-- Save a reverse delta for doing undo.
local function undo_save (ty, o, osize, size)
  if cur_bp.noundo then
    return
  end

  local up = {type = ty, next = cur_bp.last_undop}

  up.o = o

  if ty == UNDO_REPLACE_BLOCK then
    up.size = size
    up.text = get_buffer_region (cur_bp, {start = o, finish = o + osize})
    up.unchanged = not cur_bp.modified
  end

  cur_bp.last_undop = up
end


function undo_start_sequence ()
  undo_save (UNDO_START_SEQUENCE, get_buffer_o (cur_bp), 0, 0)
end

function undo_end_sequence ()
  undo_save (UNDO_END_SEQUENCE, 0, 0, 0)
end

function undo_save_block (o, osize, size)
  undo_save (UNDO_REPLACE_BLOCK, o, osize, size)
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
  if up.type == UNDO_END_SEQUENCE then
    undo_start_sequence ()
    up = up.next
    while up.type ~= UNDO_START_SEQUENCE do
      up = revert_action (up)
    end
    undo_end_sequence ()
  end

  if up.type ~= UNDO_END_SEQUENCE then
    goto_offset (up.o)
  end
  if up.type == UNDO_REPLACE_BLOCK then
    replace_estr (up.size, up.text)
  end

  if up.unchanged then
    cur_bp.modified = false
  end

  return up.next
end

Defun ("undo",
       {},
[[
Undo some previous changes.
Repeat this command to undo more changes.
]],
  true,
  function ()
    if cur_bp.noundo then
      minibuf_error ("Undo disabled in this buffer")
      return false
    end

    if warn_if_readonly_buffer () then
      return false
    end

    if not cur_bp.next_undop then
      minibuf_error ("No further undo information")
      cur_bp.next_undop = cur_bp.last_undop
      return false
    end

    cur_bp.next_undop = revert_action (cur_bp.next_undop)
    minibuf_write ("Undo!")
  end
)

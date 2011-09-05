-- Macro facility functions
--
-- Copyright (c) 2010-2011 Free Software Foundation, Inc.
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

cmd_mp = {}
cur_mp = {}

function add_cmd_to_macro ()
  cur_mp = list.concat (cur_mp, cmd_mp)
  cmd_mp = {}
end

function add_key_to_cmd (key)
  table.insert (cmd_mp, key)
end

function remove_key_from_cmd ()
  table.remove (cmd_mp)
end

function cancel_kbd_macro ()
  cmd_mp = {}
  cur_mp = {}
  thisflag.defining_macro = false
end

Defun ("start-kbd-macro",
       {},
[[
Record subsequent keyboard input, defining a keyboard macro.
The commands are recorded even as they are executed.
Use @kbd{C-x )} to finish recording and make the macro available.
]],
  true,
  function ()
    if thisflag.defining_macro then
      minibuf_error ("Already defining a keyboard macro")
      return leNIL
    end

    if cur_mp ~= nil then
      cancel_kbd_macro ()
    end

    minibuf_write ("Defining keyboard macro...")

    thisflag.defining_macro = true
    cur_mp = {}
  end
)

Defun ("end-kbd-macro",
       {},
[[
Finish defining a keyboard macro.
The definition was started by @kbd{C-x (}.
The macro is now available for use via @kbd{C-x e}.
]],
  true,
  function ()
    if not thisflag.defining_macro then
      minibuf_error ("Not defining a keyboard macro")
      return leNIL
    end

    thisflag.defining_macro = false
  end
)

Defun ("call-last-kbd-macro",
       {},
[[
Call the last keyboard macro that you defined with @kbd{C-x (}.
A prefix argument serves as a repeat count.
]],
  true,
  function ()
    if cur_mp == nil then
      minibuf_error ("No kbd macro has been defined")
      return leNIL
    end

    undo_save (UNDO_START_SEQUENCE, cur_bp.pt, 0, 0)
    for _ = 1, current_prefix_arg or 1 do
      process_keys (cur_mp)
    end
    undo_save (UNDO_END_SEQUENCE, cur_bp.pt, 0, 0)
  end
)

Defun ("execute-kbd-macro",
  {"string"},
[[
Execute macro as string of editor command characters.
]],
  false,
  function (keystr)
    local keys = keystrtovec (keystr)
    if keys ~= nil then
      undo_save (UNDO_START_SEQUENCE, cur_bp.pt, 0, 0)
      process_keys (keys)
      undo_save (UNDO_END_SEQUENCE, cur_bp.pt, 0, 0)
      return leT
    else
      return leNIL
    end
  end
)

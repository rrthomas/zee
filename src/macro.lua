-- Macro facility functions
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

Defun ("macro-record",
       {},
[[
Record subsequent keyboard input, defining a keyboard macro.
The commands are recorded even as they are executed.
Use @kbd{C-x )} to finish recording and make the macro available.
]],
  function ()
    if thisflag.defining_macro then
      minibuf_error ("Already defining a keyboard macro")
      return false
    end

    if cur_mp ~= nil then
      cancel_kbd_macro ()
    end

    minibuf_write ("Defining keyboard macro...")

    thisflag.defining_macro = true
    cur_mp = {}
  end
)

Defun ("macro-stop",
       {},
[[
Finish defining a keyboard macro.
The definition was started by @kbd{C-x (}.
The macro is now available for use via @kbd{C-x e}.
]],
  function ()
    if not thisflag.defining_macro then
      minibuf_error ("Not defining a keyboard macro")
      return false
    end

    thisflag.defining_macro = false
  end
)

local function process_keys (keys)
  local cur = term_buf_len ()

  for i = #keys, 1, -1 do
    term_ungetkey (keys[i])
  end

  undo_start_sequence ()
  while term_buf_len () > cur do
    get_and_run_command ()
  end
  undo_end_sequence ()
end

Defun ("call-last-kbd-macro",
       {},
[[
Call the last keyboard macro that you defined with @kbd{C-x (}.
A prefix argument serves as a repeat count.
]],
  function ()
    if cur_mp == nil then
      minibuf_error ("No kbd macro has been defined")
      return false
    end

    -- FIXME: Call macro_play (needs a way to reverse keycode)
    process_keys (cur_mp)
    return true
  end
)

function macro_play (...)
  local keys = {}
  for _, keystr in ipairs ({...}) do
    local key = keycode (keystr)
    if key == nil then return false end
    table.insert (keys, key)
  end
  process_keys (keys)
  return true
end

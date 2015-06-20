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

local macro = {}

-- FIXME: macros should be executed immediately and abort on error;
-- they should be stored as a macro list, not a series of
-- keystrokes. Macros should return success/failure.
function add_key_to_macro (key)
  table.insert (macro, key)
end

function remove_key_from_macro ()
  table.remove (macro)
end

function cancel_macro_definition ()
  macro = {}
  thisflag.defining_macro = false
end

Define ("macro-record",
[[
Record subsequent keyboard input, defining a macro.
The commands are recorded even as they are executed.
Use `macro-stop' to finish recording and make the macro available.
]],
  function ()
    if thisflag.defining_macro then
      minibuf_error ("Already defining a keyboard macro")
      return true
    end

    if macro ~= nil then
      cancel_macro_definition ()
    end

    minibuf_write ("Defining keyboard macro...")

    thisflag.defining_macro = true
    macro = {}
  end
)

Define ("macro-stop",
[[
Finish defining a keyboard macro.
The definition was started by `macro-record'.
The macro is now available for use via `macro-play'.
]],
  function ()
    if not thisflag.defining_macro then
      minibuf_error ("Not defining a keyboard macro")
      return true
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

Define ("macro-play",
[[
Play back the last macro that you defined.
]],
  function (...)
    local m = {...}
    if #m > 0 then
      m = functional.map (keycode, std.ielems, m)
    elseif interactive () then
      m = macro
      if m == nil then
        minibuf_error ("No macro has been defined")
        return true
      end
    end

    process_keys (m)
  end
)

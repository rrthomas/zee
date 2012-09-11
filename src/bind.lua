-- Key binding functions
-- Copyright (c) 2007 Reuben Thomas.  All rights reserved.
--
-- This file is part of Zee.
--
-- Zee is free software; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free
-- Software Foundation; either version 2, or (at your option) any later
-- version.
--
-- Zee is distributed in the hope that it will be useful, but WITHOUT ANY
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
-- for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Zee; see the file COPYING.  If not, write to the Free
-- Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
-- 02111-1301, USA.

_bindings = {}

function bind_key(key, cmd)
  _bindings[key] = cmd
end

function command_to_binding(cmd)
  local t = {}
  for i, v in pairs(_bindings) do
    if v == cmd then
      table.insert(t, chordtostr(i))
    end
  end
  return table.concat(t, ", ")
end

function binding_to_command(key)
  return _bindings[key]
end

def{key_bind =
    function (key, name)
      local code
      if key then
        code = strtochord(key)
      else
        minibuf_write("Bind key: ")
        local key_name = chordtostr(getkey())
        name = minibuf_read_name(string.format("Bind key %s to command: ", key_name))
      end

      if name and name ~= "" then
        if code ~= 0 then
          bind_key(code, name)
        else
          minibuf_error("Invalid key")
          -- FIXME: Use ok as in C
          return false
        end
      end
      -- FIXME: Use ok as in C
      return true
    end,
  "Bind a command to a key chord.\nRead key chord and command name, and bind the command to the key\nchord."
}

def{key_unbind =
    function (key)
      local code
      if key then
        code = strtochord(key)
      else
        minibuf_write("Unbind key: ")
        code = getkey()
      end
      
      if code ~= 0 then
        bind_key(code, nil)
      else
        minibuf_error("Invalid key")
        -- FIXME: Use ok as in C
        return false
      end

      minibuf_clear()
        -- FIXME: Use ok as in C
      return true
    end,
  "Unbind a key.\nRead key chord, and unbind it."
}

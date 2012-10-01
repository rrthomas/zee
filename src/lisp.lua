-- User commands
--
-- Copyright (c) 2009-2012 Free Software Foundation, Inc.
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
-- along with this program; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.


-- User commands
usercmd = {}

function Defun (name, doc, func)
  usercmd[name] = {
    doc = texi (doc:chomp ()),
    func = function (...)
             local ret = func (...)
             return ret == nil and true or ret
           end
  }
end

function get_function_doc (name)
  return usercmd[name] and usercmd[name].doc or nil
end

function execute_function (name, ...)
  return usercmd[name] and usercmd[name].func and pcall (usercmd[name].func (...))
end

Defun ("load",
[[
Execute a file of Lua code named FILE.
]],
  function (file)
    if file then
      local f = loadfile (file)
      if not f then
        return false
      end
      return f ()
    end
  end
)

function function_exists (f)
  return usercmd[f] ~= nil
end


-- FIXME: Better name for execute-command.
Defun ("execute-command",
[[
Read function name and call it.
]],
  function ()
    local msg = "M-x "
    local name = minibuf_read_function_name (msg)
    return name and execute_function (name) or nil
  end
)

-- Read a function name from the minibuffer.
local functions_history = history_new ()
function minibuf_read_function_name (fmt)
  local cp = completion_new ()

  for name, func in pairs (usercmd) do
    table.insert (cp.completions, name)
  end

  return minibuf_vread_completion (fmt, "", cp, functions_history,
                                   "No function name given",
                                   "Undefined function name `%s'")
end

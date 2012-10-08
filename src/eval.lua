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


-- User things
env = {}

function Define (name, doc, value)
  env[name] = {
    doc = texi (doc:chomp ()),
  }
  if type (value) == "function" then
    env[name].func = function (...)
      local ret = value (...)
      return ret == nil and true or ret -- FIXME: avoid needing this wrapper
    end
  else
    env[name].val = value
  end
end

function get_doc (name)
  if env[name] then
    if env[name].func then
      return env[name].doc
    elseif env[name].doc then
      return (string.format ("%s is a variable.\n\nIts value is %s\n\n%s",
                             name, get_variable (name), env[name].doc))
    end
  end
end

function execute_command (name, ...)
  return env[name] and env[name].func and pcall (env[name].func (...))
end

Define ("eval",
[[
Evaluation a Lua chunk CHUNK.
]],
  function (file)
    local func, err = load (file)
    if func == nil then
      minibuf_error (string.format ("Error evaluating Lua: %s", err))
      return false
    end
    return func ()
  end
)

function command_exists (f)
  return env[f] and env[f].func
end


-- FIXME: Better name for execute-command.
Define ("execute-command",
[[
Read command name, then run it.
]],
  function ()
    local name = minibuf_read_command_name ("M-x ")
    return name and execute_command (name) or nil
  end
)

-- Read a function name from the minibuffer.
local commands_history = history_new ()
function minibuf_read_command_name (fmt)
  local cp = completion_new ()

  for name, func in pairs (env) do
    cp.completions:insert (name)
  end

  return minibuf_vread_completion (fmt, "", cp, commands_history,
                                   "No command name given",
                                   "Undefined command name `%s'")
end

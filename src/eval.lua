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

-- Turn texinfo markup into plain text
local function texi (s)
  s = s:gsub ("@i{([^}]+)}", function (s) return s:upper () end)
  s = s:gsub ("@kbd{([^}]+)}", "%1")
  return s
end

function Define (name, doc, value)
  env[name] = {
    doc = texi (doc:chomp ()),
  }
  -- FIXME: Avoid needing different fields
  if type (value) == "function" then
    env[name].func = value
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
  return env[name] and env[name].func and pcall (env[name].func, ...)
end

Define ("eval",
[[
Evaluation a Lua chunk CHUNK.
]],
  function (file)
    local func, err = load (file)
    if func == nil then
      minibuf_error (string.format ("Error evaluating Lua: %s", err))
      return true
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
    local name = minibuf_read_completion ("Command: ",
                                          completion_new (filter (function (e)
                                                                    return env[e].func
                                                                  end,
                                                                  list.elems, table.keys (env))),
                                          "command")
    if name == "" then
      return true
    end
    return execute_command (name)
  end
)

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

-- Initialise prefix arg
prefix_arg = false -- Not nil, so it is picked up in environment table

function Defun (name, argtypes, doc, func)
  usercmd[name] = {
    doc = texi (doc),
    func = function (...)
             local args = {}
             for i, v in ipairs ({...}) do
               local ty = argtypes[i]
               if ty == "number" then
                 v = tonumber (v, 10)
               elseif ty == "boolean" then
                 v = v ~= "nil"
               elseif ty == "string" then
                 v = tostring (v)
               end
               table.insert (args, v)
             end
             setfenv (func, setmetatable ({current_prefix_arg = prefix_arg},
                                          {__index = _G, __newindex = _G}))
             prefix_arg = false
             local ret = func (unpack (args))
             if ret == nil then
               ret = true
             end
             return ret
           end
  }
end

function get_function_doc (name)
  return usercmd[name] and usercmd[name].doc or nil
end

function execute_function (name, ...)
  return usercmd[name] and usercmd[name].func and usercmd[name].func (...)
end

Defun ("load",
       {"string"},
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

function execute_with_uniarg (undo, uniarg, forward, backward)
  uniarg = uniarg or 1

  if backward and uniarg < 0 then
    forward = backward
    uniarg = -uniarg
  end
  if undo then
    undo_start_sequence ()
  end
  local ret = true
  for _ = 1, uniarg do
    ret = forward ()
    if not ret then
      break
    end
  end
  if undo then
    undo_end_sequence ()
  end

  return ret
end

function move_with_uniarg (uniarg, move)
  local ret = true
  for uni = 1, math.abs (uniarg) do
    ret = move (uniarg < 0 and - 1 or 1)
    if not ret then
      break
    end
  end
  return ret
end


Defun ("execute-command",
       {"number"},
[[
Read function name, then read its arguments and call it.
]],
  function (n)
    local msg = ""

    if lastflag.set_uniarg then
      if lastflag.uniarg_empty then
        msg = "C-u "
      else
        msg = string.format ("%d ", current_prefix_arg)
      end
    end
    msg = msg .. "M-x "

    local name = minibuf_read_function_name (msg)
    return name and execute_function (name, n) or nil
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

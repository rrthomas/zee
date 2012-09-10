-- Self documentation facility functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
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

-- FIXME: Add apropos

local function write_function_description (name, doc)
  insert_string (string.format ("%s is %s built-in function in `Lua source code'.\n\n%s",
                                name,
                                get_function_interactive (name) and "an interactive" or "a",
                                doc))
end

Defun ("describe-function",
       {"string"},
[[
Display the full documentation of a function.
]],
  true,
  function (func)
    if not func then
      func = minibuf_read_function_name ("Describe function: ")
      if not func then
        return false
      end
    end

    local doc = get_function_doc (func)
    if not doc then
      return false
    else
      write_temp_buffer ("*Help*", true, write_function_description, func, doc)
    end

    return true
  end
)

local function write_key_description (name, doc, binding)
  local _interactive = get_function_interactive (name)
  assert (_interactive ~= nil)

  insert_string (string.format ("%s runs the command %s, which is %s built-in\n" ..
                                "function in `Lua source code'.\n\n%s",
                              binding, name,
                              _interactive and "an interactive" or "a",
                              doc))
end

Defun ("help-key",
       {"string"},
[[
Display documentation of the command invoked by a key sequence.
]],
  true,
  function (keystr)
    local name, binding, keys
    if keystr then
      keys = keystrtovec (keystr)
      if not keys then
        return false
      end
      name = get_function_by_keys (keys)
      binding = tostring (keys)
    else
      minibuf_write ("Describe key:")
      keys = get_key_sequence ()
      name = get_function_by_keys (keys)
      binding = tostring (keys)

      if not name then
        return minibuf_error (binding .. " is undefined")
      end
    end

    minibuf_write (string.format ("%s runs the command `%s'", binding, name))

    local doc = get_function_doc (name)
    if not doc then
      return false
    end
    write_temp_buffer ("*Help*", true, write_key_description, name, doc, binding)

    return true
  end
)

local function write_variable_description (name, curval, doc)
  insert_string (string.format ("%s is a variable defined in `Lua source code'.\n\n" ..
                                "Its value is %s\n\n%s",
                              name, curval, doc))
end

Defun ("describe-variable",
       {"string"},
[[
Display the full documentation of a variable.
]],
  true,
  function (name)
    local ok = true

    if not name then
      name = minibuf_read_variable_name ("Describe variable: ")
    end

    if not name then
      ok = false
    else
      local doc = main_vars[name].doc

      if not doc then
        ok = false
      else
        write_temp_buffer ("*Help*", true,
                           write_variable_description,
                           name, get_variable (name), doc)
      end
    end
    return ok
  end
)

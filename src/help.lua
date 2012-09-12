-- Self documentation facility functions
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

local function get_variable_doc (name)
  local doc = main_vars[name].doc
  if doc then
    return (string.format ("%s is a variable.\n\nIts value is %s\n\n%s",
                           name, get_variable (name), doc))
  end
end

Defun ("help-thing",
       {"string"},
[[
Display the documentation of a function or variable.
]],
  function (name)
    name = name or minibuf_read_name ("Describe function or variable: ")
    if not name then
      return false
    end

    local doc = get_function_doc (name) or get_variable_doc (name) or "No help available"
    popup_set (string.format ("Help for `%s':\n%s", name, doc))
    return true
  end
)

Defun ("help-key",
       {"string"},
[[
Display documentation of the command invoked by a key sequence.
]],
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

    popup_set (string.format ("%s runs the command `%s'.\n%s", binding, name, doc))

    return true
  end
)

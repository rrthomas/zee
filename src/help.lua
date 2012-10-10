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

Define ("help-thing",
[[
Display the help for the given command or variable.
]],
  function (name)
    name = name or (interactive () and minibuf_read_name ("Describe command or variable: "))
    if not name then
      return false
    end

    local doc = get_doc (name) or "No help available"
    popup_set (string.format ("Help for `%s':\n%s", name, doc))
    return true
  end
)

Define ("help-key",
[[
Display the command invoked by a key combination.
]],
  function (keystr)
    local key
    if keystr then
      key = keycode (keystr)
    else
      minibuf_write ("Describe key: ")
      key = get_key_chord (true)
    end
    if key then
      local name = get_command_by_key (key)
      local binding = tostring (key)
      if not name then
        return minibuf_error (binding .. " is undefined")
      end
      local doc = get_doc (name)
      if doc then
        popup_set (string.format ("%s runs the command `%s'.\n%s", binding, name, doc))
        return true
      end
      return false
    end
  end
)

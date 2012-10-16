-- Editor variables
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

Define ("indent-width", "Number of spaces inserted for an indentation.",
        2)
Define ("caseless-search", "Whether searching ignores case by default.",
        true)
Define ("case-replace", "Whether `query-replace' should preserve case.",
        false)


function get_variable (var)
  return (env[var] or {}).val
end

function preferences_set_variable (var, val)
  env[var] = env[var] or {}
  env[var].val = val
end

Define ("preferences-set-variable",
[[
Set a variable to the specified value.
]],
  function (var, val)
    var = var or (interactive () and
                  minibuf_read_completion ("Set variable: ",
                                           completion_new (filter (function (e)
                                                                     return not command_exists (e)
                                                                   end,
                                                                   list.elems, table.keys (env))),
                                           "variable"))
    if var == "" then
      return true
    end

    if val == nil and interactive () then
      val = minibuf_read (string.format ("Set %s to value: ", var))
    end
    if val == nil then
      return ding ()
    end
    preferences_set_variable (var, val)
  end
)

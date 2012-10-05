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

function get_variable (var)
  return (vars[var] or {}).val
end

function preferences_set_variable (var, val)
  vars[var] = vars[var] or {}
  vars[var].val = val
end

Command ("preferences-set-variable",
[[
Set a variable to the specified value.
]],
  function (var, val)
    var = var or minibuf_read_variable_name ("Set variable: ")
    if not var then
      return false
    end

    val = val or minibuf_read (string.format ("Set %s to value: ", var), "")
    if not val then
      return ding ()
    end
    preferences_set_variable (var, val)
  end
)

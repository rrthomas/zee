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
  return ((cur_bp and cur_bp.vars and cur_bp.vars[var]) or main_vars[var] or {}).val
end

function preferences_set_variable (var, val)
  local vars
  if (main_vars[var] or {}).islocal then
    cur_bp.vars = cur_bp.vars or {}
    vars = cur_bp.vars
  else
    vars = main_vars
  end
  vars[var] = vars[var] or {}
  vars[var].val = val
end

Defun ("preferences-set-variable",
[[
Set a variable value to the user-specified value.
]],
  function (var, val)
    var = var or minibuf_read_variable_name ("Set variable: ")
    if not var then
      return false
    end

    val = val or minibuf_read (string.format ("Set %s to value: ", var), "")
    local ok = true
    if not val then
      ok = execute_function ("keyboard-quit")
    end

    if ok then
      preferences_set_variable (var, val)
    end

    return ok
  end
)

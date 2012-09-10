-- Zile variables handling functions
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

function get_variable (var)
  return get_variable_bp (cur_bp, var)
end

function get_variable_bp (bp, var)
  return ((bp and bp.vars and bp.vars[var]) or main_vars[var] or {}).val
end

function get_variable_number_bp (bp, var)
  return tonumber (get_variable_bp (bp, var), 10)
  -- FIXME: Check result and signal error.
end

function get_variable_number (var)
  return get_variable_number_bp (cur_bp, var)
end

function get_variable_bool (var)
  local p = get_variable (var)
  if p then
    return p ~= "nil"
  end

  return false
end

function set_variable (var, val)
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
       {"string", "string"},
[[
Set a variable value to the user-specified value.
]],
  true,
  function (var, val)
    local ok = true

    if not var then
      var = minibuf_read_variable_name ("Set variable: ")
    end
    if not var then
      return false
    end
    if not val then
      val = minibuf_read (string.format ("Set %s to value: ", var), "")
    end
    if not val then
      ok = execute_function ("keyboard-quit")
    end

    if ok then
      set_variable (var, val)
    end

    return ok
  end
)

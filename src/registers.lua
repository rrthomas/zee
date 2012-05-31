-- Registers facility functions
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

local regs = {}

Defun ("copy-to-register",
       {"number"},
[[
Copy region into register @i{register}.
]],
  true,
  function (reg)
    if not reg then
      minibuf_write ("Copy to register: ")
      reg = getkey_unfiltered (GETKEY_DEFAULT)
    end

    if reg == 7 then
      return execute_function ("keyboard-quit")
    else
      minibuf_clear ()
      local rp = calculate_the_region ()
      if not rp then
        return false
      else
        regs[term_bytetokey (reg)] = tostring (get_buffer_region (cur_bp, rp)) -- FIXME: Convert newlines.
      end
    end

    return true
  end
)

local regnum

function insert_register ()
  insert_string (regs[term_bytetokey (regnum)])
  return true
end

Defun ("insert-register",
       {"number"},
[[
Insert contents of the user specified register.
Puts point before and mark after the inserted text.
]],
  true,
  function (reg)
    local ok = true

    if warn_if_readonly_buffer () then
      return false
    end

    if not reg then
      minibuf_write ("Insert register: ")
      reg = getkey_unfiltered (GETKEY_DEFAULT)
    end

    if reg == 7 then
      ok = execute_function ("keyboard-quit")
    else
      minibuf_clear ()
      if not regs[term_bytetokey (reg)] then
        minibuf_error ("Register does not contain text")
        ok = false
      else
        execute_function ("set-mark-command")
        regnum = reg
        execute_with_uniarg (true, current_prefix_arg, insert_register)
        execute_function ("exchange_point_and_mark")
        deactivate_mark ()
      end
    end

    return ok
  end
)

local function write_registers_list (i)
  for i, r in pairs (regs) do
    if r then
      insert_string (string.format ("Register %s contains ", tostring (i)))

      if r == "" then
        insert_string ("the empty string\n")
      elseif r:match ("^%s+$") then
        insert_string ("whitespace\n")
      else
        local len = math.min (20, math.max (0, cur_wp.ewidth - 6)) + 1
        insert_string (string.format ("text starting with\n    %s\n", string.sub (r, 1, len)))
      end
    end
  end
end

Defun ("list-registers",
       {},
[[
List defined registers.
]],
  true,
  function ()
    write_temp_buffer ("*Registers List*", true, write_registers_list)
  end
)

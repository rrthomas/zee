-- Lua implementation of estr
--
-- Copyright (c) 2011 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

-- Formats of end-of-line
coding_eol_lf = "\n"
coding_eol_crlf = "\r\n"
coding_eol_cr = "\r"

-- Make a new estr, determining EOL type from buffer contents.
-- Maximum number of EOLs to check before deciding type.
local max_eol_check_count = 3
function estr_new (s)
  local first_eol = true
  local total_eols = 0
  local es = {s = s}
  local i = 1
  while i <= #s and total_eols < max_eol_check_count do
    local c = s[i]
    if c == '\n' or c == '\r' then
      local this_eol_type
      total_eols = total_eols + 1
      if c == '\n' then
        this_eol_type = coding_eol_lf
      elseif i == #s or s[i + 1] ~= '\n' then
        this_eol_type = coding_eol_cr
      else
        this_eol_type = coding_eol_crlf
        i = i + 1
      end

      if first_eol then
        -- This is the first end-of-line.
        es.eol = this_eol_type
        first_eol = false
      elseif es.eol ~= this_eol_type then
        -- This EOL is different from the last; arbitrarily choose LF.
        es.eol = coding_eol_lf
        break
      end
    end
    i = i + 1
  end
  return es
end

estr_dup = table.clone

function estr_prev_line (es, o)
  local so = estr_start_of_line (es, o)
  if so == 0 then
    return nil
  end
  return estr_start_of_line (es, so - #es.eol)
end

function estr_next_line (es, o)
  local eo = estr_end_of_line (es, o)
  if eo == #es.s then
    return nil
  end
  return eo + #es.eol
end

function estr_start_of_line (es, o)
  local prev = find_substr (es.s, es.eol, 0, o, false, true, true, false, false)
  return prev and (prev + #es.eol - 1) or 0
end

function estr_end_of_line (es, o)
  local next = string.find (string.sub (es.s, o + 1), es.eol)
  return next and o + next - 1 or #es.s
end

function estr_line_len (es, o)
  return estr_end_of_line (es, o) - estr_start_of_line (es, o)
end

function estr_cat (es, src)
  local o = 1
  while o and o <= #src.s do
    local nexto = estr_next_line (src, o)
    es.s = es.s .. string.sub (src.s, o, nexto and nexto - #src.eol or #src.s)
    o = nexto
    if o then
      es.s = es.s .. es.eol
    end
  end

  return es
end

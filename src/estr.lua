-- Lua implementation of estr
--
-- Copyright (c) 2011-2012 Free Software Foundation, Inc.
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
  local es = {s = s, eol = coding_eol_lf}
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

function estr_prev_line (es, o)
  local so = estr_start_of_line (es, o)
  return so ~= 0 and estr_start_of_line (es, so - #es.eol) or nil
end

function estr_next_line (es, o)
  local eo = estr_end_of_line (es, o)
  return eo ~= #es.s and eo + #es.eol or nil
end

function estr_start_of_line (es, o)
  local prev = find_substr (es.s, "", es.eol, 0, o, false, true, true, false, false)
  return prev and (prev + #es.eol - 1) or 0
end

function estr_end_of_line (es, o)
  local next = es.s:find (es.eol, o + 1)
  return next and next - 1 or #es.s
end

function estr_line_len (es, o)
  return estr_end_of_line (es, o) - estr_start_of_line (es, o)
end

function estr_lines (es)
  local lines = 0
  local s = 1
  local next
  repeat
    next = es.s:find (es.eol, s)
    if next then
      lines = lines + 1
      s = next + #es.eol + 1
    end
  until not next
  return lines
end

local function replace_str (s, pos, rep)
  return s:sub (1, pos) .. rep .. s:sub (pos + 1 + #rep)
end

function estr_replace_estr (es, pos, src)
  local s = 1
  local len = #src.s
  while len > 0 do
    local next = src.s:find (src.eol, s)
    local line_len = next and next - s or len
    es.s = replace_str (es.s, pos, src.s:sub (s, s + line_len))
    pos = pos + line_len
    len = len - line_len
    s = next
    if len > 0 then
      es.s = replace_str (es.s, pos, es.eol)
      s = s + #src.eol
      len = len - #src.eol
      pos = pos + #es.eol
    end
  end
  return es
end

function estr_cat (es, src)
 local oldlen = #es.s
 es.s = es.s:sub (1, oldlen) .. string.rep ("\0", estr_len (src, es.eol)) .. es.s:sub (oldlen + 1)
 return estr_replace_estr (es, oldlen, src)
end

function estr_len (es, eol_type)
  return #es.s + estr_lines (es) * (#eol_type - #es.eol)
end

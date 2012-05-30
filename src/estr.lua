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

-- Maximum number of EOLs to check before deciding EStr type arbitrarily.
local max_eol_check_count = 3

local function replace_str (s, pos, rep)
  return s:sub (1, pos) .. rep .. s:sub (pos + 1 + #rep)
end

EStr = Object {
  _clone = function (self, s, eol)
    self.s = s
    if eol then -- if eol supplied, use it
      self.eol = eol
    else -- otherwise, guess
      local first_eol = true
      local total_eols = 0
      self.eol = coding_eol_lf
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
            self.eol = this_eol_type
            first_eol = false
          elseif self.eol ~= this_eol_type then
            -- This EOL is different from the last; arbitrarily choose LF.
            self.eol = coding_eol_lf
            break
          end
        end
        i = i + 1
      end
    end
    local object = table.clone (self)
    return setmetatable (object, object)
  end,

  prev_line = function (self, o)
    local so = self:start_of_line (o)
    return so ~= 0 and self:start_of_line (so - #self.eol) or nil
  end,

  next_line = function (self, o)
    local eo = self:end_of_line (o)
    return eo ~= #self.s and eo + #self.eol or nil
  end,

  start_of_line = function (self, o)
    local prev = find_substr (self.s, "", self.eol, 0, o, false, true, true, false, false)
    return prev and (prev + #self.eol - 1) or 0
  end,

  end_of_line = function (self, o)
    local next = self.s:find (self.eol, o + 1)
    return next and next - 1 or #self.s
  end,

  lines = function (self)
    local lines = 0
    local s = 1
    local next
    repeat
      next = self.s:find (self.eol, s)
      if next then
        lines = lines + 1
        s = next + #self.eol + 1
      end
    until not next
    return lines
  end,

  replace_estr = function (self, pos, src)
    local s = 1
    local len = #src.s
    while len > 0 do
      local next = src.s:find (src.eol, s)
      local line_len = next and next - s or len
      self.s = replace_str (self.s, pos, src.s:sub (s, s + line_len))
      pos = pos + line_len
      len = len - line_len
      s = next
      if len > 0 then
        self.s = replace_str (self.s, pos, self.eol)
        s = s + #src.eol
        len = len - #src.eol
        pos = pos + #self.eol
      end
    end
    return self
  end,

  cat = function (self, src)
    local oldlen = #self.s
    self.s = self.s:sub (1, oldlen) .. string.rep ("\0", src:len (self.eol)) .. self.s:sub (oldlen + 1)
    return self:replace_estr (oldlen, src)
  end,

  bytes = function (self)
    return #self.s
  end,
  
  len = function (self, eol_type)
    return self:bytes () + self:lines () * (#eol_type - #self.eol)
  end,

  sub = function (self, from, to)
    return self.s:sub (from, to)
  end,

  move = function (self, to, from, n)
    assert (math.max (from, to) + n <= #self.s)
    self.s = self.s:sub (1, to) .. self.s:sub (from + 1, from + n) .. self.s:sub (to + n + 1)
  end,

  set = function (self, from, c, n)
    assert (from + n <= #self.s)
    self.s = self.s:sub (1, from) .. string.rep (c, n) .. self.s:sub (from + n + 1)
  end,

  remove = function (self, from, n)
    assert (from + n <= #self.s)
    self.s = self.s:sub (1, from) .. self.s:sub (from + n + 1)
  end,

  insert = function (self, from, n)
    self.s = self.s:sub (1, from) .. string.rep ('\0', n) .. self.s:sub (from + 1)
  end
}

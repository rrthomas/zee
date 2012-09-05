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

AStr = Object {
  _init = function (self, s)
    self.s = s
    return self
  end,

  __tostring = function (self)
    return self.s
  end,

  len = function (self) -- FIXME: In Lua 5.2 use __len metamethod (doesn't work for tables in 5.1)
    return #self.s
  end,

  sub = function (self, from, to)
    return self.s:sub (from, to)
  end,

  move = function (self, to, from, n)
    assert (math.max (from, to) + n <= self.s:len () + 1)
    self.s = self.s:sub (1, to - 1) .. self.s:sub (from, from + n - 1) .. self.s:sub (to + n)
  end,

  set = function (self, from, c, n)
    assert (from + n <= self.s:len () + 1)
    self.s = self.s:sub (1, from - 1) .. string.rep (c, n) .. self.s:sub (from + n)
  end,

  remove = function (self, from, n)
    assert (from + n <= self.s:len () + 1)
    self.s = self.s:sub (1, from - 1) .. self.s:sub (from + n)
  end,

  insert = function (self, from, n)
    assert (from <= self.s:len () + 1)
    self.s = self.s:sub (1, from - 1) .. string.rep ('\0', n) .. self.s:sub (from)
  end,

  replace = function (self, pos, rep)
    assert (pos + #rep <= self.s:len () + 1)
    self.s = self.s:sub (1, pos - 1) .. rep .. self.s:sub (pos + #rep)
  end,

  find = function (self, s, from)
    return self.s:find (s, from)
  end,

  rfind = function (self, s, from)
    return find_substr (self.s, "", s, 1, from - 1, false, true, true, false, false)
  end
}

require "alien"

local allocation_chunk_size = 16
BStr = Object {
  _init = function (self, s)
    self.buf = alien.array ("char", #s, alien.buffer (s))
    self.length = #s
    return self
  end,

  __tostring = function (self)
    return self.buf.buffer:tostring (self:len ())
  end,

  len = function (self) -- FIXME: In Lua 5.2 use __len metamethod (doesn't work for tables in 5.1)
    return self.length
  end,

  sub = function (self, from, to)
    return tostring (self):sub (from, to) -- FIXME
  end,

  set_len = function (self, n)
    if n > self.buf.length or n < self.buf.length / 2 then
      self.buf:realloc (n + allocation_chunk_size)
    end
    self.length = n
  end,

  move = function (self, to, from, n)
    assert (math.max (from, to) + n <= self:len ())
    alien.memmove (self.buf.buffer:topointer (to), self.buf.buffer:topointer (from), n)
  end,

  set = function (self, from, c, n)
    assert (from + n <= self:len ())
    alien.memset (self.buf.buffer:topointer (from), c:byte (), n)
  end,

  remove = function (self, from, n)
    assert (from + n <= self:len ())
    self:move (from + n, from, n)
    self:set_len (self:len () - n)
  end,

  insert = function (self, from, n)
    assert (from <= self:len ())
    self:set_len (self:len () + n)
    self:move (from + n, from, self:len () - (from + n))
    self:set (from, '\0', n)
  end,

  replace = function (self, from, rep)
    assert (from + #rep < self:len ())
    alien.memmove (self.buf.buffer:topointer (from), rep, #rep)
  end,

  find = function (self, s, from)
    return tostring (self):find (s, from) -- FIXME
  end,

  rfind = function (self, s, from)
    return find_substr (tostring (self), "", s, 1, from - 1, false, true, true, false, false) -- FIXME
  end
}

-- Formats of end-of-line
coding_eol_lf = "\n"
coding_eol_crlf = "\r\n"
coding_eol_cr = "\r"

-- Maximum number of EOLs to check before deciding EStr type arbitrarily.
local max_eol_check_count = 3

EStr = Object {
  _init = function (self, s, eol)
    self.s = s
    if type (s) == "string" then
      self.s = AStr (s)
    end
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
    return self
  end,

  __tostring = function (self)
    return tostring (self.s)
  end,

  prev_line = function (self, o)
    local so = self:start_of_line (o)
    return so ~= 1 and self:start_of_line (so - #self.eol) or nil
  end,

  next_line = function (self, o)
    local eo = self:end_of_line (o)
    return eo <= self.s:len () and eo + #self.eol or nil
  end,

  start_of_line = function (self, o)
    local prev = self.s:rfind (self.eol, o)
    return prev and prev + #self.eol or 1
  end,

  end_of_line = function (self, o)
    local next = self.s:find (self.eol, o)
    return next or self.s:len () + 1
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

  replace = function (self, pos, src)
    local s = 1
    local len = src.s:len ()
    while len > 0 do
      local next = src.s:find (src.eol, s + #src.eol + 1)
      local line_len = next and next - s or len
      self.s:replace (pos, src.s:sub (s, s + line_len))
      pos = pos + line_len
      len = len - line_len
      s = next
      if len > 0 then
        self.s:replace (pos, self.eol)
        s = s + #src.eol
        len = len - #src.eol
        pos = pos + #self.eol
      end
    end
    return self
  end,

  cat = function (self, src)
    local oldlen = self.s:len ()
    self.s:insert (oldlen + 1, src:len (self.eol))
    return self:replace (oldlen + 1, src)
  end,

  bytes = function (self)
    return self.s:len ()
  end,

  len = function (self, eol_type) -- FIXME in Lua 5.2 use __len metamethod
    return self:bytes () + self:lines () * (#eol_type - #self.eol)
  end,

  -- FIXME: The following should all convert their character arguments to byte values before calling the underlying *Str method
  sub = function (self, from, to)
    return self.s:sub (from, to)
  end,

  move = function (self, to, from, n)
    self.s:move (to, from, n)
  end,

  set = function (self, from, c, n)
    self.s:set (from, c, n)
  end,

  remove = function (self, from, n)
    self.s:remove (from, n)
  end,

  insert = function (self, from, n)
    self.s:insert (from, n)
  end
}

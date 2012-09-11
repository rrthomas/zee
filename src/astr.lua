-- Efficient string buffers
--
-- Copyright (c) 2011-2012 Free Software Foundation, Inc.
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

require "alien"

local allocation_chunk_size = 16
AStr = Object {
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
    assert (math.max (from, to) + n <= self:len () + 1)
    alien.memmove (self.buf.buffer:topointer (to), self.buf.buffer:topointer (from), n)
  end,

  set = function (self, from, c, n)
    assert (from + n <= self:len () + 1)
    alien.memset (self.buf.buffer:topointer (from), c:byte (), n)
  end,

  remove = function (self, from, n)
    assert (from + n <= self:len () + 1)
    self:move (from + n, from, n)
    self:set_len (self:len () - n)
  end,

  insert = function (self, from, n)
    assert (from <= self:len () + 1)
    self:set_len (self:len () + n)
    self:move (from + n, from, self:len () + 1 - (from + n))
    self:set (from, '\0', n)
  end,

  replace = function (self, from, rep)
    assert (from + #rep <= self:len () + 1)
    alien.memmove (self.buf.buffer:topointer (from), rep, #rep)
    return self
  end,

  find = function (self, s, from)
    return tostring (self):find (s, from) -- FIXME
  end,

  rfind = function (self, s, from)
    return find_substr (tostring (self), "", s, 1, from - 1, false, true, true, false, false) -- FIXME
  end,

  cat = function (self, src)
    local oldlen = self:len ()
    self:insert (oldlen + 1, src:len ())
    return self:replace (oldlen + 1, tostring (src))
  end,

  prev_line = function (self, o)
    local so = self:start_of_line (o)
    return so ~= 1 and self:start_of_line (so - 1) or nil
  end,

  next_line = function (self, o)
    local eo = self:end_of_line (o)
    return eo <= self:len () and eo + 1 or nil
  end,

  start_of_line = function (self, o)
    local prev = self:rfind ("\n", o)
    return prev and prev + 1 or 1
  end,

  end_of_line = function (self, o)
    local next = self:find ("\n", o)
    return next or self:len () + 1
  end,

  lines = function (self)
    local lines = 0
    local s = 1
    local next
    repeat
      next = tostring (self):find ("\n", s)
      if next then
        lines = lines + 1
        s = next + 1
      end
    until not next
    return lines
  end,
}

-- Efficient string buffers
--
-- Copyright (c) 2011-2014 Free Software Foundation, Inc.
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

local Object = require "std.object"

alien.default.memchr:types ("pointer", "pointer", "int", "size_t")

local allocation_chunk_size = 16
AStr = Object {
  -- Primitive methods

  _init = function (self, init, len)
    if type (init) == "string" then
      self.buf = alien.array ("char", #init, alien.buffer (init))
      self.length = #init
    elseif type (init) == "number" then
      self.buf = alien.array ("char", math.max (init, 1))
      self.length = init
    elseif type (init) == "userdata" then
      self.buf = alien.array ("char", len, alien.buffer (init))
      self.length = len
    else
      self.buf = alien.array ("char", init)
      self.length = #init
    end
    return self
  end,

  __tostring = function (self)
    return self.buf.buffer:tostring (#self)
  end,

  __index = function (self, n)
    return self.buf[n]
  end,

  __len = function (self)
    return self.length
  end,

  sub = function (self, from, to)
    to = to or self.length
    return AStr (self:topointer (from), to - from + 1)
  end,

  set_len = function (self, n)
    if n > self.buf.length or n < self.buf.length / 2 then
      self.buf:realloc (n + allocation_chunk_size)
    end
    self.length = n
  end,

  move = function (self, to, from, n)
    assert (math.max (from, to) + n <= #self + 1)
    alien.memmove (self:topointer (to), self:topointer (from), n)
  end,

  set = function (self, from, c, n)
    assert (from + n <= #self + 1)
    alien.memset (self:topointer (from), c:byte (), n)
  end,

  remove = function (self, from, n)
    local b, e, eob = from, from + n, #self + 1
    assert (e <= eob)
    self:move (e, b, eob - e)
    self:set_len (#self - n)
  end,

  insert = function (self, from, n)
    assert (from <= #self + 1)
    self:set_len (#self + n)
    self:move (from + n, from, #self + 1 - (from + n))
    self:set (from, '\0', n)
  end,

  replace = function (self, from, rep, len)
    local len = len or #rep
    assert (from + len <= #self + 1)
    if type (rep) ~= "userdata" and type (rep) ~= "string" then
      rep = rep:topointer ()
    end
    alien.memmove (self:topointer (from), rep, len)
    return self
  end,

  rchr = function (self, c, from)
    local b = self.buf
    for i = from - 1, 1, -1 do
      if b[i] == c then return i end
    end
  end,

  start_of_line = function (self, o)
    local prev = self:rchr (string.byte ('\n'), o)
    return prev and prev + 1 or 1
  end,

  end_of_line = function (self, o)
    local next = alien.default.memchr (self:topointer (o), string.byte ('\n'), #self - (o - 1))
    return next and self.buf.buffer:tooffset (next) or #self + 1
  end,


  -- Derived methods

  topointer = function (self, offset)
    return self.buf.buffer:topointer (offset)
  end,

  cat = function (self, src)
    local oldlen = #self
    self:insert (oldlen + 1, #src)
    return self:replace (oldlen + 1, src:topointer (), #src)
  end,

  prev_line = function (self, o)
    local so = self:start_of_line (o)
    return so ~= 1 and self:start_of_line (so - 1) or nil
  end,

  next_line = function (self, o)
    local eo = self:end_of_line (o)
    return eo <= #self and eo + 1 or nil
  end,

  lines = function (self)
    local lines = -1
    local next = 1
    repeat
      next = self:next_line (next)
      lines = lines + 1
    until not next
    return lines
  end,
}

local have_memrchr = pcall (loadstring 'alien.default.memrchr:types ("pointer", "pointer", "int", "size_t")')
if have_memrchr then
  AStr.rchr = function (self, c, from)
    local p = alien.default.memrchr(self:topointer (), c, from - 1)
    return p and self.buf.buffer:tooffset (p)
  end
end

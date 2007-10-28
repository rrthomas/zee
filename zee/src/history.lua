-- History facility functions
-- Copyright (c) 2007 Reuben Thomas.  All rights reserved.
--
-- This file is part of Zee.
--
-- Zee is free software; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free
-- Software Foundation; either version 2, or (at your option) any later
-- version.
--
-- Zee is distributed in the hope that it will be useful, but WITHOUT ANY
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
-- for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Zee; see the file COPYING.  If not, write to the Free
-- Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
-- 02111-1301, USA.

function history_new()
  return history_prepare({})
end

function history_prepare(self)
  self.sel = 0
end

function add_history_element(self, s)
  if self[#self] ~= s then
    table.insert(self, s)
  end
end

function previous_history_element(self)
  local s

  if self.sel == 0 then -- First call for this history
    -- Select last element
    if #self > 0 then
      self.sel = #self
      s = self[self.sel]
    end
  elseif self.sel > 1 then
    -- If there is there another element, select it
    self.sel = self.sel - 1
    s = self[self.sel]
  end

  return s or ""
end

function next_history_element(self)
  local s

  if self.sel ~= 0 then
    -- Next element
    if self.sel < #self then
      self.sel = self.sel + 1
      s = self[self.sel]
    else -- No more elements (back to original status)
      self.sel = 0
    end
  end

  return s or ""
end

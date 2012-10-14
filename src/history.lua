-- History facility functions
--
-- Copyright (c) 2007, 2009-2010, 2012 Free Software Foundation, Inc.
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

function history_new ()
  return history_prepare ({})
end

function history_prepare (hp)
  hp.sel = 0
  return hp
end

function add_history_element (hp, s)
  if hp[#hp] ~= s then
    table.insert (hp, s)
  end
end

function previous_history_element (hp)
  if hp.sel == 0 then -- First call for this history
    -- Select last element
    if #hp > 0 then
      hp.sel = #hp
    end
  elseif hp.sel > 1 then
    -- If there is there another element, select it
    hp.sel = hp.sel - 1
  end
  return hp[hp.sel] or ""
end

function next_history_element (hp)
  -- Next element
  if hp.sel < #hp then
    hp.sel = hp.sel + 1
  else -- No more elements (back to original status)
    hp.sel = 1
  end

  return hp[hp.sel] or ""
end

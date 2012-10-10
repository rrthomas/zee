-- Editor-specific library functions
--
-- Copyright (c) 2006-2010, 2012 Free Software Foundation, Inc.
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


-- Recase str according to newcase.
function recase (s, newcase)
  local bs = ""
  local i, len

  if newcase == "capitalized" or newcase == "upper" then
    bs = bs .. s[1]:upper ()
  else
    bs = bs .. s[1]:lower ()
  end

  for i = 2, #s do
    bs = bs .. (newcase == "upper" and string.upper or string.lower) (s[i])
  end

  return bs
end

-- Turn texinfo markup into plain text
function texi (s)
  s = s:gsub ("@i{([^}]+)}", function (s) return s:upper () end)
  s = s:gsub ("@kbd{([^}]+)}", "%1")
  return s
end

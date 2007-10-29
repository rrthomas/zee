-- Completion facility functions
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

-- Completions table:
-- {
--   completions -- list of completion strings
--   matches -- list of matches
--   match -- the current matched string
-- }

-- If there are no completions, the popup just says "No completions".
-- If there is exactly one completion the popup says "Sole completion:
-- %s". Otherwise, write the matches in `l' in a set of columns. The
-- width of the columns is chosen to be big enough for the longest
-- rblist, with a COLUMN_GAP-character gap between each column.
local COLUMN_GAP = 3
function completion_write(cp, width)
  local s = "Completions\n\n"
  if #cp.matches == 0 then
    s = s .. "No completions"
  elseif #cp.matches == 1 then
    s = s .. "Sole completion: " .. cp.matches[1]
  else
    s = s .."Possible completions are:\n"
    local maxlen = 0
    for i, v in ipairs(cp.matches) do
      maxlen = math.max(maxlen, #v)
    end
    maxlen = maxlen + COLUMN_GAP
    local numcols = math.floor((width + COLUMN_GAP - 1) / maxlen)
    local col = 0
    for i, v in ipairs(cp.matches) do
      if col >= numcols then
        col = 0
        s = s .. "\n"
      end
      s = s .. v
      col = col + 1
      if col < numcols then
        s = s .. string.rep(" ", maxlen - #v)
      end
    end
  end
  return s
end

-- Returns the length of the longest string that is a prefix of
-- both s1 and s2.
function common_prefix_length(s1, s2)
  local len = math.min(#s1, #s2)
  for i = 1, len do
    if string.sub(s1, 1, i) ~= string.sub(s2, 1, i) then
      return i - 1
    end
  end
  return len
end

--  Match completions
--  cp - the completions
--  search - the prefix to search for (not modified).
--  Returns false if `search' is not a prefix of any completion, and true
--  otherwise. The effect on cp is as follows:
--  cp.completions - not modified.
--  cp.matches - replaced with the list of matching completions, sorted.
--  cp.match - replaced with the longest common prefix of the matches, if the
--  function returns true, otherwise not modified.
-- 
--  To see the completions in a popup, you should call completion_popup
--  after this method. You may want to call completion_remove_suffix and/or
--  completion_remove_prefix in between to keep the list manageable.
function completion_try(cp, search)
  fullmatches = 0
  cp.matches = {}
  for i, v in pairs(cp.completions) do
    if type(i) == "string" then
      if string.sub(i, 1, #search) == search then
        table.insert(cp.matches, i)
        if i == search then
          fullmatches = fullmatches + 1
        end
      end
    end
  end

  if #cp.matches == 0 then
    return false
  end

  table.sort(cp.matches)
  cp.match = cp.matches[1]
  local prefix_len = #cp.match
  for _, v in cp.matches do
    prefix_len = math.min(prefix_len, common_prefix_length(cp.match, v))
  end
  cp.match = string.sub(cp.match, 1, prefix_len)

  return true
end

-- Find the last occurrence of string `s2' before `before_pos' in
-- `s1'. Returns the offset into `s1' of `s2', or 0 if not found.
function last_occurrence(s, before_pos, c)
  return before_pos - (string.find(string.reverse(string.sub(s, 1, before_pos - 1)), c) or before_pos)
end

-- If two or more `cp.matches' have a common prefix that is longer than
-- `cp.match' and end in `_', replaces them with the longest such prefix.
-- Repeats as often as possible.
function completion_remove_suffix(cp)
  if #cp.matches == 0 then
    return
  end
  local ans = {}
  local previous = cp.matches[1]
  for p = 2, #cp.matches do
    local length = last_occurrence(previous, common_prefix_length(previous, cp.matches[p]), '_')
    if length > #cp.matches then
      previous = string.sub(previous, 0, length)
    else
      table.insert(ans, previous)
      previous = cp.matches[p]
    end
  end
  table.insert(ans, previous)
  cp.matches = ans
end

--  Finds the longest prefix of `search' that ends in an underscore, and removes
--  it from all `cp->matches'. Does nothing if there is no such prefix.
function completion_remove_prefix(cp, search)
  local pos = last_occurrence(search, #search, '_')
  if pos > 0 then
    for i, v in ipairs(cp.matches) do
      cp.matches[i] = string.sub(v, pos)
    end
  end
end

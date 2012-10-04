-- Completion facility functions
--
-- Copyright (c) 2007, 2009-2012 Free Software Foundation, Inc.
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

-- Completions table:
-- {
--   completions: list of completion strings
--   matches: list of matches
--   match: the current matched string
--   filename: true if the completion is a filename completion
--   poppedup: true if the completion is currently displayed
--   close: true if the completion window should be closed
-- }
-- FIXME: Use Objects here and elsewhere, and add an __index
-- metamethod that works like strict.lua.

-- Make a new completions table
function completion_new (filename, completions)
  return {completions = completions or {}, matches = {}, filename = filename}
end

-- Write the matches in `l' in a set of columns. The width of the
-- columns is chosen to be big enough for the longest string, with a
-- COLUMN_GAP-character gap between each column.
local COLUMN_GAP = 5
function completion_write (cp, width)
  local maxlen = 0
  for i, v in ipairs (cp.matches) do
    maxlen = math.max (maxlen, #v)
  end
  maxlen = maxlen + COLUMN_GAP
  local numcols = math.floor ((width - 1) / maxlen)
  local col = 0

  local s = "Possible completions are:\n"
  for i, v in ipairs (cp.matches) do
    s = s .. string.format ("%-" .. maxlen .. "s", v)
    col = (col + 1) % numcols
    if col == 0 then
      s = s .. '\n'
    end
  end
  return s
end

-- Returns the length of the common prefix of s1 and s2.
local function common_prefix_length (s1, s2)
  local len = math.min (#s1, #s2)
  for i = 1, len do
    if string.sub (s1, 1, i) ~= string.sub (s2, 1, i) then
      return i - 1
    end
  end
  return len
end


-- Reread directory for completions.
local function completion_readdir (cp, path)
  cp.completions = {}

  -- Normalize path, and abort if it fails
  path = normalize_path (path)
  if not path then
    return false
  end

  -- Split up path with dirname and basename, unless it ends in `/',
  -- in which case it's considered to be entirely dirname.
  local pdir
  if path[-1] ~= "/" then
    pdir = posix.dirname (path)
    if pdir ~= "/" then
      pdir = pdir .. "/"
    end
    path = posix.basename (path)
  else
    pdir = path
    path = ""
  end

  local dir = posix.dir (pdir)
  if dir then
    for _, d in ipairs (dir) do
      local s = posix.stat (pdir .. d)
      if s and s.type == "directory" then
        d = d .. "/"
      end
      table.insert (cp.completions, d)
    end

    cp.path = compact_path (pdir)
  end

  return path
end

-- Arguments:
--
-- cp - the completions
-- search - the prefix to search for
--
-- Returns the search status.
--
-- The effect on cp is as follows:
--
--   completions - reread for filename completion; otherwise unchanged
--   matches - replaced with the list of matching completions, sorted
--   match - replaced with the longest common prefix of the matches
--
-- To format the completions for a popup, call completion_write
-- after this function.
function completion_try (cp, search)
  cp.matches = {}

  if cp.filename then
    search = completion_readdir (cp, search)
  end

  local fullmatch = false
  for _, v in ipairs (cp.completions) do
    if type (v) == "string" then
      local len = math.min (#v, #search)
      if string.sub (v, 1, len) == string.sub (search, 1, len) then
        table.insert (cp.matches, v)
        if #v == #search then
          fullmatch = true
        end
      end
    end
  end

  table.sort (cp.matches)
  local match = cp.matches[1] or ""
  local prefix_len = #match
  for _, v in ipairs (cp.matches) do
    prefix_len = math.min (prefix_len, common_prefix_length (match, v))
  end
  cp.match = string.sub (match, 1, prefix_len)

  local ret = "incomplete"
  if #cp.matches == 0 then
    ret = "no match"
  elseif #cp.matches == 1 then
    ret = "match"
  elseif fullmatch and #cp.matches > 1 then
    local len = math.min (#search, #cp.match)
    if len > 0 and string.sub (cp.match, 1, len) == string.sub (search, 1, len) then
      ret = "matches"
    end
  end

  return ret
end

-- Popup the completion window.
function popup_completion (cp)
  cp.poppedup = true
  cp.close = true
  popup_set (completion_write (cp, win.ewidth))
  term_redisplay ()
end

-- FIXME: Common up minibuf_read*_name
function minibuf_read_name (fmt)
  local cp = completion_new (nil, list.concat (table.keys (main_vars), table.keys (usercmd)))

  return minibuf_vread_completion (fmt, "", cp, nil,
                                   "No name given",
                                   "No such thing `%s'")
end

function minibuf_read_variable_name (fmt)
  local cp = completion_new (nil, table.keys (main_vars))

  return minibuf_vread_completion (fmt, "", cp, nil,
                                   "No variable name given",
                                   "Undefined variable name `%s'")
end

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
-- }
-- FIXME: Use Objects here and elsewhere, and add an __index
-- metamethod that works like strict.lua.

-- Make a new completions table
function completion_new (completions)
  return {completions = set.new (completions or {}), matches = {}}
end

-- Write the matches in `l' in a set of columns. The width of the
-- columns is chosen to be big enough for the longest string, with a
-- COLUMN_GAP-character gap between each column.
local COLUMN_GAP = 3
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
    if s1:sub (i, i) ~= s2:sub (i, i) then
      return i - 1
    end
  end
  return len
end

-- Arguments:
--
-- cp - the completions
-- search - the prefix to search for
--
-- Returns false if `search' is not a prefix of any completion, and true
-- otherwise.
--
-- The effect on cp is as follows:
--
--   completions - unchanged
--   matches - replaced with the list of matching completions, sorted
--   match - replaced with the longest common prefix of the matches
--
-- To format the completions for a popup, call completion_write
-- after this function.
-- FIXME: To see the completions in a popup, you should call
-- completion_popup after this method. You may want to call
-- completion_remove_suffix and/or completion_remove_prefix in between
-- to keep the list manageable. (See C Zee's completion.lua.)
function completion_try (cp, search)
  cp.matches = {}

  local fullmatch = false
  for c in pairs (cp.completions) do
    if type (c) == "string" then
      if string.sub (c, 1, #search) == search then
        table.insert (cp.matches, c)
        if c == search then
          fullmatch = true
        end
      end
    end
  end

  if #cp.matches == 0 then
    return false
  end

  table.sort (cp.matches)
  cp.match = cp.matches[1]
  local prefix_len = #cp.match
  for _, v in ipairs (cp.matches) do
    prefix_len = math.min (prefix_len, common_prefix_length (cp.match, v))
  end
  cp.match = cp.match:sub (1, prefix_len)

  return true
end

-- Popup the completion window.
function popup_completion (cp)
  popup_set (completion_write (cp, win.ewidth))
  term_display ()
end

-- FIXME: Common up minibuf_read*_name
function minibuf_read_name (fmt)
  local cp = completion_new (table.keys (env))

  return minibuf_read_completion (fmt, "", cp, nil,
                                  "No name given",
                                  "No such thing `%s'")
end

function minibuf_read_variable_name (fmt)
  local cp = completion_new (table.keys (env)) -- FIXME: filter out the commands

  return minibuf_read_completion (fmt, "", cp, nil,
                                  "No variable name given",
                                  "Undefined variable name `%s'")
end

-- Read a function name from the minibuffer.
local commands_history = history_new ()
function minibuf_read_command_name (fmt)
  local cp = completion_new (table.keys (env)) -- FIXME: filter out the variables

  return minibuf_read_completion (fmt, "", cp, commands_history,
                                  "No command name given",
                                  "Undefined command name `%s'")
end

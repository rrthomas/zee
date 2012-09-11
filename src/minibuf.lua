--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
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

files_history = history_new ()

minibuf_contents = nil


-- Minibuffer wrapper functions.

function minibuf_refresh ()
  if cur_wp then
    if minibuf_contents then
      term_minibuf_write (minibuf_contents)
    end

    -- Redisplay (and leave the cursor in the correct position).
    term_redraw_cursor ()
    term_refresh ()
  end
end

-- Clear the minibuffer.
function minibuf_clear ()
  minibuf_write ("")
end

-- Write the specified string in the minibuffer.
function minibuf_write (s)
  if s ~= minibuf_contents then
    minibuf_contents = s
    minibuf_refresh ()
  end
end

-- Write the specified error string in the minibuffer and signal an error.
function minibuf_error (s)
  minibuf_write (s)
  return ding ()
end

local function minibuf_test_in_completions (ms, cp)
  for _, v in pairs (cp.completions) do
    if v == ms then
      return true
    end
  end
  return false
end

-- Read a string from the minibuffer using a completion.
function minibuf_vread_completion (fmt, value, cp, hp, empty_err, invalid_err)
  local ms

  while true do
    ms = term_minibuf_read (fmt, value, -1, cp, hp)

    if not ms then -- Cancelled.
      execute_function ("keyboard-quit")
      break
    elseif ms == "" then
      minibuf_error (empty_err)
      ms = nil
      break
    else
      -- Complete partial words if possible.
      local comp = completion_try (cp, ms)
      if comp == "match" then
        ms = cp.match
      elseif comp == "incomplete" then
        popup_completion (cp)
      end

      if minibuf_test_in_completions (ms, cp) then
        if hp then
          add_history_element (hp, ms)
        end
        minibuf_clear ()
        break
      else
        minibuf_error (string.format (invalid_err, ms))
        waitkey (WAITKEY_DEFAULT)
      end
    end
  end

  return ms
end

-- Read a filename from the minibuffer.
function minibuf_read_filename (fmt, name, file)
  if not file and #name > 0 and name[-1] ~= '/' then
    name = name .. '/'
  end
  name = normalize_path (name)
  if name then
    name = compact_path (name)

    local pos = #name
    if file then
      pos  = pos - #file
    end
    name = term_minibuf_read (fmt, name, pos, completion_new (true), files_history)

    if name then
      name = normalize_path (name)
      if name then
        add_history_element (files_history, name)
      end
    end
  end

  return name
end

function minibuf_read_yesno (fmt)
  local errmsg = "Please answer yes or no."
  local ret = nil

  local cp = completion_new ()
  cp.completions = {"no", "yes"}
  local ms = minibuf_vread_completion (fmt, "", cp, nil, errmsg, errmsg)

  if ms then
    ret = ms == "yes"
  end

  return ret
end

function minibuf_read_yn (fmt)
  local errmsg = ""

  while true do
    minibuf_write (errmsg .. fmt)
    local key = getkeystroke (GETKEY_DEFAULT)
    if key == keycode "y" then
      return true
    elseif key == keycode "n" then
      return false
    elseif key == keycode "\\C-g" then
      return -1
    else
      errmsg = "Please answer y or n.  "
    end
  end
end

-- Read a string from the minibuffer.
function minibuf_read (fmt, value, cp, hp)
  return term_minibuf_read (fmt, value, -1, cp, hp)
end

-- Read a non-negative number from the minibuffer.
function minibuf_read_number (fmt)
  local n
  repeat
    local ms = minibuf_read (fmt, "")
      if not ms then
        execute_function ("keyboard-quit")
        break
      elseif #ms == 0 then
        n = ""
      else
        n = tonumber (ms, 10)
      end
      if not n then
        minibuf_write ("Please enter a number.")
      end
  until n

  return n
end

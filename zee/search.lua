-- Search and replace functions
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

-- Return true if there are no upper-case letters in the given string.
-- If `regex' is true, ignore escaped letters.
local function no_upper (s)
  local quote_flag = false
  for i = 1, #s do
    if s[i] == '\\' then
      quote_flag = not quote_flag
    elseif not quote_flag and s[i] == s[i]:upper () then
      return false
    end
  end
  return true
end

local re_flags = rex_gnu.flags ()
local re_find_err

local function find_substr (as, s, forward, notbol, noteol, icase)
  local ret
  local ok, r = pcall (rex_gnu.new, s, bit32.bor (re_flags.SYNTAX_EGREP, icase and re_flags.ICASE or 0))
  if ok then
    local ef = 0
    if notbol then
      ef = bit32.bor (ef, re_flags.not_bol)
    end
    if noteol then
      ef = bit32.bor (ef, re_flags.not_eol)
    end
    if not forward then
      ef = bit32.bor (ef, re_flags.backward)
    end
    local match_from, match_to = r:find (as, nil, ef)
    if match_from then
      if forward then
        ret = match_to + 1
      else
        ret = match_from
      end
    end
  else
    re_find_err = r
  end

  return ret
end

function regex_match (s, pat)
  return find_substr (s, pat, true, false, false, false) ~= nil
end

local function search (s, forward)
  if #s < 1 then
    return false
  end

  -- Attempt match.
  local o = get_buffer_pt (buf)
  local notbol = forward and o > 1
  local noteol = not forward and o <= get_buffer_size (buf)
  local downcase = get_variable ("caseless-search") and no_upper (s)
  local as = (forward and get_buffer_post_cursor or get_buffer_pre_cursor) (buf)
  local pos = find_substr (as, s, forward, notbol, noteol, downcase)
  if not pos then
    return false
  end

  goto_offset (pos + (forward and (get_buffer_pt (buf) - 1) or 0))
  thisflag.need_resync = true
  return true
end


-- Incremental search engine.
-- FIXME: Once the search is underway, "find next" is hard-wired to Ctrl-f.
-- Having it hard-wired is obviously broken, but something neutral like Return
-- would be better, or look up current binding of relevant command.
-- The proposed meaning of Escape obviates the current behaviour of Return.
local last_search
local function isearch (forward, pattern)
  local old_mark
  if buf.mark then
    old_mark = copy_marker (buf.mark)
  end

  buf.search = true

  local last = true
  local start = get_buffer_pt (buf)
  local cur = start
  while true do
    -- Make the minibuf message.
    local ms = string.format ("%sI-search%s: %s",
                              (last and "" or "Failing "),
                              forward and "" or " backward",
                              pattern)

    -- Regex error.
    if re_find_err then
      ms = ms .. string.format (" [%s]", re_find_err)
      re_find_err = nil
    end

    minibuf_write (ms)

    local c = interactive () and get_key_chord ()

    if not c then
    elseif c == keycode "Ctrl-g" then
      goto_offset (start)
      buf.mark = old_mark
      ding ()
      break
    elseif c == keycode "BACKSPACE" then
      if #pattern > 0 then
        pattern = pattern:sub (1, -2)
        cur = start
        goto_offset (start)
      else
        ding ()
      end
    elseif c == keycode "Ctrl-Alt-q" then
      minibuf_write (string.format ("%s^Q-", ms))
      pattern = pattern .. string.char (getkey_unfiltered (GETKEY_DEFAULT))
    elseif c == keycode "Ctrl-Alt-f" or c == keycode "Ctrl-f" then -- Invert direction.
      forward = c == keycode "Ctrl-f"
      if #pattern > 0 then -- Find next match.
        cur = get_buffer_pt (buf)
        last_search = pattern -- Save search string.
      elseif last_search then
        pattern = last_search
      end
    elseif c.ALT or c.CTRL or c == keycode "Return" or term_keytobyte (c) == nil then
      if #pattern > 0 then
        last_search = pattern -- Save search string.
      end
      if c ~= keycode "Return" then
        ungetkey (c)
      end
      break
    else
      pattern = pattern .. string.char (term_keytobyte (c))
    end

    if #pattern > 0 then
      goto_offset (cur)
      last = search (pattern, forward)
      if not c then
        break
      end
    else
      last = true
    end

    window_resync (win)
    term_display ()
  end

  -- done
  buf.search = false

  return true
end

Define ("edit-find",
[[
Do incremental search forward for regular expression.
As you type characters, they add to the search string and are found.
Type @kbd{Return} to exit, leaving cursor at location found.
Type @kbd{Ctrl-f} to search again forward, @kbd{Ctrl-Alt-f} to search again backward.
@kbd{Ctrl-g} when search is successful aborts and moves cursor to starting
point.
]],
  function (s)
    isearch (true, s or "")
  end
)

Define ("edit-find-backward",
[[
Do incremental search backward for regular expression.
As you type characters, they add to the search string and are found.
Type @kbd{Return} to exit, leaving cursor at location found.
Type @kbd{Ctrl-Alt-f} to search again backward, @kbd{Ctrl-f} to search again forward.
@kbd{Ctrl-g} when search is successful aborts and moves cursor to starting
point.
]],
  function (s)
    isearch (false, s or "")
  end
)

-- Check the case of a string.
-- Returns "uppercase" if it is all upper case, "capitalized" if just
-- the first letter is, and nil otherwise.
local function check_case (s)
  if regex_match (s, "^[[:upper:]]+$") then
    return "uppercase"
  elseif regex_match (s, "^[[:upper:]][^[:upper:]]*") then
    return "capitalized"
  end
end

-- Recase str according to newcase.
local function recase (s, newcase)
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

-- FIXME: Make edit-replace run on selection.
Define ("edit-replace",
[[
Replace occurrences of a regular expression with other text.
As each match is found, the user must type a character saying
what to do with it.
]],
  function (find)
    local find = find or (interactive () and minibuf_read ("Query replace string: "))
    if not find then
      return ding ()
    end
    local find_no_upper = no_upper (find)

    local repl = interactive () and minibuf_read (string.format ("Query replace `%s' with: ", find))
    if repl == "" then
      ding ()
    end

    local noask = false
    local count = 0
    local ok = true
    while search (find, true) do
      local c = keycode ' '

      if not noask then
        if thisflag.need_resync then
          window_resync (win)
        end
        minibuf_write (string.format ("Replace `%s' with `%s' (y, n, !, ., q)? ", find, repl))
        c = get_key_chord ()
        minibuf_clear ()

        if c == keycode "q" then -- Quit immediately.
          break
        elseif c == keycode "Ctrl-g" then
          ding ()
          break
        elseif c == keycode "!" then -- Replace all without asking.
          noask = true
        end
      end

      if c == keycode " " or c == keycode "y" or c == keycode "." or c == keycode "!" then
        -- Perform replacement.
        count = count + 1
        local case_repl = repl
        local r = region_new (get_buffer_pt (buf) - #find, get_buffer_pt (buf))
        if find_no_upper and get_variable ("case-replace") then
          local case_type = check_case (get_buffer_region (buf, r))
          if case_type then
            case_repl = recase (repl, case_type)
          end
        end
        local m = cursor_marker ()
        goto_offset (r.start)
        replace_string (#find, case_repl)
        goto_offset (m.o)

        if c == keycode "." then -- Replace and quit.
          break
        end
      elseif c ~= keycode "n" and c ~= keycode "Return" and c ~= keycode "Delete" then
        ungetkey (c)
        ok = false
        break
      end
    end

    if thisflag.need_resync then
      window_resync (win)
    end

    if ok then
      minibuf_write (string.format ("Replaced %d occurrence%s", count, count ~= 1 and "s" or ""))
    end

    return not ok
  end
)

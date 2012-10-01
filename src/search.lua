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
    elseif not quote_flag and s[i] == string.upper (s[i]) then
      return false
    end
  end
  return true
end

local re_flags = rex_gnu.flags ()
local re_find_err

function find_substr (as, s, from, to, forward, notbol, noteol, regex, icase)
  local ret

  if not regex then
    s = string.gsub (s, "([$^.*[%]\\+?])", "\\%1")
  end

  local ok, r = pcall (rex_gnu.new, s, icase and re_flags.ICASE or 0)
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
    local match_from, match_to = r:find (as:sub (from, to), nil, ef)
    if match_from then
      if forward then
        ret = match_to + from
      else
        ret = match_from + from - 1
      end
    end
  else
    re_find_err = r
  end

  return ret
end

local function search (o, s, forward)
  if #s < 1 then
    return false
  end

  -- Attempt match.
  local notbol = forward and o > 1
  local noteol = not forward and o <= get_buffer_size (cur_bp)
  local from = forward and o or 1
  local to = forward and get_buffer_size (cur_bp) + 1 or o - 1
  local downcase = get_variable ("case-fold-search") and no_upper (s)
  -- FIXME: The following is horribly inefficient, but re_search_2 also concatenates copies of the strings
  local pos = find_substr (tostring (get_buffer_pre_point (cur_bp)) .. tostring (get_buffer_post_point (cur_bp)), s, from, to, forward, notbol, noteol, true, downcase)
  if not pos then
    return false
  end

  goto_offset (pos)
  thisflag.need_resync = true
  return true
end

local last_search

function do_search (forward, pattern)
  local ok = false

  if not pattern then
    pattern = minibuf_read (string.format ("Search%s: ", forward and "" or " backward"), last_search or "")
  end

  if not pattern then
    return execute_function ("keyboard-quit")
  end
  if #pattern > 0 then
    last_search = pattern

    if not search (get_buffer_pt (cur_bp), pattern, forward) then
      minibuf_error (string.format ("Search failed: \"%s\"", pattern))
    else
      ok = true
    end
  end

  return ok
end


-- Incremental search engine.
local function isearch (forward)
  local old_mark
  if cur_bp.mark then
    old_mark = copy_marker (cur_bp.mark)
  end

  cur_bp.isearch = true

  local last = true
  local pattern = ""
  local start = get_buffer_pt (cur_bp)
  local cur = start
  while true do
    -- Make the minibuf message.
    local buf = string.format ("%sI-search%s: %s",
                               (last and "" or "Failing "),
                               forward and "" or " backward",
                               pattern)

    -- Regex error.
    if re_find_err then
      if string.sub (re_find_err, 1, 10) == "Premature " or
        string.sub (re_find_err, 1, 10) == "Unmatched " or
        string.sub (re_find_err, 1, 8) == "Invalid " then
        re_find_err = "incomplete input"
      end
      buf = buf .. string.format (" [%s]", re_find_err)
      re_find_err = nil
    end

    minibuf_write (buf)

    local c = getkey (GETKEY_DEFAULT)

    if c == keycode "C-g" then
      goto_offset (start)
      thisflag.need_resync = true

      -- Quit.
      execute_function ("keyboard-quit")

      -- Restore old mark position.
      if cur_bp.mark then
        unchain_marker (cur_bp.mark)
      end
      cur_bp.mark = old_mark
      break
    elseif c == keycode "BACKSPACE" then
      if #pattern > 0 then
        pattern = string.sub (pattern, 1, -2)
        cur = start
        goto_offset (start)
        thisflag.need_resync = true
      else
        ding ()
      end
    elseif c == keycode "C-q" then
      minibuf_write (string.format ("%s^Q-", buf))
      pattern = pattern .. string.char (getkey_unfiltered (GETKEY_DEFAULT))
    elseif c == keycode "C-r" or c == keycode "C-s" then
      -- Invert direction.
      if c == keycode "C-r" then
        forward = false
      elseif c == keycode "C-s" then
        forward = true
      end
      if #pattern > 0 then
        -- Find next match.
        cur = get_buffer_pt (cur_bp)
        -- Save search string.
        last_search = pattern
      elseif last_search then
        pattern = last_search
      end
    elseif c.META or c.CTRL or c == keycode "RET" or term_keytobyte (c) == nil then
      if c == keycode "RET" and #pattern == 0 then
        do_search (forward)
      else
        if #pattern > 0 then
          -- Save mark.
          set_mark ()
          cur_bp.mark.o = start

          -- Save search string.
          last_search = pattern

          minibuf_write ("Mark saved when search started")
        else
          minibuf_clear ()
        end
        if c ~= keycode "RET" then
          ungetkey (c)
        end
      end
      break
    else
      pattern = pattern .. string.char (term_keytobyte (c))
    end

    if #pattern > 0 then
      last = search (cur, pattern, forward)
    else
      last = true
    end

    if thisflag.need_resync then
      window_resync (cur_wp)
      term_redisplay ()
    end
  end

  -- done
  cur_bp.isearch = false

  return true
end

Defun ("edit-find",
[[
Do incremental search forward for regular expression.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type @kbd{C-s} to search again forward, @kbd{C-r} to search again backward.
@kbd{C-g} when search is successful aborts and moves point to starting point.
]],
  function (s)
    return (_interactive and isearch or do_search) (true, s)
  end
)

Defun ("edit-find-backward",
[[
Do incremental search backward for regular expression.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type @kbd{C-r} to search again backward, @kbd{C-s} to search again forward.
@kbd{C-g} when search is successful aborts and moves point to starting point.
]],
  function (s)
    return (_interactive and isearch or do_search) (false, s)
  end
)

-- Check the case of a string.
-- Returns "uppercase" if it is all upper case, "capitalized" if just
-- the first letter is, and nil otherwise.
local function check_case (s)
  if s:match ("^%u+$") then
    return "uppercase"
  elseif s:match ("^%u%U*") then
    return "capitalized"
  end
end

Defun ("edit-replace",
[[
Replace occurrences of a string with other text.
As each match is found, the user must type a character saying
what to do with it.
]],
  function ()
    local find = minibuf_read ("Query replace string: ", "")
    if not find then
      return execute_function ("keyboard-quit")
    end
    if find == "" then
      return false
    end
    local find_no_upper = no_upper (find)

    local repl = minibuf_read (string.format ("Query replace `%s' with: ", find), "")
    if not repl then
      execute_function ("keyboard-quit")
    end

    local noask = false
    local count = 0
    local ok = true
    while search (get_buffer_pt (cur_bp), find, true) do
      local c = string.byte (' ')

      if not noask then
        if thisflag.need_resync then
          window_resync (cur_wp)
        end
        while true do
          minibuf_write (string.format ("Query replacing `%s' with `%s' (y, n, !, ., q)? ", find, repl))
          c = getkey (GETKEY_DEFAULT)
          if c == keycode "C-g" or c == keycode "RET" or c == keycode " " or c == keycode "y" or c == keycode "n"  or c == keycode "q" or c == keycode "." or c == keycode "!" then
            break
          end
          minibuf_error ("Please answer y, n, !, . or q.")
          waitkey ()
        end
        minibuf_clear ()

        if c == keycode "q" then -- Quit immediately.
          break
        elseif c == keycode "C-g" then
          ok = execute_function ("keyboard-quit")
          break
        elseif c == keycode "!" then -- Replace all without asking.
          noask = true
        end
      end

      if c ~= keycode "n" and c ~= keycode "RET" and c ~= keycode "DELETE" then -- Do not replace.
        -- Perform replacement.
        count = count + 1
        local case_repl = repl
        local r = region_new (get_buffer_pt (cur_bp) - #find, get_buffer_pt (cur_bp))
        if find_no_upper and get_variable ("case-replace") then
          local case_type = check_case (tostring (get_buffer_region (cur_bp, r))) -- FIXME
          if case_type then
            case_repl = recase (repl, case_type)
          end
        end
        local m = point_marker ()
        goto_offset (r.start)
        replace_astr (#find, AStr (case_repl))
        goto_offset (m.o)
        unchain_marker (m)

        if c == keycode "." then -- Replace and quit.
          break
        end
      end
    end

    if thisflag.need_resync then
      window_resync (cur_wp)
    end

    if ok then
      minibuf_write (string.format ("Replaced %d occurrence%s", count, count ~= 1 and "s" or ""))
    end

    return ok
  end
)

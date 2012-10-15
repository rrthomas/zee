-- Minibuffer
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

local function term_minibuf_write (s)
  term_move (term_height () - 1, 0)
  term_clrtoeol ()
  term_addstr (s:sub (1, math.min (#s, term_width ())))
end

-- FIXME: Turn term_minibuf_read inside out so it's a minor mode
function term_minibuf_read (prompt, value, pos, cp, hp)
  local quit = false

  Define ("minibuf-insert-character", "",
          function ()
            local key = term_keytobyte (lastkey ())
            if not key then
              ding ()
              return false
            end
            value = value:sub (1, pos) .. string.char (key) .. value:sub (pos + 1)
            pos = pos + 1
          end)

  Define ("minibuf-exit", "",
          function ()
            quit = true
          end)

  Define ("minibuf-suspend", "",
          function ()
            execute_command ("file-suspend")
          end)

  Define ("minibuf-quit", "",
          function ()
            value = nil
            quit = true
          end)

  Define ("minibuf-move-start-line", "",
          function ()
            pos = 0
          end)

  Define ("minibuf-move-end-line", "",
          function ()
            pos = #value
          end)

  Define ("minibuf-move-previous-character", "",
          function ()
            if pos > 0 then
              pos = pos - 1
            else
              ding ()
            end
          end)

  Define ("minibuf-move-next-character", "",
          function ()
            if pos < #value then
              pos = pos + 1
            else
              ding ()
            end
          end)

  Define ("minibuf-delete-previous-character", "",
          function ()
            if pos > 0 then
              value = value:sub (1, pos - 1) .. value:sub (pos + 1)
              pos = pos - 1
            else
              ding ()
            end
          end)

  Define ("minibuf-delete-next-character", "",
          function ()
            if pos < #value then
              value = value:sub (1, pos) .. value:sub (pos + 2)
            else
              ding ()
            end
          end)

  Define ("minibuf-previous-page", "",
          function ()
            if cp == nil then
              ding ()
            else
              popup_scroll_down ()
            end
          end)

  Define ("minibuf-next-page", "",
          function ()
            if cp == nil then
              ding ()
            else
              popup_scroll_up ()
            end
          end)

  Define ("minibuf-complete", "",
          function ()
            if not cp or #cp.matches == 0 then
              ding ()
            else
              if value ~= cp.match then
                value = cp.match
                pos = #value
              else
                popup_scroll_down_and_loop ()
              end
            end
          end)

  local minibuf_bindings = {}
  bindings = minibuf_bindings
  bind_printing_chars ("minibuf-insert-character")
  key_bind ("RET", "minibuf-exit")
  key_bind ("C-z", "minibuf-suspend")
  key_bind ("C-g", "minibuf-quit")
  key_bind ("C-a", "minibuf-move-start-line")
  key_bind ("HOME", "minibuf-move-start-line")
  key_bind ("C-e", "minibuf-move-end-line")
  key_bind ("END", "minibuf-move-end-line")
  key_bind ("C-b", "minibuf-move-previous-character")
  key_bind ("LEFT", "minibuf-move-previous-character")
  key_bind ("C-f", "minibuf-move-next-character")
  key_bind ("RIGHT", "minibuf-move-next-character")
  key_bind ("BACKSPACE", "minibuf-delete-previous-character")
  key_bind ("C-d", "minibuf-delete-next-character")
  key_bind ("DELETE", "minibuf-delete-next-character")
  key_bind ("M-v", "minibuf-previous-page")
  key_bind ("PAGEUP", "minibuf-previous-page")
  key_bind ("C-v", "minibuf-next-page")
  key_bind ("PAGEDOWN", "minibuf-next-page")
  key_bind ("TAB", "minibuf-complete")

  local saved
  pos = pos or #value

  local completion_text, old_completion_text
  repeat
    if cp and (not old_completion_text or old_completion_text ~= completion_text) then
      popup_completion (cp)
      completion_try (cp, value)
      local completion_text = completion_write (cp, win.ewidth)
      popup_set (completion_text)
      old_completion_text = completion_text
    end

    term_minibuf_write (prompt)

    local w, h = term_width (), term_height ()
    local margin = 1
    local n = 0

    if #prompt + pos + 1 >= w then
      margin = margin + 1
      term_addstr ('$')
      n = pos - pos % (w - #prompt - 2)
    end

    term_addstr (value:sub (n + 1, math.min (w - #prompt - margin, #value - n)))

    if #value - n >= w - #prompt - margin then
      term_move (h - 1, w - 1)
      term_addstr ('$')
    end

    term_move (h - 1, #prompt + margin - 1 + pos % (w - #prompt - margin))
    term_refresh ()

    local key = get_key_chord (true)
    local name = binding_to_command (key)
    if command_exists (name) then
      call_command (name)
    else
      ding ()
    end
  until quit

  minibuf_clear ()
  if cp then
    popup_clear ()
    term_display ()
  end
  bindings = root_bindings
  return value
end


minibuf_contents = nil


-- Minibuffer wrapper functions.

function minibuf_refresh ()
  if win then
    if minibuf_contents then
      term_minibuf_write (minibuf_contents)
    end
    term_refresh ()
  end
end

-- Clear the minibuffer.
function minibuf_clear ()
  term_minibuf_write ("")
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

-- Read a string from the minibuffer using a completion.
function minibuf_read_completion (fmt, cp, class_name)
  local empty_err = "No " .. class_name .. " given"
  local invalid_err = "There is no " .. class_name .. " named `%s'"
  local ms

  while true do
    ms = term_minibuf_read (fmt, "", nil, cp)

    if not ms then -- Cancelled.
      ding ()
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

      if cp.completions:member (ms) then
        minibuf_clear ()
        break
      else
        minibuf_error (string.format (invalid_err, ms))
        waitkey ()
      end
    end
  end

  return ms
end

-- Read a string from the minibuffer.
function minibuf_read (fmt, value, cp, hp)
  return term_minibuf_read (fmt, value, nil, cp, hp)
end

-- Read a non-negative number from the minibuffer.
function minibuf_read_number (fmt)
  local n
  repeat
    local ms = minibuf_read (fmt, "")
      if not ms then
        ding ()
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

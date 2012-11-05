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
end

-- FIXME: Turn minibuf_read inside out so it's a minor mode
-- Read a string from the minibuffer.
function minibuf_read (prompt, cp)
  local quit, value, pos = false, "", 0

  -- FIXME: Stop these definitions appearing in global environment
  -- (Have nested environments)
  Define ("minibuf-insert-character", "",
          function ()
            local key = term_keytobyte (lastkey ())
            if not key then
              ding ()
              return true
            end
            value = value:sub (1, pos) .. string.char (key) .. value:sub (pos + 1)
            pos = pos + 1
          end)

  Define ("minibuf-exit", "",
          function ()
            quit = true
          end)

  Define ("minibuf-quit", "",
          function ()
            value = ""
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
  key_bind ("Return", "minibuf-exit")
  key_bind ("Ctrl-z", "file-suspend")
  key_bind ("Ctrl-g", "minibuf-quit")
  key_bind ("Ctrl-a", "minibuf-move-start-line")
  key_bind ("Home", "minibuf-move-start-line")
  key_bind ("Ctrl-e", "minibuf-move-end-line")
  key_bind ("End", "minibuf-move-end-line")
  key_bind ("Ctrl-b", "minibuf-move-previous-character")
  key_bind ("Left", "minibuf-move-previous-character")
  key_bind ("Ctrl-f", "minibuf-move-next-character")
  key_bind ("Right", "minibuf-move-next-character")
  key_bind ("Backspace", "minibuf-delete-previous-character")
  key_bind ("Ctrl-d", "minibuf-delete-next-character")
  key_bind ("Delete", "minibuf-delete-next-character")
  key_bind ("Alt-v", "minibuf-previous-page")
  key_bind ("PageUp", "minibuf-previous-page")
  key_bind ("Ctrl-v", "minibuf-next-page")
  key_bind ("PageDown", "minibuf-next-page")
  key_bind ("Tab", "minibuf-complete")

  local saved
  repeat
    if cp then
      completion_try (cp, value)
      popup_completion (cp)
    end

    minibuf_write (prompt)
    minibuf_refresh ()

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


-- Minibuffer wrapper functions.

minibuf_contents = nil

function minibuf_refresh ()
  if win then
    if minibuf_contents then
      term_move (term_height () - 1, 0)
      term_clrtoeol ()
      term_addstr (minibuf_contents:sub (1, math.min (#minibuf_contents, term_width ())))
    end
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

-- Read a string from the minibuffer using a completion.
function minibuf_read_completion (fmt, cp, class_name)
  while true do
    local ms = minibuf_read (fmt, cp)
    if ms == "" then -- Cancelled.
      ding ()
      return nil
    else
      -- Complete partial words if possible.
      if completion_try (cp, ms) then
        ms = cp.match
      end

      if cp.completions:member (ms) then
        minibuf_clear ()
        return ms
      else
        minibuf_error (string.format ("There is no " .. class_name .. " named `%s'", ms))
        waitkey ()
      end
    end
  end
end

-- Read a non-negative number from the minibuffer.
function minibuf_read_number (prompt)
  local n
  repeat
    local ms = minibuf_read (prompt)
      if not ms then
        return ding ()
      else
        n = tonumber (ms, 10)
      end
      if not n then
        minibuf_write ("Please enter a number.")
      end
  until n

  return n
end

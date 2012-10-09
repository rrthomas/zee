-- Minibuffer handling
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

local function draw_minibuf_read (prompt, value, pointo)
  term_minibuf_write (prompt)

  local w, h = term_width (), term_height ()
  local margin = 1
  local n = 0

  if #prompt + pointo + 1 >= w then
    margin = margin + 1
    term_addstr ('$')
    n = pointo - pointo % (w - #prompt - 2)
  end

  term_addstr (string.sub (value, n + 1, math.min (w - #prompt - margin, #value - n)))

  if #value - n >= w - #prompt - margin then
    term_move (h - 1, w - 1)
    term_addstr ('$')
  end

  term_move (h - 1, #prompt + margin - 1 + pointo % (w - #prompt - margin))

  term_refresh ()
end

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
            elseif cp.poppedup then
              popup_scroll_down ()
            end
          end)

  Define ("minibuf-next-page", "",
          function ()
            if cp == nil then
              ding ()
            elseif cp.poppedup then
              popup_scroll_up ()
            end
          end)

  Define ("minibuf-history-previous", "",
          function ()
            if hp then
              local elem = previous_history_element (hp)
              if elem then
                if not saved then
                  saved = value
                end
                value = elem
              end
            end
          end)

  Define ("minibuf-history-next", "",
          function ()
            if hp then
              local elem = next_history_element (hp)
              if elem then
                value = elem
              elseif saved then
                value = saved
                saved = nil
              end
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
  key_bind ("M-p", "minibuf-history-previous")
  key_bind ("UP", "minibuf-history-previous")
  key_bind ("M-n", "minibuf-history-next")
  key_bind ("DOWN", "minibuf-history-next")
  key_bind ("TAB", "minibuf-complete")

  if hp then
    history_prepare (hp)
  end

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

    draw_minibuf_read (prompt, value, pos)
    local key = get_key_chord (true)
    local name = get_command_by_key (key)
    if command_exists (name) then
      call_command (name)
    else
      ding ()
    end
  until quit

  minibuf_clear ()
  if cp then
    popup_clear ()
    term_redisplay ()
  end
  bindings = root_bindings
  return value
end

function term_minibuf_write (s)
  term_move (term_height () - 1, 0)
  term_clrtoeol ()
  term_addstr (string.sub (s, 1, math.min (#s, term_width ())))
end

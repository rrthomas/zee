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

    local c = getkeystroke (GETKEY_DEFAULT)
    if c == nil or c == keycode "RET" then
    elseif c == keycode "C-z" then
      execute_command ("file-suspend")
    elseif c == keycode "C-g" then
      value = nil
      break
    elseif c == keycode "C-a" or c == keycode "HOME" then
      pos = 0
    elseif c == keycode "C-e" or c == keycode "END" then
      pos = #value
    elseif c == keycode "C-b" or c == keycode "LEFT" then
      if pos > 0 then
        pos = pos - 1
      else
        ding ()
      end
    elseif c == keycode "C-f" or c == keycode "RIGHT" then
      if pos < #value then
        pos = pos + 1
      else
        ding ()
      end
    elseif c == keycode "BACKSPACE" then
      if pos > 0 then
        value = value:sub (1, pos - 1) .. value:sub (pos + 1)
        pos = pos - 1
      else
        ding ()
      end
    elseif c == keycode "C-d" or c == keycode "DELETE" then
      if pos < #value then
        value = value:sub (1, pos) .. value:sub (pos + 2)
      else
        ding ()
      end
    elseif c == keycode "M-v" or c == keycode "PAGEUP" then
      if cp == nil then
        ding ()
      elseif cp.poppedup then
        popup_scroll_down ()
      end
    elseif c == keycode "C-v" or c == keycode "PAGEDOWN" then
      if cp == nil then
        ding ()
      elseif cp.poppedup then
        popup_scroll_up ()
      end
    elseif c == keycode "M-p" or c == keycode "UP" then
      if hp then
        local elem = previous_history_element (hp)
        if elem then
          if not saved then
            saved = value
          end
          value = elem
        end
      end
    elseif c == keycode "M-n" or c == keycode "DOWN" then
      if hp then
        local elem = next_history_element (hp)
        if elem then
          value = elem
        elseif saved then
          value = saved
          saved = nil
        end
      end
    elseif c == keycode "TAB" then
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
    else
      if c.META or c.CTRL or not posix.isprint (c.code) then
        ding ()
      else
        value = value:sub (1, pos) .. string.char (c.code) .. value:sub (pos + 1)
        pos = pos + 1
      end
    end

  until c == keycode "RET" or c == keycode "C-g"

  minibuf_clear ()
  if cp then
    popup_clear ()
    term_redisplay ()
  end
  return value
end

function term_minibuf_write (s)
  term_move (term_height () - 1, 0)
  term_clrtoeol ()
  term_addstr (string.sub (s, 1, math.min (#s, term_width ())))
end

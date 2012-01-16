-- Curses terminal
--
-- Copyright (c) 2009-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

local attr_map, codetokey, keytocode, key_buf

function term_init ()
  local functable_mt = {
    __call = function (t, k)
               return t[k]
             end,
  }

  curses.initscr ()

  attr_map = {
    [FONT_REVERSE] = curses.A_REVERSE,
    [FONT_UNDERLINE] = curses.A_UNDERLINE,
  }

  key_buf = {}

  -- from curses keycodes to zile keycodes
  codetokey = {
    [0]                    = keycode "\\C-@",
    [1]                    = keycode "\\C-a",
    [2]                    = keycode "\\C-b",
    [3]                    = keycode "\\C-c",
    [4]                    = keycode "\\C-d",
    [5]                    = keycode "\\C-e",
    [6]                    = keycode "\\C-f",
    [7]                    = keycode "\\C-g",
    [8]                    = keycode "\\C-h",
    [9]                    = keycode "\\TAB",
    [10]                   = keycode "\\C-j",
    [11]                   = keycode "\\C-k",
    [12]                   = keycode "\\C-l",
    [13]                   = keycode "\\RET",
    [14]                   = keycode "\\C-n",
    [15]                   = keycode "\\C-o",
    [16]                   = keycode "\\C-p",
    [17]                   = keycode "\\C-q",
    [18]                   = keycode "\\C-r",
    [19]                   = keycode "\\C-s",
    [20]                   = keycode "\\C-t",
    [21]                   = keycode "\\C-u",
    [22]                   = keycode "\\C-v",
    [23]                   = keycode "\\C-w",
    [24]                   = keycode "\\C-x",
    [25]                   = keycode "\\C-y",
    [26]                   = keycode "\\C-z",
    [27]                   = KBD_META,
    [31]                   = keycode "\\C-_",
    [127]                  = keycode "\\BACKSPACE",
    [curses.KEY_DC]        = keycode "\\DELETE",
    [curses.KEY_DOWN]      = keycode "\\DOWN",
    [curses.KEY_END]       = keycode "\\END",
    [curses.KEY_F1]        = keycode "\\F1",
    [curses.KEY_F2]        = keycode "\\F2",
    [curses.KEY_F3]        = keycode "\\F3",
    [curses.KEY_F4]        = keycode "\\F4",
    [curses.KEY_F5]        = keycode "\\F5",
    [curses.KEY_F6]        = keycode "\\F6",
    [curses.KEY_F7]        = keycode "\\F7",
    [curses.KEY_F8]        = keycode "\\F8",
    [curses.KEY_F9]        = keycode "\\F9",
    [curses.KEY_F10]       = keycode "\\F10",
    [curses.KEY_F11]       = keycode "\\F11",
    [curses.KEY_F12]       = keycode "\\F12",
    [curses.KEY_HOME]      = keycode "\\HOME",
    [curses.KEY_IC]        = keycode "\\INSERT",
    [curses.KEY_LEFT]      = keycode "\\LEFT",
    [curses.KEY_NPAGE]     = keycode "\\PAGEDOWN",
    [curses.KEY_PPAGE]     = keycode "\\PAGEUP",
    [curses.KEY_RIGHT]     = keycode "\\RIGHT",
    [curses.KEY_SUSPEND]   = keycode "\\C-z",
    [curses.KEY_UP]        = keycode "\\UP"
  }

  local kbs = curses.tigetstr("kbs")
  if not kbs or #kbs ~= 1 then
    kbs = string.char(127)
  end

  codetokey[curses.KEY_BACKSPACE] = codetokey[string.byte (kbs)]

  for c=0,0x7f do
    codetokey[c] = codetokey[c] or keycode (string.char (c))
    if not codetokey[c + 0x100] then
      codetokey[c + 0x100] = keycode ("\\M-" .. string.char (c))
    end
  end

  setmetatable (codetokey, functable_mt)

  -- FIXME: How do we handle an unget on e.g. KBD_F1?
  --        shouldn't crash Zile with: (execute-kbd-macro "\C-q\F1")

  keytocode = table.merge (table.invert (codetokey), {
                            [keycode "\\C-h"]       = 8,
                            [keycode "\t"]          = 9,
                            [keycode "\\t"]         = 9,
                            [keycode "\\C-z"]       = 26,
                            [keycode "\\e"]         = 27,
                            [keycode " "]           = 32,
                            [keycode "\\SPC"]       = 32,
                            [keycode "\\BACKSPACE"] = 127,
                          })
  setmetatable (keytocode, functable_mt)

  curses.echo (false)
  curses.nl (false)
  curses.raw (true)
  curses.stdscr ():meta (true)
  curses.stdscr ():intrflush (false)
  curses.stdscr ():keypad (true)
end

function term_close ()
  curses.endwin ()
end

function term_reopen ()
  curses.flushinp ()
  -- FIXME: implement def_shell_mode in lcurses
  --curses.def_shell_mode ()
  curses.doupdate ()
end

local function get_char (delay)
  if #key_buf > 0 then
    return table.remove (key_buf)
  end

  curses.stdscr ():timeout (delay)

  local c
  repeat
    c = curses.stdscr ():getch ()
    if curses.KEY_RESIZE == c then
      resize_windows ()
    end
  until curses.KEY_RESIZE ~= c

  curses.stdscr ():timeout (-1)

  return c
end

function term_getkey (delay)
  local key = codetokey (get_char (delay))
  while KBD_META == key do
    key = bit.bor (codetokey (get_char (GETKEY_DEFAULT)), KBD_META)
  end
  return key
end

function term_getkey_unfiltered (delay)
  curses.stdscr ():keypad (false)
  local c = get_char (delay)
  curses.stdscr ():keypad (true)
  return c
end

local function keytocodes (key)
  local codevec = {}

  if key ~= nil then
    if bit.band (key, KBD_META) ~= 0 then
      table.insert (codevec, 27)
      key = bit.band (key, bit.bnot (KBD_META))
    end

    local code = keytocode (key)
    if code then
      table.insert (codevec, code)
    end
  end

  return codevec
end

function term_ungetkey (key)
  key_buf = list.concat (key_buf, list.reverse (keytocodes (key)))
end

function term_buf_len ()
  return #key_buf
end

function term_move (y, x)
  curses.stdscr ():move (y, x)
end

function term_clrtoeol ()
  curses.stdscr ():clrtoeol ()
end

function term_refresh ()
  curses.stdscr ():refresh ()
end

function term_clear ()
  curses.stdscr ():clear ()
end

function term_addch (c)
  curses.stdscr ():addch (c)
end

function term_addstr (s)
  curses.stdscr ():addstr (s)
end

function term_attrset (attrs)
  local cattrs = 0
  for i, v in pairs (attr_map) do
    if bit.band (attrs, i) ~= 0 then
      cattrs = bit.bor (cattrs, v)
    end
  end
  curses.stdscr ():attrset (cattrs)
end

function term_beep ()
  curses.beep ()
end

function term_width ()
  return curses.cols ()
end

function term_height ()
  return curses.lines ()
end

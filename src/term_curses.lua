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

local key_buf

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
  curses.stdscr ():addch (bit.band (c, bit.bnot (curses.A_ATTRIBUTES)))
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

local codetokey_map, keytocode_map


local function CTRL (c)
  return bit.bor (KBD_CTRL, string.byte (c))
end

function term_init ()
  curses.initscr ()

  attr_map = {
    [FONT_REVERSE] = curses.A_REVERSE,
    [FONT_UNDERLINE] = curses.A_UNDERLINE,
  }

  -- from curses keycodes to zile keycodes
  codetokey_map = {
    [0]                    = CTRL ('@'),
    [1]                    = CTRL ('a'),
    [2]                    = CTRL ('b'),
    [3]                    = CTRL ('c'),
    [4]                    = CTRL ('d'),
    [5]                    = CTRL ('e'),
    [6]                    = CTRL ('f'),
    [7]                    = CTRL ('g'),
    [8]                    = CTRL ('h'),
    [9]                    = KBD_TAB,
    [10]                   = CTRL ('j'),
    [11]                   = CTRL ('k'),
    [12]                   = CTRL ('l'),
    [13]                   = KBD_RET,
    [14]                   = CTRL ('n'),
    [15]                   = CTRL ('o'),
    [16]                   = CTRL ('p'),
    [17]                   = CTRL ('q'),
    [18]                   = CTRL ('r'),
    [19]                   = CTRL ('s'),
    [20]                   = CTRL ('t'),
    [21]                   = CTRL ('u'),
    [22]                   = CTRL ('v'),
    [23]                   = CTRL ('w'),
    [24]                   = CTRL ('x'),
    [25]                   = CTRL ('y'),
    [26]                   = CTRL ('z'),
    [27]                   = KBD_META,
    [31]                   = CTRL ('_'),
    [127]                  = KBD_BS,     -- delete char left of cursor
    [curses.KEY_DC]        = KBD_DEL,    -- delete char under cursor
    [curses.KEY_DOWN]      = KBD_DOWN,
    [curses.KEY_END]       = KBD_END,
    [curses.KEY_F1]        = KBD_F1,
    [curses.KEY_F2]        = KBD_F2,
    [curses.KEY_F3]        = KBD_F3,
    [curses.KEY_F4]        = KBD_F4,
    [curses.KEY_F5]        = KBD_F5,
    [curses.KEY_F6]        = KBD_F6,
    [curses.KEY_F7]        = KBD_F7,
    [curses.KEY_F8]        = KBD_F8,
    [curses.KEY_F9]        = KBD_F9,
    [curses.KEY_F10]       = KBD_F10,
    [curses.KEY_F11]       = KBD_F11,
    [curses.KEY_F12]       = KBD_F12,
    [curses.KEY_HOME]      = KBD_HOME,
    [curses.KEY_IC]        = KBD_INS,    -- INSERT
    [curses.KEY_LEFT]      = KBD_LEFT,
    [curses.KEY_NPAGE]     = KBD_PGDN,
    [curses.KEY_PPAGE]     = KBD_PGUP,
    [curses.KEY_RIGHT]     = KBD_RIGHT,
    [curses.KEY_SUSPEND]   = CTRL ('z'),
    [curses.KEY_UP]        = KBD_UP,
  }

  local kbs = curses.tigetstr("kbs")
  if (nil == kbs or 1 ~= #kbs) then
    kbs = string.char(127)
  end

  codetokey_map[curses.KEY_BACKSPACE] = codetokey_map[string.byte(kbs)]

  keytocode_map = table.invert (codetokey_map)

  -- FIXME: How do we handle an unget on e.g. KBD_F1?
  --        shouldn't crash Zile with: (execute-kbd-macro "\C-q\F1")

  keytocode_map = table.merge (keytocode_map, {
                                [CTRL ('h')] = 8,
                                [CTRL ('z')] = 26,
                                [KBD_BS] = 127,
                              })

  term_set_size (curses.cols (), curses.lines ())
  curses.echo (false)
  curses.nl (false)
  curses.raw (true)
  curses.stdscr ():meta (true)
  curses.stdscr ():intrflush (false)
  curses.stdscr ():keypad (true)
  key_buf = {}
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

local function codetokey (c)
  local ret
  if codetokey_map[c] then
    ret = codetokey_map[c]
  elseif nil == c or c > 0xff or c < 0 then
    ret = KBD_NOKEY -- Undefined behaviour.
  else
    ret = c
  end
  return ret
end

local function keytocodes (key)
  local codevec = {}

  if key ~= KBD_NOKEY then
    if bit.band (key, KBD_META) ~= 0 then
      table.insert (codevec, 27)
      key = bit.band (key, bit.bnot (KBD_META))
    end

    if keytocode_map[key] then
      table.insert (codevec, keytocode_map[key])
    elseif key < 0x100 then
      table.insert (codevec, key)
    end
  end

  return codevec
end

local function get_char (delay)
  local c

  if #key_buf > 0 then
    return table.remove (key_buf)
  else
    curses.stdscr ():timeout (delay)

    repeat
      c = curses.stdscr ():getch ()

      if curses.KEY_RESIZE == c then
        term_set_size (curses.cols (), curses.lines ())
        resize_windows ()
      end
    until curses.KEY_RESIZE ~= c

    curses.stdscr ():timeout (-1)
  end

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

function term_ungetkey (key)
  key_buf = list.concat (key_buf, list.reverse (keytocodes (key)))
end

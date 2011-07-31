-- Curses terminal
--
-- Copyright (c) 2009-2011 Free Software Foundation, Inc.
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

function term_attrset (attr)
  curses.stdscr ():attrset (attr == FONT_REVERSE and curses.A_REVERSE or 0)
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

  keytocode_map = {
    [CTRL ('@')] = 0,
    [CTRL ('a')] = 1,
    [CTRL ('b')] = 2,
    [CTRL ('c')] = 3,
    [CTRL ('d')] = 4,
    [CTRL ('e')] = 5,
    [CTRL ('f')] = 6,
    [CTRL ('g')] = 7,
    [CTRL ('h')] = 8,
    [KBD_TAB] = 9,
    [CTRL ('j')] = 10,
    [CTRL ('k')] = 11,
    [CTRL ('l')] = 12,
    [KBD_RET] = 13,
    [CTRL ('n')] = 14,
    [CTRL ('o')] = 15,
    [CTRL ('p')] = 16,
    [CTRL ('q')] = 17,
    [CTRL ('r')] = 18,
    [CTRL ('s')] = 19,
    [CTRL ('t')] = 20,
    [CTRL ('u')] = 21,
    [CTRL ('v')] = 22,
    [CTRL ('w')] = 23,
    [CTRL ('x')] = 24,
    [CTRL ('y')] = 25,
    [CTRL ('z')] = 26,
    [CTRL ('_')] = 31,
    [KBD_PGUP] = curses.KEY_PPAGE,
    [KBD_PGDN] = curses.KEY_NPAGE,
    [KBD_HOME] = curses.KEY_HOME,
    [KBD_END] = curses.KEY_END,
    [KBD_DEL] = curses.KEY_DC,
    [KBD_BS] = curses.KEY_BACKSPACE,
    [KBD_INS] = curses.KEY_IC, -- INSERT
    [KBD_LEFT] = curses.KEY_LEFT,
    [KBD_RIGHT] = curses.KEY_RIGHT,
    [KBD_UP] = curses.KEY_UP,
    [KBD_DOWN] = curses.KEY_DOWN,
    [KBD_F1] = curses.KEY_F1,
    [KBD_F2] = curses.KEY_F2,
    [KBD_F3] = curses.KEY_F3,
    [KBD_F4] = curses.KEY_F4,
    [KBD_F5] = curses.KEY_F5,
    [KBD_F6] = curses.KEY_F6,
    [KBD_F7] = curses.KEY_F7,
    [KBD_F8] = curses.KEY_F8,
    [KBD_F9] = curses.KEY_F9,
    [KBD_F10] = curses.KEY_F10,
    [KBD_F11] = curses.KEY_F11,
    [KBD_F12] = curses.KEY_F12,
  }

  codetokey_map = table.invert (keytocode_map)

  -- When there are duplicates, merge uses the one from argument two,
  -- hence when curses returns KEY_BACKSPACE we treat it as \C-h below.
  codetokey_map = table.merge (codetokey_map,
                               {
                                 [27] = KBD_META, -- Escape key
                                 [127] = KBD_BS, -- Delete key
                                 [curses.KEY_BACKSPACE] = CTRL ('h'),
                                 [curses.KEY_SUSPEND] = CTRL ('z'),
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
  elseif c > 0xff or c < 0 then
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

local function get_char ()
  local c

  if #key_buf > 0 then
    c = key_buf[#key_buf]
    table.remove (key_buf)
  else
    c = curses.stdscr ():getch ()
  end

  return c
end

function term_xgetkey (mode, timeout)
  while true do
    if bit.band (mode, GETKEY_DELAYED) ~= 0 then
      curses.stdscr ():timeout (timeout * 100)
    end
    if bit.band (mode, GETKEY_UNFILTERED) ~= 0 then
      curses.stdscr ():keypad (false)
    end

    local c = get_char (mode)
    if bit.band (mode, GETKEY_UNFILTERED) ~= 0 then
      curses.stdscr ():keypad (true)
    end
    if bit.band (mode, GETKEY_DELAYED) ~= 0 then
      curses.stdscr ():timeout (-1)
    end

    if c == curses.KEY_RESIZE then
      term_set_size (curses.cols (), curses.lines ())
      resize_windows ()
    elseif c ~= nil then
      local key
      if bit.band (mode, GETKEY_UNFILTERED) ~= 0 then
        key = c
      else
        key = codetokey (c)
        while key == KBD_META do
          key = bit.bor (codetokey (get_char ()), KBD_META)
        end
      end
      return key
    else
      return KBD_NOKEY
    end
  end
end

function term_ungetkey (key)
  key_buf = list.concat (key_buf, list.reverse (keytocodes (key)))
end

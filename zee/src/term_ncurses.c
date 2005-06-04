/* Exported terminal
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   All rights reserved.

   This file is part of Zee.

   Zee is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zee is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zee; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include "config.h"

#include <stddef.h>
#include <stdarg.h>
#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

typedef SCREEN Screen;

Screen *screen;

#include "main.h"
#include "extern.h"

void term_move(size_t y, size_t x)
{
  move((int)y, (int)x);
}

void term_clrtoeol(void)
{
  clrtoeol();
}

void term_refresh(void)
{
  refresh();
}

void term_clear(void)
{
  clear();
}

void term_addch(int c)
{
  addch((chtype)c);
}

void term_attrset(size_t attrs, ...)
{
  size_t i;
  unsigned long a = 0;
  va_list ap;
  va_start(ap, attrs);
  for (i = 0; i < attrs; i++) {
    Font f = va_arg(ap, Font);
    switch (f) {
    case FONT_NORMAL:
      a = 0;
      break;
    case FONT_REVERSE:
      a |= A_REVERSE;
      break;
    }
  }
  va_end(ap);
  attrset(a);
}

void term_beep(void)
{
  beep();
}

void term_init(void)
{
  screen = newterm(NULL, stdout, stdin);
  set_term(screen);

  term_set_size((size_t)COLS, (size_t)LINES);

  noecho();
  nonl();
  raw();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
}

void term_close(void)
{
  /* Clear last line. */
  term_move((size_t)(LINES - 1), 0);
  term_clrtoeol();
  term_refresh();

  /* Free memory and finish with ncurses. */
  endwin();
  delscreen(screen);
  screen = NULL;
}

static size_t translate_key(int c)
{
  switch (c) {
  case '\0':		/* C-@ */
    return KBD_CTL | '@';
  case '\1':  case '\2':  case '\3':  case '\4':  case '\5':
  case '\6':  case '\7':  case '\10':             case '\12':
  case '\13': case '\14':             case '\16': case '\17':
  case '\20': case '\21': case '\22': case '\23': case '\24':
  case '\25': case '\26': case '\27': case '\30': case '\31':
  case '\32':		/* C-a ... C-z */
    return KBD_CTL | ('a' + c - 1);
  case '\11':
    return KBD_TAB;
  case '\15':
    return KBD_RET;
  case '\37':
    return KBD_CTL | (c ^ 0x40);
#ifdef __linux__
  case 0627:		/* C-z */
    return KBD_CTL | 'z';
#endif
  case '\33':		/* META */
    return KBD_META;
  case KEY_PPAGE:		/* PGUP */
    return KBD_PGUP;
  case KEY_NPAGE:		/* PGDN */
    return KBD_PGDN;
  case KEY_HOME:		/* HOME */
    return KBD_HOME;
  case KEY_END:		/* END */
    return KBD_END;
  case KEY_DC:		/* DEL */
    return KBD_DEL;
  case KEY_BACKSPACE:	/* BS */
  case 0177:		/* BS */
    return KBD_BS;
  case KEY_IC:		/* INSERT */
    return KBD_INS;
  case KEY_LEFT:		/* LEFT */
    return KBD_LEFT;
  case KEY_RIGHT:		/* RIGHT */
    return KBD_RIGHT;
  case KEY_UP:		/* UP */
    return KBD_UP;
  case KEY_DOWN:		/* DOWN */
    return KBD_DOWN;
  case KEY_F(1):
    return KBD_F1;
  case KEY_F(2):
    return KBD_F2;
  case KEY_F(3):
    return KBD_F3;
  case KEY_F(4):
    return KBD_F4;
  case KEY_F(5):
    return KBD_F5;
  case KEY_F(6):
    return KBD_F6;
  case KEY_F(7):
    return KBD_F7;
  case KEY_F(8):
    return KBD_F8;
  case KEY_F(9):
    return KBD_F9;
  case KEY_F(10):
    return KBD_F10;
  case KEY_F(11):
    return KBD_F11;
  case KEY_F(12):
    return KBD_F12;
  default:
    if (c > 255)
      return KBD_NOKEY;	/* Undefined behaviour. */
    return c;
  }
}

#define MAX_UNGETKEY_BUF	16

static int ungetkey_buf[MAX_UNGETKEY_BUF];
static int *ungetkey_p = ungetkey_buf;

static size_t _getkey(void)
{
  int c;
  size_t key;

  if (ungetkey_p > ungetkey_buf)
    return *--ungetkey_p;

#ifdef KEY_RESIZE
  for (;;) {
    c = getch();
    if (c != KEY_RESIZE)
      break;
    resize_windows();
  }
#else
  c = getch();
#endif

  if (c == ERR)
    return KBD_NOKEY;

  key = translate_key(c);

  while (key == KBD_META) {
    c = getch();
    key = translate_key(c);
    key |= KBD_META;
  }

  return key;
}

static size_t _xgetkey(int mode, size_t arg)
{
  size_t c = KBD_NOKEY;
  switch (mode) {
  case GETKEY_UNFILTERED:
    c = (size_t)getch();
    break;
  case GETKEY_DELAYED:
    wtimeout(stdscr, (int)arg);
    c = _getkey();
    wtimeout(stdscr, -1);
    break;
  case GETKEY_UNFILTERED | GETKEY_DELAYED:
    wtimeout(stdscr, (int)arg);
    c = (size_t)getch();
    wtimeout(stdscr, -1);
    break;
  default:
    c = _getkey();
  }
  return c;
}

size_t term_xgetkey(int mode, size_t arg)
{
  int c;

  if (ungetkey_p > ungetkey_buf)
    return *--ungetkey_p;

#ifdef KEY_RESIZE
  for (;;) {
    c = _xgetkey(mode, arg);
    if (c != KEY_RESIZE)
      break;
    resize_windows();
  }
#else
  c = _xgetkey(mode, arg);
#endif

  return c;
}

void term_ungetkey(size_t key)
{
  if (ungetkey_p - ungetkey_buf >= MAX_UNGETKEY_BUF)
    return;

  *ungetkey_p++ = key;
}

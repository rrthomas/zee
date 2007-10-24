/* Allegro terminal
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
   Copyright (c) 2004 David A. Capello.
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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

#include "config.h"

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <allegro.h>

#include "main.h"
#include "extern.h"
#include "term.h"

// Font width and height
#define FW		8       // font_length (font, ...)
#define FH		8       // font_height (font)

// current position and color
static size_t cur_x = 0;
static size_t cur_y = 0;
static Font cur_color = 0;

static short *cur_scr = NULL;
static short *new_scr = NULL;

// Text and cursor foreground and background colours
#define FG_RED		255
#define FG_GREEN	255
#define FG_BLUE		255
#define BG_RED		0
#define BG_GREEN	0
#define BG_BLUE		0

static bool cursor_state = false;
static volatile bool blink_state = false;

static volatile int cur_time = 0;

static void draw_char(int c, size_t x, size_t y)
{
  int fg = makecol(FG_RED, FG_GREEN, FG_BLUE);
  int bg = makecol(BG_RED, BG_GREEN, BG_BLUE);

  if (c & FONT_REVERSE) {
    int aux = fg;
    fg = bg;
    bg = aux;
  }

  char s[2];
  c &= 0xff;
  c = c ? c : ' ';
  s[0] = c;
  s[1] = '\0';
  textout_ex(screen, font, s, (int)(x * FW), (int)(y * FH), fg, bg);
}
END_OF_STATIC_FUNCTION(draw_char)

static void draw_cursor(bool state)
{
  if (cursor_state && cur_x < win.fwidth && cur_y < win.fheight) {
    if (state)
      rectfill(screen, (int)(cur_x * FW), (int)(cur_y * FH),
               (int)(cur_x * FW + FW - 1), (int)(cur_y * FH + FH - 1),
               makecol(FG_RED, FG_GREEN, FG_BLUE));
    else
      draw_char(new_scr[cur_y * win.fwidth + cur_x], cur_x, cur_y);
  }
}
END_OF_STATIC_FUNCTION(draw_cursor)

static void control_blink_state(void)
{
  blink_state = !blink_state;
  draw_cursor(blink_state);
}
END_OF_STATIC_FUNCTION(control_blink_state)

static void reset_cursor(void)
{
  blink_state = 1;
  cursor_state = true;
  draw_cursor(blink_state);
  cursor_state = false;
}
END_OF_STATIC_FUNCTION(reset_cursor)

static void inc_cur_time(void)
{
  cur_time++;
}
END_OF_STATIC_FUNCTION(inc_cur_time)

static SAMPLE *the_beep = NULL;

void term_move(size_t y, size_t x)
{
  cur_x = x;
  cur_y = y;
}

void term_clrtoeol(void)
{
  if (cur_x < win.fwidth && cur_y < win.fheight) {
    size_t x;
    for (x = cur_x; x < win.fwidth; x++)
      new_scr[cur_y * win.fwidth + x] = 0;
  }
}

void term_refresh(void)
{
  size_t i = 0;
  for (size_t y = 0; y < win.fheight; y++)
    for (size_t x = 0; x < win.fwidth; x++, i++)
      if (new_scr[i] != cur_scr[i]) {
        cur_scr[i] = new_scr[i];
        draw_char(cur_scr[i], x, y);
      }
  reset_cursor();
}

void term_clear(void)
{
  memset(new_scr, 0, sizeof(short) * win.fwidth * win.fheight);
}

void term_addch(int c)
{
  if (cur_x < win.fwidth && cur_y < win.fheight) {
    int color = 0;

    if (c & 0x0f00)
      color |= c & 0x0f00;
    else if (cur_color & 0x0f00)
      color |= cur_color & 0x0f00;
    else
      color |= FONT_NORMAL;

    if (c & 0xf000)
      color |= c & 0xf000;
    else
      color |= cur_color & 0xf000;

    new_scr[cur_y * win.fwidth + cur_x] = (c & 0x00ff) | color;
  }
  cur_x++;
}

void term_nl(void)
{
  cur_x = 0;
  if (cur_y < win.fheight)
    cur_y++;
}

void term_attrset(size_t attrs, ...)
{
  size_t i, a = 0;
  va_list ap;

  va_start(ap, attrs);
  for (i = 0; i < attrs; i++)
    a |= va_arg(ap, Font);
  va_end(ap);
  cur_color = a;
}

void term_beep(void)
{
   play_sample(the_beep, 255, 128, 1000, FALSE);
}

void term_init(void)
{
  if (allegro_init() != 0) {
    allegro_message("Could not initialise Allegro.\n");
    die(1);
  }
  install_timer();
  install_keyboard();

  // If we can initialise sound, loud the beep sample.
  if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) == 0) {
    char *sample_file = PKGDATADIR "/beep.wav";
    if ((the_beep = load_sample(sample_file)) == NULL) {
      allegro_message("Could not read beep sample `%s'\n", sample_file);
      die(1);
    }
  }

  // Set up screen. After this we can't call allegro_message.
  set_color_depth(8);
  if (set_gfx_mode(GFX_SAFE, 640, 480, 0, 0) < 0) {
    allegro_message("Could not set up screen.\n");
    die(1);
  }

  LOCK_VARIABLE(blink_state);
  LOCK_FUNCTION(control_blink_state);
  LOCK_FUNCTION(draw_cursor);
  install_int(control_blink_state, 300);

  LOCK_VARIABLE(cur_time);
  LOCK_FUNCTION(inc_cur_time);
  install_int(inc_cur_time, 1);

  term_set_size((size_t)SCREEN_W / FW, (size_t)SCREEN_H / FH);

  cur_scr = zmalloc(sizeof(short) * win.fwidth * win.fheight);
  new_scr = zmalloc(sizeof(short) * win.fwidth * win.fheight);
}

void term_close(void)
{
  if (the_beep)
    destroy_sample(the_beep);
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_exit();
}

static int translate_key(int c)
{
  int ascii = c & 0xff;
  int scancode = c >> 8;

  if (!ascii && key_shifts & KB_ALT_FLAG) {
    if ((ascii = scancode_to_ascii(scancode)))
      return KBD_META | ascii |
        ((key_shifts & KB_CTRL_FLAG) ? KBD_CTRL : 0);
    else
      return KBD_NOKEY;
  }

  switch (scancode) {
  case KEY_F1: return KBD_F1;
  case KEY_F2: return KBD_F2;
  case KEY_F3: return KBD_F3;
  case KEY_F4: return KBD_F4;
  case KEY_F5: return KBD_F5;
  case KEY_F6: return KBD_F6;
  case KEY_F7: return KBD_F7;
  case KEY_F8: return KBD_F8;
  case KEY_F9: return KBD_F9;
  case KEY_F10: return KBD_F10;
  case KEY_F11: return KBD_F11;
  case KEY_F12: return KBD_F12;
  case KEY_ESC: return KBD_META;
  case KEY_TAB: return KBD_TAB;
  case KEY_ENTER:
  case KEY_ENTER_PAD: return KBD_RET;
  case KEY_BACKSPACE: return KBD_BS;
  case KEY_INSERT: return KBD_INS;
  case KEY_DEL: return KBD_DEL;
  case KEY_HOME: return KBD_HOME;
  case KEY_END: return KBD_END;
  case KEY_PGUP: return KBD_PGUP;
  case KEY_PGDN: return KBD_PGDN;
  case KEY_LEFT: return KBD_LEFT;
  case KEY_RIGHT: return KBD_RIGHT;
  case KEY_UP: return KBD_UP;
  case KEY_DOWN: return KBD_DOWN;
  case KEY_SPACE:
    if (key_shifts & KB_CTRL_FLAG)
      return '@' | KBD_CTRL;
    else
      return ' ';
  default:
    if ((scancode >= KEY_0 && scancode <= KEY_9) &&
        (ascii < 32 || ascii == 127))
      return ('0' + scancode - KEY_0) | KBD_CTRL;
    else if (ascii >= 1 && ascii <= 'z' - 'a' + 1)
      return ('a' + ascii - 1) | KBD_CTRL;
    else if (ascii >= ' ')
      return ascii;
  }

  return KBD_NOKEY;
}
END_OF_STATIC_FUNCTION(translate_key)

static int hooked_readkey(int mode, size_t timeout)
{
  size_t beg_time = cur_time;
  term_refresh();

  cursor_state = true;
  if (timeout > 0)
    while (!keypressed() && (!(mode & GETKEY_DELAYED) ||
                             cur_time - beg_time < timeout))
      rest(0);

  int ret = readkey();
  draw_cursor(false);
  cursor_state = false;

  return ret;
}
END_OF_STATIC_FUNCTION(hooked_readkey)

size_t term_xgetkey(int mode, size_t timeout)
{
  if (mode & GETKEY_UNFILTERED)
    return hooked_readkey(mode, timeout) & 0xff;
  else {
    size_t key = translate_key(hooked_readkey(mode, timeout));
    while (key == KBD_META) {
      key = translate_key(hooked_readkey(mode, 0));
      key |= KBD_META;
    }
    return key;
  }
}

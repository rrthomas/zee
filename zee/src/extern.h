/* Global functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
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


#include <stddef.h>
#include <unistd.h>
#include <lua.h>


// basic.c ----------------------------------------------------------------
size_t get_goalc(void);
bool goto_line(size_t to_line);
bool goto_point(Point pt);

// bind.c -----------------------------------------------------------------
void process_key(size_t key);
void init_bindings(void);

// buffer.c ---------------------------------------------------------------
void buffer_new(void);
bool warn_if_readonly_buffer(void);
bool warn_if_no_mark(void);
bool calculate_the_region(Region *rp);
size_t tab_width(void);
size_t indent_width(void);
rblist copy_text_block(Point start, size_t size);

// file.c -----------------------------------------------------------------
rblist get_home_dir(void);
rblist file_read(rblist filename);
void file_open(rblist filename);
void die(int exitcode);

// keys.c -----------------------------------------------------------------
size_t lastkey(void);
size_t xgetkey(int mode, size_t timeout);
size_t getkey(void);
void waitkey(size_t timeout);
void ungetkey(size_t key);

// killring.c -------------------------------------------------------------
void init_kill_ring(void);

// line.c -----------------------------------------------------------------
void remove_marker(Marker *marker);
Marker *marker_new(Point pt);
Marker *point_marker(void);
Marker *get_mark(void);
void set_mark(Marker *m);
void set_mark_to_point(void);
bool is_empty_line(void);
bool is_blank_line(void);
int following_char(void);
int preceding_char(void);
bool bobp(void);
bool eobp(void);
bool bolp(void);
bool eolp(void);
bool line_replace_text(size_t line, size_t offset, size_t oldlen, rblist newtext, bool replace_case);
bool insert_char(int c);
bool wrap_break_line(void);
bool replace_nstring(size_t size, rblist *as, rblist bs);

// lua.c ------------------------------------------------------------------
lua_obj get_variable(const char *key);
const char *get_variable_string(const char *var);
int get_variable_number(const char *var);
bool get_variable_bool(const char *var);
void init_commands(void);
bool cmd_eval(rblist rbl, rblist source);
void require(char *s);

// macro.c ----------------------------------------------------------------
void cancel_macro_definition(void);
void add_cmd_to_macro(void);
void add_key_to_cmd(size_t key);
void call_macro(const char *name);

// main.c -----------------------------------------------------------------
extern Window win;
extern Buffer *buf;
extern int thisflag, lastflag;

// minibuf.c --------------------------------------------------------------
void minibuf_write(rblist rbl);
void minibuf_error(rblist rbl);
rblist minibuf_read(rblist rbl, rblist value);
void minibuf_clear(void);
rblist minibuf_read_name(rblist rbl);

// point.c ----------------------------------------------------------------
Point make_point(size_t lineno, size_t offset);
ssize_t point_dist(Point pt1, Point pt2);
size_t count_lines(Point pt1, Point pt2);
void swap_point(Point *pt1, Point *pt2);
Point point_min(Buffer *bp);
Point point_max(Buffer *bp);

// term.c -----------------------------------------------------------------
void resync_display(void);
void term_resize(void);
size_t column_to_character(rblist rbl, size_t goal);
size_t string_display_width(rblist rbl);
void popup_set(rblist rbl);
void popup_clear(void);
void popup_scroll_down_and_loop(void);
void popup_scroll_down(void);
void popup_scroll_up(void);
void term_set_size(size_t cols, size_t rows);
void term_display(void);
void term_tidy(void);
void term_print(rblist rbl);
void term_beep(void);

// undo.c -----------------------------------------------------------------
void undo_save(int type, Point pt, size_t arg1, size_t arg2);
void undo_reset_unmodified(Undo *up);

// C functions for interactive commands -----------------------------------
#define X(cmd_name) \
  int F_ ## cmd_name(lua_State *L);
#include "tbl_funcs.h"
#undef X

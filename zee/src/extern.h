/* Global functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2006 Reuben Thomas.
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

/* basic.c ---------------------------------------------------------------- */
size_t get_goalc(void);
int goto_line(size_t to_line);
int goto_column(size_t to_col);
int goto_point(Point pt);

/* bind.c ----------------------------------------------------------------- */
void bind_key(size_t key, Function func);
astr minibuf_read_function_name(astr as);
astr binding_to_function(size_t key);
void process_key(size_t key);
void init_bindings(void);
Function get_function(astr name);

/* buffer.c --------------------------------------------------------------- */
void buffer_new(void);
int warn_if_readonly_buffer(void);
int warn_if_no_mark(void);
int calculate_the_region(Region *rp);
void anchor_mark(void);
void weigh_mark(void);
size_t tab_width(void);
astr copy_text_block(Point start, size_t size);

/* completion.c ----------------------------------------------------------- */
Completion *completion_new(void);
int completion_try(Completion *cp, astr search, int popup_when_complete);

/* display.c -------------------------------------------------------------- */
void resync_display(void);
void resize_window(void);
void recenter(void);
Line *popup_get(void);
size_t popup_lines(void);
void popup_set(astr as);
void popup_clear(void);
size_t popup_pos(void);
void popup_scroll_up(void);
void popup_scroll_down(void);
size_t term_width(void);
size_t term_height(void);
void term_set_size(size_t cols, size_t rows);
void term_display(void);
void term_tidy(void);
void term_print(astr as);
void ding(void);

/* file.c ----------------------------------------------------------------- */
astr get_home_dir(void);
astr file_read(astr *as, astr filename);
void file_open(astr filename);
void die(int exitcode);

/* history.c -------------------------------------------------------------- */
void add_history_element(History *hp, astr string);
void prepare_history(History *hp);
astr previous_history_element(History *hp);
astr next_history_element(History *hp);

/* keys.c ----------------------------------------------------------------- */
size_t xgetkey(int mode, size_t timeout);
size_t getkey(void);
void waitkey(size_t timeout);
void ungetkey(size_t key);
astr chordtostr(size_t key);
size_t strtochord(astr chord);

/* killring.c ------------------------------------------------------------- */
void init_kill_ring(void);

/* line.c ----------------------------------------------------------------- */
void remove_marker(Marker *marker);
Marker *marker_new(Point pt);
Marker *point_marker(void);
Marker *get_mark(void);
void set_mark(Marker *m);
void set_mark_to_point(void);
Line *line_new(void);
Line *string_to_lines(astr as, astr eol, size_t *lines);
int is_empty_line(void);
int is_blank_line(void);
int following_char(void);
int preceding_char(void);
int bobp(void);
int eobp(void);
int bolp(void);
int eolp(void);
int line_replace_text(Line **lp, size_t offset, size_t oldlen, astr newtext, int replace_case);
int insert_char(int c);
void fill_break_line(void);
int insert_nstring(astr as, astr eolstr, int intercalate);
int delete_nstring(size_t size, astr *as);

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro(void);
void add_cmd_to_macro(void);
void add_key_to_cmd(size_t key);
void call_macro(Macro *mp);
Macro *get_macro(astr name);

/* main.c ----------------------------------------------------------------- */
extern Window win;
extern Buffer buf;
extern int thisflag, lastflag, uniarg;

/* minibuf.c -------------------------------------------------------------- */
void minibuf_write(astr as);
void minibuf_error(astr as);
astr minibuf_read(astr as, astr value);
int minibuf_read_yesno(astr as);
int minibuf_read_boolean(astr as);
astr minibuf_read_completion(astr prompt, astr value, Completion *cp, History *hp);
void minibuf_clear(void);

/* parser.c --------------------------------------------------------------- */
void cmd_parse_init(astr as);
void cmd_parse_end(void);
void cmd_eval(void);
void cmd_eval_file(astr file);

/* point.c ---------------------------------------------------------------- */
Point make_point(size_t lineno, size_t offset);
int cmp_point(Point pt1, Point pt2);
int point_dist(Point pt1, Point pt2);
int count_lines(Point pt1, Point pt2);
void swap_point(Point *pt1, Point *pt2);
Point point_min(Buffer *bp);
Point point_max(Buffer *bp);

/* term_{allegro,ncurses}.c ----------------------------------------------- */
void term_init(void);
void term_close(void);
void term_move(size_t y, size_t x);
void term_clrtoeol(void);
void term_refresh(void);
void term_clear(void);
void term_addch(int c);
void term_nl(void);
void term_attrset(size_t attrs, ...);
void term_beep(void);
size_t term_xgetkey(int mode, size_t timeout);

/* undo.c ----------------------------------------------------------------- */
void undo_save(int type, Point pt, size_t arg1, size_t arg2);
void undo_reset_unmodified(Undo *up);

/* variables.c ------------------------------------------------------------ */
void init_variables(void);
void set_variable(astr var, astr val);
astr get_variable(astr var);
int get_variable_number(astr var);
int get_variable_bool(astr var);
astr minibuf_read_variable_name(astr msg);

/* External C functions for interactive commands -------------------------- */
#define X(cmd_name) \
  int F_ ## cmd_name(size_t argc, int intarg, list l);
#include "tbl_funcs.h"
#undef X

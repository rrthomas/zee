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
astr minibuf_read_function_name(const char *fmt, ...);
const char *binding_to_function(size_t key);
void process_key(size_t key);
void init_bindings(void);
Function get_function(const char *name);
const char *get_function_name(Function p);

/* buffer.c --------------------------------------------------------------- */
void buffer_new(void);
void set_buffer_filename(Buffer *bp, const char *filename);
int warn_if_readonly_buffer(void);
int warn_if_no_mark(void);
int calculate_the_region(Region *rp);
void anchor_mark(void);
void weigh_mark(void);
size_t tab_width(void);

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

/* file.c ----------------------------------------------------------------- */
astr get_home_dir(void);
astr file_read(astr *as, const char *filename);
void file_open(const char *filename);
void die(int exitcode);

/* glue.c ----------------------------------------------------------------- */
void ding(void);
size_t xgetkey(int mode, size_t timeout);
size_t getkey(void);
void waitkey(size_t timeout);
void ungetkey(size_t key);
astr copy_text_block(Point start, size_t size);
astr shorten_string(char *s, int maxlen);
char *getln(FILE *fp);

/* history.c -------------------------------------------------------------- */
void add_history_element(History *hp, const char *string);
void prepare_history(History *hp);
const char *previous_history_element(History *hp);
const char *next_history_element(History *hp);

/* keys.c ----------------------------------------------------------------- */
astr chordtostr(size_t key);
size_t strtochord(const char *buf);

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
Line *string_to_lines(astr as, const char *eol, size_t *lines);
int is_empty_line(void);
int is_blank_line(void);
int following_char(void);
int preceding_char(void);
int bobp(void);
int eobp(void);
int bolp(void);
int eolp(void);
int line_replace_text(Line **lp, size_t offset, size_t oldlen, const char *newtext, size_t newlen, int replace_case);
int insert_char(int c);
void fill_break_line(void);
int insert_nstring(astr as, const char *eolstr, int intercalate);
int delete_nstring(size_t size, astr *as);

/* macro.c ---------------------------------------------------------------- */
void cancel_kbd_macro(void);
void add_cmd_to_macro(void);
void add_key_to_cmd(size_t key);
void call_macro(Macro *mp);
Macro *get_macro(const char *name);

/* main.c ----------------------------------------------------------------- */
extern Window win;
extern Buffer buf;
extern int thisflag, lastflag, uniarg;

/* minibuf.c -------------------------------------------------------------- */
char *minibuf_format(const char *fmt, va_list ap);
void minibuf_error(const char *fmt, ...);
void minibuf_write(const char *fmt, ...);
astr minibuf_read(const char *fmt, const char *value, ...);
int minibuf_read_yesno(const char *fmt, ...);
int minibuf_read_boolean(const char *fmt, ...);
astr minibuf_read_dir(const char *fmt, const char *value, ...);
astr minibuf_read_completion(const char *fmt, char *value, Completion *cp, History *hp, ...);
void minibuf_clear(void);

/* parser.c --------------------------------------------------------------- */
void cmd_parse_init(astr as);
void cmd_parse_end(void);
void cmd_eval(void);
void cmd_eval_file(const char *file);

/* point.c ---------------------------------------------------------------- */
Point make_point(size_t lineno, size_t offset);
int cmp_point(Point pt1, Point pt2);
int point_dist(Point pt1, Point pt2);
int count_lines(Point pt1, Point pt2);
void swap_point(Point *pt1, Point *pt2);
Point point_min(Buffer *bp);
Point point_max(Buffer *bp);

/* term_display.c --------------------------------------------------------- */
size_t term_width(void);
size_t term_height(void);
void term_set_size(size_t cols, size_t rows);
void term_display(void);
void term_tidy(void);
int term_printf(const char *fmt, ...);

/* term_minibuf.c --------------------------------------------------------- */
void term_minibuf_write(const char *fmt);
astr term_minibuf_read(const char *prompt, const char *value, Completion *cp, History *hp);

/* term_{allegro,epocemx,ncurses}.c --------------------------------------- */
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
void undo_save(int type, Point pt, size_t arg1, size_t arg2, int intercalate);

/* variables.c ------------------------------------------------------------ */
void init_variables(void);
void set_variable(const char *var, const char *val);
const char *get_variable(const char *var);
int get_variable_number(char *var);
int get_variable_bool(char *var);
astr minibuf_read_variable_name(char *msg);

/* External C functions for interactive commands -------------------------- */
#define X(cmd_name, c_name) \
  int F_ ## c_name(int argc, int uniarg, list *branch);
#include "tbl_funcs.h"
#undef X

/* Main types and definitions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2005 Reuben Thomas.
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

#ifndef MAIN_H
#define MAIN_H

#include <limits.h>

#include "zmalloc.h"
#include "vector.h"
#include "list.h"
#include "astr.h"
#include "parser.h"
#include "eval.h"

#undef TRUE
#define TRUE                            1
#undef FALSE
#define FALSE                           0

#undef min
#define min(a, b)                       ((a) < (b) ? (a) : (b))
#undef max
#define max(a, b)                       ((a) > (b) ? (a) : (b))

/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

/*
 * The type of a Zee exported function.  `uniarg' is the number of
 * times to repeat the function.
 */
typedef int (*Function)(int argc, le *branch);

/* Line.
 * A line is a list whose items are astrs. The newline at the end of
 * each line is implicit.
 */
typedef struct list_s Line;

/* Point and Marker */
typedef struct Point {
  Line *p;                      /* Line pointer */
  size_t n;                     /* Line number */
  size_t o;                     /* Offset */
} Point;

typedef struct Buffer Buffer; /* Forward reference */
typedef struct Marker Marker;
struct Marker {
  Buffer *bp;             /* Buffer that points into */
  Point pt;               /* Point position */
  Marker *next;           /* Used to chain all markers in the buffer */
};

/* Undo delta types */
enum {
  UNDO_REPLACE_BLOCK,           /* Replace a block of characters */
  UNDO_START_SEQUENCE,          /* Start a multi operation sequence */
  UNDO_END_SEQUENCE             /* End a multi operation sequence */
};

typedef struct Undo Undo;
struct Undo {
  /* Next undo delta in list */
  Undo *next;

  /* The type of undo delta */
  int type;

  /* Where the undo delta need to be applied.
     Warning!: Do not use the "pt.p" field */
  Point pt;

  /* Flag indicating that reverting this undo leaves the buffer
     in an unchanged state */
  int unchanged;

  /* The undo delta */
  struct {
    astr text;                /* String */
    size_t size;              /* Block size for replace */
    int intercalate;          /* TRUE means intercalate, FALSE insert */
  } delta;
};

typedef struct Region {
  Point start;            /* The region start */
  Point end;              /* The region end */
  size_t size;                /* The region size */

  /* The total number of lines ('\n' newlines) in region */
  int num_lines;
} Region;

/* Buffer flags or minor modes */

#define BFLAG_MODIFIED  (0x0001) /* The buffer has been modified */
#define BFLAG_NEEDNAME  (0x0002) /* On save, ask for a file name */
#define BFLAG_READONLY  (0x0004) /* The buffer cannot be modified */
#define BFLAG_AUTOFILL  (0x0008) /* The buffer is in Auto Fill mode */
#define BFLAG_ISEARCH   (0x0010) /* The buffer is in Isearch loop */

/* Represents a buffer: an open file.
 * To support multiple simultaneous buffers, they can be organised into a linked
 * list using the 'next' field.
 * Every buffer has its own:
 *  - Point and mark (i.e. cursor and selection)
 *  - List of markers
 *  - Undo history
 *  - Flags, including the line terminator
 *  - Filename
 */
struct Buffer {
  /* The next buffer in buffer list */
  Buffer *next;

  /* The lines of text */
  Line *lines;

  /* The point */
  Point pt;

  /* The mark */
  Marker *mark;

  /* Markers (points that are updated when text is modified) */
  Marker *markers;

  /* The undo deltas recorded for this buffer */
  Undo *next_undop;
  Undo *last_undop;

  /* Buffer flags */
  int flags;
  size_t mark_anchored : 1;

  /* Buffer-local variables */
  le *vars;

  /* The total number of lines in the buffer */
  size_t num_lines;

  /* The name of the buffer and the file name */
  char *name;
  char *filename;

  /* EOL string (up to 2 chars) for this buffer */
  char eol[3];
};

/*
 * Represents a window on the screen: a rectangular area used to
 * display a buffer. To allow more than one window at a time, windows
 * can be organised into a linked list using the 'next' field.
 */
typedef struct Window Window;
struct Window {
  /* The buffer displayed in window */
  Buffer *bp;

  /* The top line delta and last point line number.
   * (Question: definitions?)
   */
  size_t topdelta;
  int lastpointn;

  /* The formal and effective width and height of window.
   * (Question: definitions?)
   */
  size_t fwidth, fheight;
  size_t ewidth, eheight;
};

enum {
  COMPLETION_NOTCOMPLETING,
  COMPLETION_NOTMATCHED,
  COMPLETION_MATCHED,
  COMPLETION_MATCHEDNONUNIQUE,
  COMPLETION_NONUNIQUE
};

typedef struct Completion {
  /* This flag is set when the vector is sorted */
  int fl_sorted;
  /* This flag is set when a completion window has been popped up */
  int fl_poppedup;

  /* The old buffer */
  Buffer *old_bp;

  /* This flag is set when this is a filename completion */
  int fl_dir;
  astr path;

  /* This flag is set when the space character is allowed */
  int fl_space;

  list completions;             /* The completions list */

  list matches;                 /* The matches list */
  char *match;                  /* The match buffer */
  size_t matchsize;             /* The match buffer size */
} Completion;

typedef struct History {
  list elements;                /* Elements (strings) */
  list sel;
} History;

typedef struct Binding {
  size_t key;
  Function func;
} Binding;

typedef struct Macro {
  vector *keys;                  /* Vector of keystrokes */
  char *name;                   /* Name of the macro */
  struct Macro *next;           /* Next macro in the list */
} Macro;

/* Type of font attributes */
typedef size_t Font;

/* Zee font codes
 * Designed to fit in an int, leaving room for a char underneath */
#define FONT_NORMAL		0x000
#define FONT_REVERSE		0x100

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED                  0x1
#define GETKEY_UNFILTERED               0x2

/* Named keys */
enum {
#define X(key_sym, key_name, key_code) \
	key_sym = key_code,
#include "tbl_keys.h"
#undef X
  KBD_CANCEL = (KBD_CTRL | 'g'), /* Special key that shouldn't be in text tables */
  KBD_NOKEY = INT_MAX
};

/*--------------------------------------------------------------------------
 * Global flags.
 *--------------------------------------------------------------------------*/

/* The last command was a C-p or a C-n */
#define FLAG_DONE_CPCN                  0x0001
/* The last command was a kill */
#define FLAG_DONE_KILL                  0x0002
/* Hint for the redisplay engine: a resync is required */
#define FLAG_NEED_RESYNC                0x0004
/* Quit the editor as soon as possible */
#define FLAG_QUIT                       0x0008
/* The last command modified the universal argument variable `uniarg' */
#define FLAG_SET_UNIARG                 0x0010
/* We are defining a macro */
#define FLAG_DEFINING_MACRO             0x0020
/* Encountered an error */
#define FLAG_GOT_ERROR                  0x0040

/*--------------------------------------------------------------------------
 * Miscellaneous stuff.
 *--------------------------------------------------------------------------*/

/* Ensure PATH_MAX is defined */
#ifndef PATH_MAX
#ifdef _POSIX_PATH_MAX
#define PATH_MAX	_POSIX_PATH_MAX
#else
/* Guess if all else fails */
#define PATH_MAX	254
#endif
#endif

/* Define an interactive function */
/* N.B. The function type is actually eval_cb */
#define DEFUN(lisp_name, c_func) \
  int F_ ## c_func(int argc, le *branch) \
  { \
    int uniused = argc > 1, ok = TRUE;

#define DEFUN_INT(lisp_name, c_func) \
  DEFUN(lisp_name, c_func) \
  int uniarg = 1; \
  if (uniused) { \
    le *value_le = evaluateNode(branch); \
    uniarg = evalCastLeToInt(value_le); \
    leWipe(value_le); \
  }

#define END_DEFUN \
    return ok; \
  }

/* Call an interactive function */
#define FUNCALL(c_func) \
  F_ ## c_func(1, NULL)

/* Call an interactive function with an universal argument */
le *funcall_arg_uniarg;
int funcall_arg_ok;
#define FUNCALL_ARG(c_func, uniarg) \
  (funcall_arg_uniarg = evalCastIntToLe(uniarg), \
   funcall_arg_ok = F_ ## c_func(2, funcall_arg_uniarg), \
   leWipe(funcall_arg_uniarg), \
   funcall_arg_ok)

/* Default waitkey pause in ds */
#define WAITKEY_DEFAULT 20

#endif /* !MAIN_H */

/* Main types and definitions
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

#ifndef MAIN_H
#define MAIN_H

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <lauxlib.h>

#include "nonstd.h"
#include "zmalloc.h"
#include "list.h"
#include "rblist.h"
#include "rbutil.h"


/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

lua_State *L;                   // The Lua state

// Point and Marker
typedef struct {
  size_t n;                     // Line number
  size_t o;                     // Offset
} Point;

/* A Marker points to a position in a buffer, and is updated when
   the buffer is modified */
typedef struct Marker {
  Point pt;                     // Point position
  struct Marker *next;      // Used to chain all markers in the buffer
} Marker;

// Undo delta types
enum {
  UNDO_REPLACE_BLOCK,           // Replace a block of characters
  UNDO_START_SEQUENCE,          // Start a multi operation sequence
  UNDO_END_SEQUENCE             // End a multi operation sequence
};

typedef struct Undo {
  struct Undo *next;            // Next undo delta in list
  int type;                     // The type of undo delta
  Point pt;                     // Where the undo delta is to be applied.
  bool unchanged; // Flag indicating that reverting this undo leaves the buffer
                  // in an unchanged state
  rblist text;                  // Replacement string
  size_t size;                  // Block size for replace
} Undo;

typedef struct {
  Point start;                  // The region start
  Point end;                    // The region end
  size_t size;                  // The region size
} Region;

// Buffer flags or minor modes

#define BFLAG_MODIFIED  (0x0001) // The buffer has been modified
#define BFLAG_READONLY  (0x0002) // The buffer cannot be modified
#define BFLAG_AUTOFILL  (0x0004) // The buffer is in Wrap mode
#define BFLAG_ISEARCH   (0x0008) // The buffer is in Isearch loop
#define BFLAG_ANCHORED  (0x0010) // The mark is anchored

/*
 * Represents a buffer.
 */
typedef struct {
  rblist lines;                 // The lines of text
  Point pt;                     // The point
  Marker *mark;                 // The mark
  Marker *markers;              // Markers
  Undo *last_undop;             // What was done most recently.
  Undo *next_undop;             // What edit_undo will undo next
                                // (points into the same list).
  int flags;                    // Buffer flags
  rblist filename;              // The name of the file being edited
} Buffer;

/*
 * Represents a window on the screen: a rectangular area used to
 * display a buffer.
 */
typedef struct {
  size_t topdelta; // The buffer line displayed in the topmost window line
  size_t lastpointn;            // Point line at last resync
  size_t term_width, term_height; /* The actual width and height of
                                     the terminal (used for resizing;
                                     normally the same as fwidth and
                                     fheight) */
  size_t fwidth, fheight; /* The formal width and height of the window
                             (space used on the terminal). */
  size_t ewidth, eheight; /* The effective width and height of the
                             window (space available for buffer display). */
} Window;

typedef struct {
  list completions;             // The completion strings
  list matches;                 // The matches list
  rblist match;                 // The current matched string
} Completion;

typedef struct {
  list elements;                // Elements (strings)
  list sel;                     // Currently selected element (pointer to elements)
} History;

// Type of font attributes
typedef size_t Font;

/* Font codes
 * Designed to fit in an int, leaving room for a char underneath */
#define FONT_NORMAL		0x000
#define FONT_REVERSE		0x100

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED                  0x1
#define GETKEY_UNFILTERED               0x2

// Default waitkey pause in ds
#define WAITKEY_DEFAULT 20

// Named keys
enum {
#define X(key_sym, key_name, key_code) \
	key_sym = key_code,
#include "tbl_keys.h"
#undef X
  KBD_NOKEY = INT_MAX
};

/*--------------------------------------------------------------------------
 * Global flags.
 *--------------------------------------------------------------------------*/

// The last command was a C-p or a C-n
#define FLAG_DONE_CPCN                  0x0001
// The last command was a kill
#define FLAG_DONE_KILL                  0x0002
// Quit the editor as soon as possible
#define FLAG_QUIT                       0x0004
// We are defining a macro
#define FLAG_DEFINING_MACRO             0x0008

/*--------------------------------------------------------------------------
 * Commands.
 *--------------------------------------------------------------------------*/

// The type of a user command.
typedef bool (*Command)(list l);

/* Define an interactive command.
 *
 * The C prototype for commands is Command (see above).
 *
 * To call such a command with an argument list a1, a2, ..., an, pass
 * the arguments (which should all be rblists) in a list.
 *
 * The macro `CMDCALL' can be used to call zero-argument commands.
 * The macro `CMDCALL_UINT' can be used to call commands taking a
 * single unsigned integer argument.
 */

// Declare a command.
#define DEF(name, doc) \
  bool F_ ## name(list l) \
  { \
    bool ok = true; \
    assert(l != NULL);

/* Declare a command that takes arguments.
   `args' is a comma-separated list. */
#define DEF_ARG(name, doc, args) \
  DEF(name, doc) \
  args

// End a command definition.
#define END_DEF \
    return ok; \
  }

// Declare a string argument, with prompt.
#define STR(name, prompt) \
    rblist name = NULL; \
    if (!list_empty(l)) \
      name = list_behead(l); \
    else if ((name = minibuf_read(rblist_from_string(prompt), rblist_empty)) == NULL) \
      ok = CMDCALL(edit_select_off);

// Declare a command name argument, with prompt.
#define COMMAND(name, prompt) \
    rblist name = NULL; \
    if (!list_empty(l)) \
      name = list_behead(l); \
    else if ((name = minibuf_read_command_name(rblist_from_string(prompt))) == NULL) \
      ok = CMDCALL(edit_select_off);

// Declare an unsigned integer argument, with prompt.
#define UINT(name, prompt) \
    size_t name = 0; \
    if (!list_empty(l)) { \
      rblist rbl = list_behead(l); \
      if ((name = strtoul(rblist_to_string(rbl), NULL, 10)) == ULONG_MAX) \
        ok = false; \
    } else do { \
      rblist ms; \
      if ((ms = minibuf_read(rblist_from_string(prompt), rblist_from_string(""))) == NULL) { \
        ok = CMDCALL(edit_select_off); \
        break; \
      } \
      if ((name = strtoul(rblist_to_string(ms), NULL, 10)) == ULONG_MAX) \
        term_beep(); \
    } while (name == ULONG_MAX);

/* FIXME: These macros don't work properly within command definitions,
   as they create a new argument list. This breaks e.g. a non-interactive
   use of execute_command. */
// Call a command
#define CMDCALL(name) \
  F_ ## name(list_new())

// Call a command with an integer argument
#define CMDCALL_UINT(name, arg) \
  F_ ## name(list_append(list_new(), rblist_fmt("%d", arg)))

#endif // !MAIN_H

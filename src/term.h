/* Terminal API
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

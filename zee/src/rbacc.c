/* Character buffers optimised for repeated append.
   Copyright (c) 2007 Alistair Turnbull.
   Copyright (c) 2007 Reuben Thomas.
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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "zmalloc.h"
#include "rblist.h"
#include "rbacc.h"


/*
 * Tuning parameter which controls memory/CPU compromise.
 * This defines the number of characters that can be added to a buffer
 * before it will call rblist_concat.
 */
#define ACCUMULATOR_LENGTH 32

// This is the opaque public type.
struct rbacc {
  rblist head;
  size_t used;
  char tail[ACCUMULATOR_LENGTH];
};


/*****************************/
// Static utility functions.

// Flushes the characters buffered in rbacc.tail into rbacc.head.
static void flush(rbacc rba)
{
  assert(rba->used <= ACCUMULATOR_LENGTH);
  if (rba->used > 0)
    rba->head = rblist_concat(rba->head, rblist_from_array(rba->tail, rba->used));
  rba->used = 0;
}


/**************************/
// Constructor.

rbacc rbacc_new(void)
{
  rbacc rba = zmalloc(sizeof(struct rbacc));
  rba->head = rblist_empty;
  rba->used = 0;
  return rba;
}


/*************/
// Methods.

rbacc rbacc_char(rbacc rba, int c)
{
  if (rba->used >= ACCUMULATOR_LENGTH)
    flush(rba);
  rba->tail[rba->used++] = (char)c;

  return rba;
}

rbacc rbacc_rblist(rbacc rba, rblist rbl)
{
  if (rba->used + rblist_length(rbl) > ACCUMULATOR_LENGTH)
    flush(rba);
  if (rblist_length(rbl) < ACCUMULATOR_LENGTH) {
    RBLIST_FOR(c, rbl)
      rba->tail[rba->used++] = c;
    RBLIST_END
  } else {
    assert(rba->used == 0);
    rba->head = rblist_concat(rba->head, rbl);
  }

  return rba;
}

rbacc rbacc_array(rbacc rba, const char *cs, size_t length)
{
  if (rba->used + length > ACCUMULATOR_LENGTH)
    flush(rba);
  if (length < ACCUMULATOR_LENGTH) {
    memcpy(&rba->tail[rba->used], cs, length * sizeof(char));
    rba->used += length;
  } else {
    assert(rba->used == 0);
    rba->head = rblist_concat(rba->head, rblist_from_array(cs, length));
  }

  return rba;
}

rbacc rbacc_string(rbacc rba, const char *s)
{
  return rbacc_array(rba, s, strlen(s));
}

rbacc rbacc_file(rbacc rba, FILE *fp)
{
  int c;

  // FIXME: Read BUFSIZ bytes at a time
  while ((c = getc(fp)) != EOF)
    rbacc_char(rba, c);

  return rba;
}

rbacc rbacc_file_line(rbacc rba, FILE *fp)
{
  int c;

  while ((c = getc(fp)) != EOF && c != '\n')
    rbacc_char(rba, c);

  return rba;
}

size_t rbacc_length(rbacc rba)
{
  return rblist_length(rba->head) + rba->used;
}

/* 
 * This function serves as a definition of the contents of an rbacc.
 */
rblist rbacc_to_rblist(rbacc rba)
{
  flush(rba);
  return rba->head;
}


/*************/
// Test code

#ifdef TEST

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// Stub to make zrealloc happy
void die(int exitcode)
{
  exit(exitcode);
}

/*
 * Checks the invariants of `rba'. If `s' is non-NULL, checks that it
 * matches the contents of `rba'. `rba' is not modified in any way.
 */
static void test(rbacc rba, const char *s, size_t length)
{
  assert(rbacc_length(rba) == length);
  if (s) {
    size_t i = 0;
    RBLIST_FOR(c, rba->head)
      assert(c == s[i++]);
    RBLIST_END
    for (size_t j = 0; j < rba->used; j++)
      assert(rba->tail[j] == s[i++]);
    assert(i == length);
  }
}

int main(void)
{
  rbacc rba;
  const char *s1 = "Hello, world!";
  const char *s2 = "Hello, very very very very very very very very big world!";
  const rblist rbl1 = rblist_from_string(s1), rbl2 = rblist_from_string(s2);
  FILE *fp;
  
  // Basic functionality tests.
  rba = rbacc_new();
  test(rba, "", 0);
  rbacc_char(rba, 'x');
  test(rba, "x", 1);
  rbacc_char(rba, 'y');
  test(rba, "xy", 2);
  rbacc_array(rba, "foo", 3);
  test(rba, "xyfoo", 5);
  assert(rba->used==5);
  flush(rba);
  test(rba, "xyfoo", 5);
  assert(rba->used==0);
  assert(!rblist_compare(rbacc_to_rblist(rba), rblist_from_string("xyfoo")));
  
  // Test various ways of appending `s1'.
  rba = rbacc_new();
  rbacc_string(rba, s1);
  test(rba, s1, strlen(s1));
  
  rba = rbacc_new();
  rbacc_rblist(rba, rbl1);
  test(rba, s1, strlen(s1));

  rba = rbacc_new();
  for (int i=1; i<=10; i++) {
    rbacc_string(rba, s1);
    assert(rbacc_length(rba) == i * strlen(s1));
  }
  flush(rba);
  assert(rbacc_length(rba) == 10 * strlen(s1));

  rba = rbacc_new();
  for (int i=1; i<=10; i++) {
    rbacc_rblist(rba, rbl1);
    assert(rbacc_length(rba) == i * strlen(s1));
  }
  
  // Test various ways of appending `s2'.
  rba = rbacc_new();
  rbacc_string(rba, s2);
  test(rba, s2, strlen(s2));
  
  rba = rbacc_new();
  rbacc_rblist(rba, rbl2);
  test(rba, s2, strlen(s2));

  rba = rbacc_new();
  for (int i=1; i<=10; i++) {
    rbacc_string(rba, s2);
    assert(rbacc_length(rba) == i * strlen(s2));
  }
  flush(rba);
  assert(rbacc_length(rba) == 10 * strlen(s2));

  rba = rbacc_new();
  for (int i=1; i<=10; i++) {
    rbacc_rblist(rba, rbl2);
    assert(rbacc_length(rba) == i * strlen(s2));
  }

  // rbacc_file_line
  assert(fp = fopen(SRCPATH "rbacc.c", "r"));
  rba = rbacc_new();
  rbacc_file_line(rba, fp);
  assert(!rblist_compare(rbacc_to_rblist(rba), rblist_from_string("/* Character buffers optimised for repeated append.")));
  assert(fclose(fp) == 0);

  return EXIT_SUCCESS;
}

#endif // TEST
/* Randomly balanced lists.
   Copyright (c) 2006 Alistair Turnbull.
   Copyright (c) 2006 Reuben Thomas.
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "zmalloc.h"
#include "rblist.h"


/* For debugging: incremented every time random_choice is called. */
static size_t choice_counter = 0;

/*
 * Returns true with probability p/(p+q).
 * Assumes (p+q) is representable as a size_t.
 */
static inline bool random_choice(size_t p, size_t q)
{
  static size_t seed = 483568341; /* Arbitrary. */

  choice_counter++;
  seed = (2 * seed - 1) * seed + 1; /* Maximal period mod any power of two. */
  return (p + q) * (float)seed < p * ((float)SIZE_MAX + 1.0);
}

/*
 * In ML, this data structure would be defined as follows:
 *
 *   datatype rblist =
 *     | leaf of int * char array
 *     | node of int * rblist * rblist;
 *
 * "leaf" represents a primitive array, and "node" represents the
 * concatenation of two shorter lists. In both cases, the int field
 * is the length of the list.
 *
 * In C, we can distinguish "leaf" from "node" by comparing the length
 * with RBLIST_MINIMUM_NODE_LENGTH. We can therefore use an untagged
 * union. To allow us to access the length without first knowing whether
 * it is a leaf or a node, we include it as a third member of the union.
 *
 * The subtle thing about randomly balanced lists is that they are not
 * completely deterministic; the arrangement of leafs and nodes is chosen
 * at random from the set of all possible structures that represent the
 * correct list.
 *
 * The only operation which truly chooses the whole structure randomly is
 * rblist_from_array. In other operations the structure of the result is
 * correlated with the structure of the operands. The result has the
 * correct probability distribution only on the assumption that the
 * operands have the correct probability distribution and are independent.
 */

struct leaf {
  size_t length;
  char data[];
};

struct node {
  size_t length;
  rblist left;
  rblist right;
};

union rblist {
  size_t length;
  struct leaf leaf;
  struct node node;
};

static struct leaf empty = {0};

rblist rblist_empty = (rblist)&empty;

rblist rblist_singleton(char c)
{
  struct leaf *ret = zmalloc(sizeof(struct leaf) + sizeof(char));
  ret->length = 1;
  ret->data[0] = c;
  return (rblist)ret;
}

rblist rblist_concat(rblist left, rblist right)
{
  size_t total_length = left->length + right->length;
  if (total_length < RBLIST_MINIMUM_NODE_LENGTH) {
    struct leaf *ret = zmalloc(sizeof(struct leaf) + sizeof(char) * total_length);
    ret->length = total_length;
    memcpy((char *)&left->leaf.data[0], &ret->data[0], left->length);
    memcpy((char *)&right->leaf.data[0], &ret->data[left->length], right->length);
    return (rblist)ret;
  } else {
    struct node *ret = zmalloc(sizeof(struct node));
    ret->length = total_length;
    if (random_choice(left->length, right->length)) {
      ret->left = left->node.left;
      ret->right = rblist_concat(left->node.right, right);
    } else {
      ret->left = rblist_concat(left, right->node.left);
      ret->right = right->node.right;
    }
    return (rblist)ret;
  }
}

rblist rblist_from_string(const char *s)
{
  rblist ret;
  for (; *s; s++)
    ret = rblist_concat(ret, rblist_singleton(*s));
  return ret;
}

/* char *rblist_to_string(rblist rbl) */
/* { */
/*   char *s = zmalloc(rbl->length + 1); */
/*   for (size_t i = 0; i < rbl->length; i++) */
/*     *s++ = ret = rblist_index(rbl, i); */
/* } */

#ifdef TEST

#include <assert.h>
#include <stdlib.h>

/* Stub to make zrealloc happy */
void die(int exitcode)
{
  exit(exitcode);
}

int main(void)
{
  rblist rbl1, rbl2, rbl3;

  rbl1 = rblist_singleton('a');
  rbl2 = rblist_singleton('b');
  rbl3 = rblist_concat(rbl1, rbl2);
/*   assert(!strcmp(rblist_to_string(rbl3), "ab")); */

  rbl1 = rbl_from_string("hello");
/*   assert(!strcmp(rblist_to_string(rbl1), "hello")); */

  return EXIT_SUCCESS;
}

#endif /* TEST */

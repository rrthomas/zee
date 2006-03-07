/* Randomly balanced lists.
   Copyright (c) 2006 Alistair Turnbull.
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


/* Start of section to move into a header file. */

/*
 * Tuning parameter which controls memroy/CPU compromise.
 * rblists shorter than this will be implemented using a primitive array.
 * Increase to use less memory and more CPU.
 */
#define RBLIST_MINIMUM_NODE_LENGTH 64

/* The type of lists. Instances are immutable. */
typedef const union rblist *rblist;

/* The empty list. */
extern rblist rblist_empty;

/* Make a list of length 1. */
extern rblist rblist_singleton(char c);

/* Concatenate two lists (non-destructive). */
extern rblist rblist_concat(rblist left, rblist right);

/* End of section to move into a header file. */

/* For debugging: incremented every time random_choice is called. */
static size_t choice_counter = 0;

#define SIZE_T_MAX (1 + (float)(size_t)-1)

/*
 * Returns true with probability p/(p+q).
 * Assumes (p+q) is representable as a size_t.
 */
static inline bool random_choice(size_t p, size_t q)
{
  static size_t seed = 483568341; /* Arbitrary. */
  
  choice_counter++;
  seed = (2 * seed - 1) * seed + 1; /* Maximal period mod any power of two. */
  return ((p + q) * (float)seed) < (p * SIZE_T_MAX);
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
  struct leaf *ans = zmalloc(sizeof(struct leaf) + sizeof(char));
  ans->length = 1;
  ans->data[0] = c;
  return (rblist)ans;
}

rblist rblist_concat(rblist left, rblist right)
{
  size_t length = left->length + right->length;
  if (length < RBLIST_MINIMUM_NODE_LENGTH) {
    struct leaf *ans = zmalloc(sizeof(struct leaf) + sizeof(char) * length);
    ans->length = length;
    memcpy(&left->leaf.data[0], &ans->data[0], left->length);
    memcpy(&right->leaf.data[0], &ans->data[left->length], right->length);
    return (rblist)ans;
  } else {
    struct node *ans = zmalloc(sizeof(struct node));
    ans->length = length;
    if (random_choice(left->length, right->length)) {
      ans->left = left->node.left;
      ans->right = rblist_concat(left->node.right, right);
    } else {
      ans->left = rblist_concat(left, right->node.left);
      ans->right = right->node.right;
    }
    return (rblist)ans;
  }
}

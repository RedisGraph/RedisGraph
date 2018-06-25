/*
 * Copyright (c) 2009-2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2009-2014, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* This skip list implementation is almost a C translation of the original
 * algorithm described by William Pugh in "Skip Lists: A Probabilistic
 * Alternative to Balanced Trees", modified in three ways:
 * a) this implementation allows for repeated scores.
 * b) the comparison is not just by key (our 'score') but by satellite data.
 * c) there is a back pointer, so it's a doubly linked list with the back
 * pointers being only at "level 1". This allows to traverse the list
 * from tail to head, useful to model certain problems.
 *
 * This implementation originates from the Redis code base but was modified
 * in different ways. */

#include "skiplist.h"

#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#define zmalloc malloc
#define zfree free

static inline unsigned long skiplistLength(skiplist *sl) { return sl->length; }

static inline skiplistNode *skiplistIteratorCurrent(skiplistIterator *it) {
  return it->current;
}
/*
 * Create a skip list node with the specified number of levels, pointing to
 * the specified object.
 */
skiplistNode *skiplistCreateNode(int level, void *key, void *val) {
  skiplistNode *zn = zmalloc(sizeof(*zn) + level * sizeof(struct skiplistLevel));
  zn->key = key;
  zn->vals = zmalloc(sizeof(void **));
  zn->valsAllocated = 1;
  if (val) {
    zn->vals[0] = val;
    zn->numVals = 1;
  } else {
    zn->vals[0] = NULL;
    zn->numVals = 0;
  }

  return zn;
}

skiplistNode *skiplistNodeAppendValue(skiplistNode *n, void *val,
                                      skiplistValCmpFunc cmp) {

  if (n->numVals >= n->valsAllocated) {
    n->valsAllocated *= 2;
    n->vals = realloc(n->vals, n->valsAllocated * sizeof(void *));
  }

  // Insert the new value
  n->vals[n->numVals ++] = val;

  return n;
}

/*
 * Create a new skip list with the specified function used in order to
 * compare elements. The function return value is the same as strcmp().
 */
skiplist *skiplistCreate(skiplistCmpFunc cmp, void *cmpCtx, skiplistValCmpFunc vcmp,
                         skiplistCloneKeyFunc cloneKey, skiplistFreeKeyFunc freeKey) {
  int j;
  skiplist *sl = zmalloc(sizeof(struct skiplist));
  sl->level = 1;
  sl->length = 0;
  sl->header = skiplistCreateNode(SKIPLIST_MAXLEVEL, NULL, NULL);
  for (j = 0; j < SKIPLIST_MAXLEVEL; j ++) {
    sl->header->level[j].forward = NULL;
    sl->header->level[j].span = 0;
  }
  sl->header->numVals = 0;
  sl->header->backward = NULL;
  sl->tail = NULL;
  sl->compare = cmp;
  sl->cmpCtx = cmpCtx;
  sl->valcmp = vcmp;
  sl->cloneKey = cloneKey;
  sl->freeKey = freeKey;

  return sl;
}

/* Free a skiplist node. We don't free the node's pointed object. */
void skiplistFreeNode(skiplistNode *node) {
  if (node->vals) zfree(node->vals);
  zfree(node);
}

/* Free an entire skiplist. */
void skiplistFree(skiplist *sl) {
  skiplistNode *node = sl->header->level[0].forward, *next;

  zfree(sl->header);
  while (node) {
    next = node->level[0].forward;
    skiplistFreeNode(node);
    node = next;
  }
  zfree(sl);
}

/*
 * Returns a random level for the new skiplist node we are going to create.
 * The return value of this function is between 1 and SKIPLIST_MAXLEVEL
 * (both inclusive), with a powerlaw-alike distribution where higher
 * levels are less likely to be returned.
 */
int skiplistRandomLevel(void) {
  int level = 1;
  while ((random() & 0xFFFF) < (SKIPLIST_P * 0xFFFF)) {
    level += 1;
  }
  return (level < SKIPLIST_MAXLEVEL) ? level : SKIPLIST_MAXLEVEL;
}

/*
 * Insert the specified object, return NULL if the element already
 * exists.
 */
skiplistNode *skiplistInsert(skiplist *sl, void *key, void *val) {
  skiplistNode *update[SKIPLIST_MAXLEVEL], *x;
  unsigned int rank[SKIPLIST_MAXLEVEL];
  int i, level;

  x = sl->header;
  for (i = sl->level - 1; i >= 0; i --) {
    /* store rank that is crossed to reach the insert position */
    rank[i] = (i == (sl->level - 1)) ? 0 : rank[i + 1];
    while (x->level[i].forward &&
           sl->compare(x->level[i].forward->key, key, sl->cmpCtx) < 0) {
      rank[i] += x->level[i].span;
      x = x->level[i].forward;
    }
    update[i] = x;
  }

  /* If the element is already inside, append the value to the element. */
  if (x->level[0].forward &&
      sl->compare(x->level[0].forward->key, key, sl->cmpCtx) == 0) {
    return skiplistNodeAppendValue(x->level[0].forward, val, sl->valcmp);
  }

  /* Add a new node with a random number of levels. */
  level = skiplistRandomLevel();
  if (level > sl->level) {
    for (i = sl->level; i < level; i++) {
      rank[i] = 0;
      update[i] = sl->header;
      update[i]->level[i].span = sl->length;
    }
    sl->level = level;
  }

  /*
   * If we have reached this point, the key is not already in the skiplist.
   * If the skiplist was provided with a cloneKey routine (to duplicate
   * volatile keys like SIValues), use it; otherwise the skiplistNode key
   * will be set through pointer assignment.
   */
  if (sl->cloneKey) sl->cloneKey(&key);
  x = skiplistCreateNode(level, key, val);
  for (i = 0; i < level; i++) {
    x->level[i].forward = update[i]->level[i].forward;
    update[i]->level[i].forward = x;

    /* update span covered by update[i] as x is inserted here */
    x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
    update[i]->level[i].span = (rank[0] - rank[i]) + 1;
  }

  /* increment span for untouched levels */
  for (i = level; i < sl->level; i++) {
    update[i]->level[i].span++;
  }

  x->backward = (update[0] == sl->header) ? NULL : update[0];
  if (x->level[0].forward) {
    x->level[0].forward->backward = x;
  } else {
    sl->tail = x;
  }
  sl->length++;

  return x;
}

/*
 * Internal function used by skiplistDelete, it needs an array of other
 * skiplist nodes that point to the node to delete in order to update
 * all the references of the node we are going to remove.
 */
void skiplistDeleteNode(skiplist *sl, skiplistNode *x, skiplistNode **update) {
  // Free the key contained in the skiplistNode if we specified a routine for it
  if (sl->freeKey) sl->freeKey(x->key);

  int i;
  for (i = 0; i < sl->level; i++) {
    if (update[i]->level[i].forward == x) {
      update[i]->level[i].span += x->level[i].span - 1;
      update[i]->level[i].forward = x->level[i].forward;
    } else {
      update[i]->level[i].span -= 1;
    }
  }
  if (x->level[0].forward) {
    x->level[0].forward->backward = x->backward;
  } else {
    sl->tail = x->backward;
  }
  while (sl->level > 1 && sl->header->level[sl->level - 1].forward == NULL) {
    sl->level--;
  }
  sl->length--;
}

/*
 * Delete an element from the skiplist. If the `val` argument is provided,
 * the single matching value will be found and deleted; otherwise, the skiplistNode
 * representing the key matching `key` and all of its elements will be deleted.
 * If the operation is successful, 1 is returned, otherwise the target was not found and 0 is returned.
 * The memory used to represent skiplist keys and values is not freed.
 */
int skiplistDelete(skiplist *sl, void *key, void *val) {
  skiplistNode *update[SKIPLIST_MAXLEVEL], *x;
  int i;

  x = sl->header;
  for (i = sl->level - 1; i >= 0; i--) {
    while (x->level[i].forward &&
           sl->compare(x->level[i].forward->key, key, sl->cmpCtx) < 0) {
      x = x->level[i].forward;
    }
    update[i] = x;
  }
  x = x->level[0].forward;
  if (x && sl->compare(x->key, key, sl->cmpCtx) == 0) {

    if (val) {
      // try to delete the value itself from the vallist
      int found = 0;
      for (i = 0; i < x->numVals; i++) {
        // found the value - let's delete it
        if (!sl->valcmp(val, x->vals[i])) {
          found = 1;
          // switch the found value with the top value
          if (i < x->numVals - 1) {
            x->vals[i] = x->vals[x->numVals - 1];
          }
          x->numVals--;

          // If the vals array has gotten >= 4x as large as its occupied region,
          // shrink the array
          if (x->numVals > 0 && x->valsAllocated >= x->numVals * 4) {
            x->valsAllocated = x->numVals * 2;
            x->vals = realloc(x->vals, x->valsAllocated * sizeof(val));
          }
          break;
        }
      }
      if (!found) {
        // Specified value was not found in skiplistNode
        return 0;
      }
    }

    if (!val || x->numVals == 0) {
      skiplistDeleteNode(sl, x, update);
      skiplistFreeNode(x);
    }
    return 1;
  }
  return 0; /* not found */
}

/*
 * Search for the element in the skip list, if found the
 * node pointer is returned, otherwise NULL is returned.
 */
skiplistNode* skiplistFind(skiplist *sl, void *key) {
  skiplistNode *x;
  int i;

  x = sl->header;
  for (i = sl->level - 1; i >= 0; i--) {
    while (x->level[i].forward &&
           sl->compare(x->level[i].forward->key, key, sl->cmpCtx) < 0) {
      x = x->level[i].forward;
    }
  }
  x = x->level[0].forward;
  if (x && sl->compare(x->key, key, sl->cmpCtx) == 0) {
    return x;
  } else {
    return NULL;
  }
}

/*
 * Search for the element in the skip list, if found the
 * node pointer is returned, otherwise the next pointer is returned.
 */
skiplistNode* skiplistFindAtLeast(skiplist *sl, void *key, int exclusive) {
  skiplistNode *x = sl->header;
  int i;

  for (i = sl->level - 1; i >= 0; i--) {
    while (x->level[i].forward) {
      int rc = sl->compare(x->level[i].forward->key, key, sl->cmpCtx);
      if (rc < 0 || (rc == 0 && exclusive)) {
        x = x->level[i].forward;
      } else {
        break;
      }
    }
  }
  x = x->level[0].forward;

  return x;
}

/*
 * If the skip list is empty, NULL is returned, otherwise the element
 * at head is removed and its pointed object returned.
 */
void *skiplistPopHead(skiplist *sl) {
  skiplistNode *x = sl->header;

  x = x->level[0].forward;
  if (!x) return NULL;

  void *ptr = x->key;
  skiplistDelete(sl, ptr, NULL);
  return ptr;
}

/*
 * If the skip list is empty, NULL is returned, otherwise the element
 * at tail is removed and its pointed object returned.
 */
void *skiplistPopTail(skiplist *sl) {
  skiplistNode *x = sl->tail;

  if (!x) return NULL;

  void *ptr = x->key;
  skiplistDelete(sl, ptr, NULL);
  return ptr;
}

skiplistIterator* skiplistIterateRange(skiplist *sl, void *min, void *max,
                                      int minExclusive, int maxExclusive) {
  skiplistNode *n;
  if (!min) {
    // Range begins with the first skiplist element if min is NULL
    n = sl->header->level[0].forward;
  } else {
    n = skiplistFindAtLeast(sl, min, minExclusive);
  }
  if (n && max) {
    // make sure the first item of the range is not already above the range end
    int c = sl->compare(n->key, max, sl->cmpCtx);
    // TODO: Fix comparisons to work with null functions
    if (c > 0 || (c == 0 && maxExclusive)) {
      n = NULL;
    }
  }

  skiplistIterator *iter = zmalloc(sizeof(skiplistIterator));
  iter->current = n;
  iter->currentValOffset = 0;
  iter->rangeMin = min;
  iter->minExclusive = minExclusive;
  iter->rangeMax = max;
  iter->maxExclusive = maxExclusive;
  iter->sl = sl;

  return iter;
}

skiplistIterator* skiplistIterateAll(skiplist *sl) {
  skiplistIterator *iter = calloc(1, sizeof(skiplistIterator));
  iter->current = sl->header->level[0].forward;
  iter->sl = sl;
  return iter;
}

void skiplistIterate_Reset(skiplistIterator *iter) {
  // If this iterator was built with a minimum value, we will traverse the skiplist
  // to initialize it properly.
  if (iter->rangeMin == NULL) {
    iter->current = iter->sl->header->level[0].forward;
  } else {
    iter->current = skiplistFindAtLeast(iter->sl, iter->rangeMin, iter->minExclusive);
  }

  iter->currentValOffset = 0;
}

void skiplistIterate_Free(skiplistIterator *iter) {
  zfree(iter);
}

void *skiplistIterator_Next(skiplistIterator *it) {

  if (!it->current) {
    return NULL;
  }

  void *ret = NULL;
  if (it->currentValOffset < it->current->numVals) {
    ret = it->current->vals[it->currentValOffset++];
  }

  if (it->currentValOffset == it->current->numVals) {
    it->current = it->current->level[0].forward;
    it->currentValOffset = 0;

    // make sure we don't pass the range max. NULL means +inf
    if (it->current && it->rangeMax) {
      int c = it->sl->compare(it->current->key, it->rangeMax, it->sl->cmpCtx);
      if (c > 0 || (c == 0 && it->maxExclusive)) {
        it->current = NULL;
      }
    }
  }
  return ret;
}


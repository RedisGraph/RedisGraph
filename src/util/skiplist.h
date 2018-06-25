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
 *   * Neither the name of Disque nor the names of its contributors may be used
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

#ifndef __DISQUE_SKIPLIST_H
#define __DISQUE_SKIPLIST_H

#define SKIPLIST_MAXLEVEL 32 /* Should be enough for 2^32 elements */
#define SKIPLIST_P 0.25      /* Skiplist P = 1/4 */

typedef struct skiplistNode {
  void *key;
  void **vals;
  unsigned int numVals;
  unsigned int valsAllocated;
  struct skiplistNode *backward;
  struct skiplistLevel {
    struct skiplistNode *forward;
    unsigned int span;
  } level[];
} skiplistNode;

typedef int (*skiplistCmpFunc)(void *p1, void *p2, void *ctx);
typedef int (*skiplistValCmpFunc)(const void *p1, const void *p2);

typedef void (*skiplistCloneKeyFunc)(void **key);
typedef void (*skiplistFreeKeyFunc)(void *key);

typedef struct skiplist {
  struct skiplistNode *header, *tail;
  skiplistCmpFunc compare;
  skiplistValCmpFunc valcmp;

  skiplistCloneKeyFunc cloneKey;
  skiplistFreeKeyFunc freeKey;

  void *cmpCtx;
  unsigned long length;
  int level;
} skiplist;

skiplist *skiplistCreate(skiplistCmpFunc cmp, void *cmpCtx, skiplistValCmpFunc vcmp,
                         skiplistCloneKeyFunc cloneKey, skiplistFreeKeyFunc freeKey);
void skiplistFree(skiplist *sl);
skiplistNode *skiplistInsert(skiplist *sl, void *key, void *val);
int skiplistDelete(skiplist *sl, void *key, void *val);
skiplistNode *skiplistFind(skiplist *sl, void *key);
skiplistNode *skiplistFindAtLeast(skiplist *sl, void *key, int exclusive);
void *skiplistPopHead(skiplist *sl);
void *skiplistPopTail(skiplist *sl);

typedef struct {
  skiplistNode *current;
  unsigned int currentValOffset;
  void *rangeMin;
  void *rangeMax;
  int minExclusive;
  int maxExclusive;
  skiplist *sl;
} skiplistIterator;

skiplistIterator* skiplistIterateRange(skiplist *sl, void *min, void *max,
                                      int minExclusive, int maxExclusive);

skiplistIterator* skiplistIterateAll(skiplist *sl);

void skiplistIterate_Reset(skiplistIterator *iter);
void skiplistIterate_Free(skiplistIterator *iter);

void *skiplistIterator_Next(skiplistIterator *it);

#endif

/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#ifndef __INDEX_ITER_H__
#define __INDEX_ITER_H__

#include "index.h"

typedef skiplistIterator IndexIter;

/* Build a new iterator to traverse all indexed values of the specified type. */
IndexIter* IndexIter_Create(Index *idx, SIType type);

/* Update the lower or upper bound of an index iterator based on a constant predicate filter
 * (if that filter represents a narrower bound than the current one). */
bool IndexIter_ApplyBound(IndexIter *iter, SIValue *bound, int op);

/* Returns a pointer to the next Node ID in the index, or NULL if the iterator has been depleted. */
GrB_Index* IndexIter_Next(IndexIter *iter);

/* Reset an iterator to its original position. */
void IndexIter_Reset(IndexIter *iter);

/* Free an index iterator. */
void IndexIter_Free(IndexIter *iter);

#endif

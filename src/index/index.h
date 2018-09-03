/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __INDEX_H__
#define __INDEX_H__

#include <assert.h>
#include "../redismodule.h"
#include "../graph/graph_entity.h"
#include "../stores/store.h"
#include "../util/skiplist.h"
#include "../parser/ast.h"
#include "../graph/node.h"
#include "../filter_tree/filter_tree.h"
#include "../arithmetic/tuples_iter.h"

#define INDEX_OK 1
#define INDEX_FAIL 0
#define INDEX_PREFIX "redis_graph_INDEX"

typedef skiplistIterator IndexIter;

/* Properties are not required to be of a consistent type, and index construction
 * will store values in separate string and numeric skiplists with different comparator
 * functions if necessary.
 * When building Index Scan operations, the types of values described by filters will
 * specify which skiplist should be traversed. */
typedef struct {
  size_t id;
  char *label;
  char *property;
  skiplist *string_sl;
  skiplist *numeric_sl;
} Index;

/* Index_Get attempts to retrieve an index from the Redis keyspace. */
Index* Index_Get(RedisModuleCtx *ctx, const char *graph, const char *label, char *property);

/* Index_Delete drops an index from the Redis keyspace and frees its associated constructs.  */
int Index_Delete(RedisModuleCtx *ctx, const char *graphName, Graph *g, const char *label, char *prop);

/* initializeSkiplists prepares the string and numeric skiplists for a new Index,
 * pointing them to the appropriate internal comparator routines. It is exposed in
 * the header so that it can be used by the Index load functions in index_type. */
void initializeSkiplists(Index *index);

/* Index_Create is a wrapper for the buildIndex function. It retrieves the appropriate label
 * matrix from the graph and saves the index in the Redis keyspace. */
int Index_Create(RedisModuleCtx *ctx, const char *graphName, Graph *g, const char *label, char *prop_str);

/* Prepare output text for EXPLAIN calls on "drop index" and "create index" */
const char* Index_OpPrint(AST_IndexNode *indexNode);

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

/* Free an index object and all its members (skiplists and strings) */
void Index_Free(Index *idx);

#endif

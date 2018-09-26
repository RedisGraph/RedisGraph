/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#ifndef __INDEX_UPDATES_H__
#define __INDEX_UPDATES_H__

#include "index.h"
#include "index_type.h"

/* Functions triggered by query clauses (CREATE, SET, DELETE)
 * that indirectly modify indexed values. */


/* SET OPERATIONS */
/* Index_UpdateNodeValue takes a node that has just had a property added
 * or modified and updates the index to reflect the change. */
void Index_UpdateNodeValue(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID id, EntityProperty *oldval, SIValue *newval);

/* DELETE OPERATIONS */
/* Indices_DeleteNodes accepts an array of Node IDs and removes each from all indices
 * they are represented in. */
void Indices_DeleteNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID *IDs, size_t IDCount);

/* Indices_UpdateNodeIDs accepts an array of Node IDs and updates all indices
 * each is represented in so that their new ID is stored. */
void Indices_UpdateNodeIDs(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID *IDs, NodeID *replacementIDs, size_t IDCount);

/* CREATE OPERATIONS */
/* Indices_AddNodes visits all indices described and adds
 * newly-created nodes with matching labels and properties. */
void Indices_AddNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, int *labels, NodeID start_id, NodeID end_id);

#endif

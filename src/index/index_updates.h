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

/* Indices_BuildModificationMap accepts an array of Node IDs and builds a triemap
 * in which the keys are the IDs of indices that require updates, and each value is 
 * a vector of Node IDs that must be represented in that update. If the replacement_IDs
 * argument is provided, vectors will also store destination IDs so that node migrations
 * can be handled as well as node deletions. */
TrieMap* Indices_BuildModificationMap(RedisModuleCtx *ctx, Graph *g, const char *graph_name,
                                  NodeID *IDs, NodeID *replacement_IDs, size_t IDCount);

/* SET OPERATIONS */
/* Index_UpdateNodeValue takes a node that has just had a property added
 * or modified and updates the index to reflect the change. */
void Index_UpdateNodeValue(RedisModuleCtx *ctx, LabelStore *store, const char *graphName,
                           NodeID id, EntityProperty *oldval, SIValue *newval);

/* DELETE OPERATIONS */
/* Indices_DeleteNodes visits all indices described by a triemap and removes
 * nodes scheduled for deletion. */
void Indices_DeleteNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, TrieMap *delete_map);

/* Indices_UpdateNodeIDs visits all indices described by a triemap and updates the specified node IDs
 * so that migrated nodes remain accurately represented. */
void Indices_UpdateNodeIDs(RedisModuleCtx *ctx, Graph *g, const char *graph_name, TrieMap *update_map);

/* CREATE OPERATIONS */
/* Indices_DeleteNodes visits all indices described by a triemap and adds
 * newly-created nodes with matching properties. */
void Indices_AddNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, TrieMap *create_map);

#endif

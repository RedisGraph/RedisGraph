/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "ast_shared.h"
#include "../graph/query_graph.h"
#include "../graph/entities/graph_entity.h"

#define DIR_DESC -1
#define DIR_ASC 1

// Unwind operations hold a collection of arithmetic expressions.
typedef struct {
	AR_ExpNode *exp;
} AST_UnwindContext;

typedef struct {
	NodeCreateCtx *nodes_to_merge;
	EdgeCreateCtx *edges_to_merge;
} AST_MergeContext;

typedef struct {
	NodeCreateCtx *nodes_to_create;
	EdgeCreateCtx *edges_to_create;
} AST_CreateContext;

// Convert the properties specified in a node or edge entity into a set of string keys and SIValue values.
PropertyMap *AST_ConvertPropertiesMap(const cypher_astnode_t *props);

// Extract the necessary information to populate an update operation from a SET clause.
EntityUpdateEvalCtx *AST_PrepareUpdateOp(const cypher_astnode_t *set_clause, uint *nitems_ref);

// Extract the necessary information to populate a delete operation from a DELETE clause.
void AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause, const QueryGraph *qg,
						 const char ***nodes_ref, const char ***edges_ref);

// Determine whether a sort operation should be ascending or descending
int AST_PrepareSortOp(const cypher_astnode_t *order_clause);

// Extract the necessary information to populate a unwind operation from an UNWIND clause.
AST_UnwindContext AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause);

// Extract the necessary information to populate a merge operation from a MERGE clause.
AST_MergeContext AST_PrepareMergeOp(const cypher_astnode_t *merge_clause, QueryGraph *qg,
									const char **bound_variables);

// Extract the necessary information to populate a create operation from a CREATE clause.
AST_CreateContext AST_PrepareCreateOp(QueryGraph *qg, rax *bound_variables);


/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "ast_shared.h"
#include "../execution_plan/record_map.h"
#include "../graph/query_graph.h"
#include "../graph/entities/graph_entity.h"

#define DIR_DESC -1
#define DIR_ASC 1

// TODO adopt this style for other functions
typedef struct {
    AR_ExpNode **exps;
    uint record_idx;
} AST_UnwindContext;

typedef struct {
    NodeCreateCtx *nodes_to_merge;
    EdgeCreateCtx *edges_to_merge;
} AST_MergeContext;

typedef struct {
    NodeCreateCtx *nodes_to_create;
    EdgeCreateCtx *edges_to_create;
} AST_CreateContext;

int TraverseRecordCap(const AST *ast);

PropertyMap* AST_ConvertPropertiesMap(const cypher_astnode_t *props, RecordMap *record_map);

AR_ExpNode** AST_ConvertCollection(const cypher_astnode_t *collection, RecordMap *record_map);

EntityUpdateEvalCtx* AST_PrepareUpdateOp(const cypher_astnode_t *set_clause, RecordMap *record_map, uint *nitems_ref);

void AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause, const QueryGraph *qg, RecordMap *record_map, uint **nodes_ref, uint **edges_ref);

int AST_PrepareSortOp(const cypher_astnode_t *order_clause);

AST_UnwindContext AST_PrepareUnwindOp(const cypher_astnode_t *unwind_clause, RecordMap *record_map);

AST_MergeContext AST_PrepareMergeOp(RecordMap *record_map, AST *ast, const cypher_astnode_t *merge_clause, QueryGraph *qg);

AST_CreateContext AST_PrepareCreateOp(GraphContext *gc, RecordMap *record_map, AST *ast, QueryGraph *qg);


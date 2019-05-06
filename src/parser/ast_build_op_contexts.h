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

// TODO adopt this style for other functions
typedef struct {
    AR_ExpNode **exps;
    const char *alias;
    uint record_len;
    uint record_idx;
} AST_UnwindContext;

typedef struct {
    NodeCreateCtx *nodes_to_merge;
    EdgeCreateCtx *edges_to_merge;
    uint record_len;
} AST_MergeContext;

typedef struct {
    NodeCreateCtx *nodes_to_create;
    EdgeCreateCtx *edges_to_create;
    uint record_len;
} AST_CreateContext;

int TraverseRecordCap(const AST *ast);

PropertyMap* AST_ConvertPropertiesMap(const AST *ast, const cypher_astnode_t *props);

AR_ExpNode** AST_ConvertCollection(const cypher_astnode_t *collection);

EntityUpdateEvalCtx* AST_PrepareUpdateOp(const cypher_astnode_t *set_clause, uint *nitems_ref);

void AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause, uint **nodes_ref, uint **edges_ref);

AR_ExpNode** AST_PrepareSortOp(const cypher_astnode_t *order_clause, int *direction);

AST_UnwindContext AST_PrepareUnwindOp(const AST *ast, const cypher_astnode_t *unwind_clause);

AST_MergeContext AST_PrepareMergeOp(AST *ast, const cypher_astnode_t *merge_clause, QueryGraph *qg);

AST_CreateContext AST_PrepareCreateOp(AST *ast, QueryGraph *qg);

const char** AST_PrepareWithOp(const cypher_astnode_t *with_clause);

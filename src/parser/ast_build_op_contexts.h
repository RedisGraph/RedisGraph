/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "ast.h"
#include "../graph/entities/graph_entity.h"

#define DIR_DESC -1
#define DIR_ASC 1

// Context describing an update expression.
typedef struct {
    const char *attribute;          /* Attribute name to update. */
    Attribute_ID attribute_idx;     /* Attribute internal ID. */
    int entityRecIdx;               /* Position of entity within record. */
    AR_ExpNode *exp;                /* Expression to evaluate. */
} EntityUpdateEvalCtx;

typedef struct {
    const char **keys;
    SIValue *values;
    int property_count;
} PropertyMap;

// TODO adopt this style for other functions
typedef struct {
    AR_ExpNode **exps;
    const char *alias;
    uint record_len;
    uint record_idx;
} AST_UnwindContext;

typedef struct {
    AR_ExpNode **exps;
    const char *alias;
    uint record_len;
    uint record_idx;
} AST_MergeContext;

int TraverseRecordCap(const AST *ast);

PropertyMap* AST_ConvertPropertiesMap(const AST *ast, const cypher_astnode_t *props);

AR_ExpNode** AST_ConvertCollection(const cypher_astnode_t *collection);

EntityUpdateEvalCtx* AST_PrepareUpdateOp(const cypher_astnode_t *set_clause, uint *nitems_ref);

void AST_PrepareDeleteOp(const cypher_astnode_t *delete_clause, uint **nodes_ref, uint **edges_ref);

AR_ExpNode** AST_PrepareSortOp(const cypher_astnode_t *order_clause, int *direction);

AST_UnwindContext AST_PrepareUnwindOp(const AST *ast, const cypher_astnode_t *unwind_clause);

AST_MergeContext AST_PrepareMergeOp(const AST *ast, const cypher_astnode_t *merge_clause);

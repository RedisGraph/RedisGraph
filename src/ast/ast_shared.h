/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../graph/graphcontext.h"
#include "../graph/entities/qg_node.h"
#include "../graph/entities/qg_edge.h"
#include "../arithmetic/arithmetic_expression.h"

struct AR_ExpNode;

typedef enum
{
    OP_NULL,
    OP_OR,
    OP_XOR,
    OP_AND,
    OP_NOT,
    OP_EQUAL,
    OP_NEQUAL,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
    OP_MOD,
    OP_POW,
    OP_CONTAINS,
    OP_STARTSWITH,
    OP_ENDSWITH,
    OP_IN
} AST_Operator;

typedef struct
{
    Attribute_ID *keys;
    struct AR_ExpNode **values;
    int property_count;
} PropertyMap;

// Context describing an update expression.
typedef struct
{
    const char *attribute;      /* Attribute name to update. */
    Attribute_ID attribute_idx; /* Attribute internal ID. */
    uint entityRecIdx;          /* Position of entity within record. */
    struct AR_ExpNode *exp;     /* Expression to evaluate. */
} EntityUpdateEvalCtx;

// Context describing a node in a CREATE or MERGE clause
typedef struct
{
    QGEdge *edge;
    PropertyMap *properties;
    uint src_idx;
    uint dest_idx;
    uint edge_idx;
} EdgeCreateCtx;

// Context describing a relationship in a CREATE or MERGE clause
typedef struct
{
    QGNode *node;
    PropertyMap *properties;
    uint node_idx;
} NodeCreateCtx;

AST_Operator AST_ConvertOperatorNode(const cypher_operator_t *op);

// Convert a map of properties from the AST into a set of attribute ID keys and AR_ExpNode values.
PropertyMap *PropertyMap_New(GraphContext *gc, const cypher_astnode_t *props,
                             RecordMap *record_map);

void PropertyMap_Free(PropertyMap *map);

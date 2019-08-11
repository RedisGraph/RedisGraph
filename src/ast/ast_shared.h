/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../graph/entities/qg_node.h"
#include "../graph/entities/qg_edge.h"
#include "../graph/entities/graph_entity.h"
#include "../arithmetic/arithmetic_expression.h"

struct AR_ExpNode;

typedef enum {
	OP_NULL,
	OP_OR,
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
	OP_POW
} AST_Operator;

typedef struct {
	SIValue *values; // TODO Replace with AR_ExpNodes
	const char **keys;
	int property_count;
    Attribute_ID *attr_ids;
} PropertyMap;

// Context describing an update expression.
typedef struct {
	const char *attribute;          /* Attribute name to update. */
	Attribute_ID attribute_idx;     /* Attribute internal ID. */
	const char *alias;              /* entity being updated. */
    int entityRecIdx;               /* entity position within Record. */
	struct AR_ExpNode *exp;         /* Expression to evaluate. */
} EntityUpdateEvalCtx;

// Context describing a node in a CREATE or MERGE clause
typedef struct {
	int rec_idx;
	int src_rec_idx;
	int dest_rec_idx;
    QGEdge *edge;
	PropertyMap *properties;    // TODO: consider migrating properties to QGEdge.
} EdgeCreateCtx;

// Context describing a relationship in a CREATE or MERGE clause
typedef struct {
	int rec_idx;
    QGNode *node;
	PropertyMap *properties;    // TODO: consider migrating properties to QGNode.
} NodeCreateCtx;

AST_Operator AST_ConvertOperatorNode(const cypher_operator_t *op);

void PropertyMap_Free(PropertyMap *map);

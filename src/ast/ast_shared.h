/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../graph/graphcontext.h"
#include "../graph/entities/qg_node.h"
#include "../graph/entities/qg_edge.h"
#include "../arithmetic/arithmetic_expression.h"
#include "ast.h"

struct AR_ExpNode;

/* Updates to this enum require parallel updates to the
 * OpName array in arithmetic_expression_construct.c */
typedef enum {
	OP_UNKNOWN = 0,
	OP_NULL = 1,
	OP_OR = 2,
	OP_XOR = 3,
	OP_AND = 4,
	OP_NOT = 5,
	OP_EQUAL = 6,
	OP_NEQUAL = 7,
	OP_LT = 8,
	OP_GT = 9,
	OP_LE = 10,
	OP_GE = 11,
	OP_PLUS = 12,
	OP_MINUS = 13,
	OP_MULT = 14,
	OP_DIV = 15,
	OP_MOD = 16,
	OP_POW = 17,
	OP_CONTAINS = 18,
	OP_STARTSWITH = 19,
	OP_ENDSWITH = 20,
	OP_IN = 21,
	OP_IS_NULL = 22,
	OP_IS_NOT_NULL = 23
} AST_Operator;

typedef struct {
	Attribute_ID *keys;
	struct AR_ExpNode **values;
	int property_count;
} PropertyMap;

// Context describing an update expression.
typedef struct {
	const char *alias;          // alias of entity being updated
	Attribute_ID attribute_id;  // id of attribute to update
	int record_idx;             // record offset this entity is stored at
	struct AR_ExpNode *exp;     // expression to evaluate
} EntityUpdateEvalCtx;

// Context describing a node in a CREATE or MERGE clause
typedef struct {
	int src_idx;                // source node record index
	int dest_idx;               // destination node record index
	int edge_idx;               // edge record index
	int reltypeId;              // edge relationship type id
	const char *src;            // source node alias
	const char *dest;           // destination node alias
	const char *alias;          // edge alias
	const char *relation;       // edge relationship type
	PropertyMap *properties;    // edge properties set
} EdgeCreateCtx;

// Context describing a relationship in a CREATE or MERGE clause
typedef struct {
	int labelId;                // node label id
	int node_idx;               // node record index
	const char *alias;          // node alias
	const char *label;          // node label
	PropertyMap *properties;    // node properties set
} NodeCreateCtx;

AST_Operator AST_ConvertOperatorNode(const cypher_operator_t *op);

// Convert a map of properties from the AST into a set of attribute ID keys and AR_ExpNode values.
PropertyMap *PropertyMap_New(GraphContext *gc, const cypher_astnode_t *props);

// Clone EntityUpdateEvalCtx.
EntityUpdateEvalCtx EntityUpdateEvalCtx_Clone(EntityUpdateEvalCtx ctx);

// Clone NodeCreateCtx.
NodeCreateCtx NodeCreateCtx_Clone(NodeCreateCtx ctx);

// Clone EdgeCreateCtx.
EdgeCreateCtx EdgeCreateCtx_Clone(EdgeCreateCtx ctx);

void PropertyMap_Free(PropertyMap *map);


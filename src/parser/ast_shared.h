/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../arithmetic/arithmetic_expression.h"

typedef struct {
    const char **keys;
    SIValue *values;
    int property_count;
} PropertyMap;

// Context describing an update expression.
typedef struct {
    const char *attribute;          /* Attribute name to update. */
    Attribute_ID attribute_idx;     /* Attribute internal ID. */
    uint entityRecIdx;              /* Position of entity within record. */
    AR_ExpNode *exp;                /* Expression to evaluate. */
} EntityUpdateEvalCtx;

// Context describing a node in a CREATE clause
typedef struct {
    Edge *edge;
    PropertyMap *properties;
    uint src_idx;
    uint dest_idx;
    uint edge_idx;
} EdgeCreateCtx;

// Context describing a relationship in a CREATE clause
typedef struct {
    Node *node;
    PropertyMap *properties;
    uint node_idx;
} NodeCreateCtx;

void PropertyMap_Free(PropertyMap *map);

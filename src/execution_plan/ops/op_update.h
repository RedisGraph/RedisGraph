/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_UPDATE_H
#define __OP_UPDATE_H

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../resultset/resultset.h"
#include "../../arithmetic/arithmetic_expression.h"

// Context describing an update expression.
typedef struct {
    AST_GraphEntity *ge;            /* Referred entity in MATCH clause. */
    char *attribute;                /* Attribute name to update. */
    Attribute_ID attribute_idx;     /* Attribute internal ID. */
    int entityRecIdx;               /* Position of entity within record. */
    AR_ExpNode *exp;                /* Expression to evaluate. */
} EntityUpdateEvalCtx;

// Context describing a pending update to perform.
typedef struct {
    AST_GraphEntity *ge;                /* Referred entity in MATCH clause. */
    char *attribute;                    /* Attribute name to update. */
    Attribute_ID attribute_idx;         /* Attribute internal ID. */
    Entity *entity_reference;           /* Graph entity to update. */
    SIValue new_value;                  /* Constant value to set. */
} EntityUpdateCtx;

typedef struct {
    OpBase op;
    AST *ast;
    GraphContext *gc;
    ResultSet *result_set;

    size_t update_expressions_count;
    EntityUpdateEvalCtx *update_expressions;    /* List of entities to update and their arithmetic expressions. */

    size_t pending_updates_cap;
    size_t pending_updates_count;
    EntityUpdateCtx *pending_updates;           /* List of entities to update and their actual new value. */
} OpUpdate;

OpBase* NewUpdateOp(GraphContext *gc, AST *ast, ResultSet *result_set);
Record OpUpdateConsume(OpBase *opBase);
OpResult OpUpdateReset(OpBase *ctx);
void OpUpdateFree(OpBase *ctx);

#endif /* __OP_UPDATE_H */

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

typedef struct {
    AST_GraphEntity *ge;    /* Referred entity in MATCH clause. */
    int entityRecIdx;       /* Position of entity within record. */
    char *property;         /* Property to update. */
    AR_ExpNode *exp;        /* Expression to evaluate. */
} EntityUpdateEvalCtx;

typedef struct {
    Entity *entity_reference;
    int prop_idx;
    SIValue new_value;                  /* Constant value to set. */
    AST_GraphEntity *ge;                /* Referred entity in MATCH clause. */
} EntityUpdateCtx;

typedef struct {
    OpBase op;
    ResultSet *result_set;
    EntityUpdateEvalCtx *update_expressions;  /* List of entities to update and their arithmetic expressions. */
    size_t update_expressions_count;
    EntityUpdateCtx *entities_to_update;    /* List of entities to update and their actual new value. */
    size_t entities_to_update_cap;
    size_t entities_to_update_count;
    AST *ast;
    GraphContext *gc;
} OpUpdate;

OpBase* NewUpdateOp(GraphContext *gc, AST *ast, ResultSet *result_set);
OpResult OpUpdateConsume(OpBase *opBase, Record r);
OpResult OpUpdateReset(OpBase *ctx);
void OpUpdateFree(OpBase *ctx);

#endif /* __OP_UPDATE_H */


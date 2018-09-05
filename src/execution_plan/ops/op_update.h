/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_UPDATE_H
#define __OP_UPDATE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../resultset/resultset.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../index/index.h"

typedef struct {
    GraphEntity **entity;   /* Entity to update. */
    char *alias;
    char *property;         /* Property to update. */
    AR_ExpNode *exp;        /* Expression to evaluate. */
} EntityUpdateEvalCtx;

typedef struct {
    char *alias; // can also use label
    NodeID id;
    EntityProperty *dest_entity_prop;   /* Entity's property to update. */
    SIValue new_value;                  /* Constant value to set. */
} EntityUpdateCtx;

typedef struct {
    OpBase op;
    ResultSet *result_set;
    EntityUpdateEvalCtx *update_expressions;  /* List of entities to update and their arithmetic expressions. */
    size_t update_expressions_count;
    EntityUpdateCtx *entities_to_update;    /* List of entities to update and their actual new value. */
    size_t entities_to_update_cap;
    size_t entities_to_update_count;
    AST_Query *ast;
    RedisModuleCtx *ctx;
    const char *graphName;
} OpUpdate;

OpBase* NewUpdateOp(RedisModuleCtx *ctx, AST_Query *ast, QueryGraph *q, ResultSet *result_set, const char *graphName);
OpResult OpUpdateConsume(OpBase *opBase, QueryGraph *graph);
OpResult OpUpdateReset(OpBase *ctx);
void OpUpdateFree(OpBase *ctx);

#endif /* __OP_UPDATE_H */
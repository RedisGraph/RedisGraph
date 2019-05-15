/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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
    const char *attribute;          /* Attribute name to update. */
    Attribute_ID attribute_idx;     /* Attribute internal ID. */
    int entityRecIdx;               /* Position of entity within record. */
    AR_ExpNode *exp;                /* Expression to evaluate. */
} EntityUpdateEvalCtx;

// Context describing a pending update to perform.
typedef struct {    
    const char *attribute;              /* Attribute name to update. */
    Node n;
    Edge e;
    GraphEntityType entity_type;        /* Graph entity type. */
    SIValue new_value;                  /* Constant value to set. */
} EntityUpdateCtx;

typedef struct {
    OpBase op;
    NEWAST *ast;
    GraphContext *gc;
    ResultSet *result_set;

    uint update_expressions_count;
    EntityUpdateEvalCtx *update_expressions;    /* List of entities to update and their arithmetic expressions. */

    uint pending_updates_cap;
    uint pending_updates_count;
    EntityUpdateCtx *pending_updates;           /* List of entities to update and their actual new value. */
    Record *records;                            /* Updated records, used only when query inspects updated entities. */
    bool updates_commited;                      /* Updates performed? */
} OpUpdate;

OpBase* NewUpdateOp(GraphContext *gc, ResultSet *result_set);
OpResult OpUpdateInit(OpBase *opBase);
Record OpUpdateConsume(OpBase *opBase);
OpResult OpUpdateReset(OpBase *ctx);
void OpUpdateFree(OpBase *ctx);

#endif /* __OP_UPDATE_H */

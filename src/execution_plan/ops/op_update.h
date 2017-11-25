#ifndef __OP_UPDATE_H
#define __OP_UPDATE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../resultset/resultset.h"
#include "../../arithmetic_expression.h"

typedef struct {
    GraphEntity **entity;   /* Entity to update. */
    char *property;         /* Property to update. */
    AR_ExpNode *exp;        /* Expression to evaluate. */
} EntityUpdateEvalCtx;

typedef struct {
    EntityProperty *dest_entity_prop;   /* Entity's property to update. */
    SIValue new_value;                  /* Constant value to set. */
} EntityUpdateCtx;

typedef struct {
    OpBase op;
    int request_refresh;
    ResultSet *result_set;
    EntityUpdateEvalCtx *update_expressions;  /* List of entities to update and their arithmetic expressions. */
    size_t update_expressions_count;
    EntityUpdateCtx *entities_to_update;    /* List of entities to update and their actual new value. */
    size_t entities_to_update_cap;
    size_t entities_to_update_count;
} OpUpdate;

OpBase* NewUpdateOp(AST_SetNode *ast, Graph *graph, ResultSet *result_set);
OpResult OpUpdateConsume(OpBase *opBase, Graph* graph);
OpResult OpUpdateReset(OpBase *ctx);
void OpUpdateFree(OpBase *ctx);

#endif /* __OP_UPDATE_H */
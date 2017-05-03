#ifndef __EXECUTION_PLAN_H__
#define __EXECUTION_PLAN_H__

#include "./ops/op.h"
#include "../parser/ast.h"
#include "../resultset/resultset.h"


struct OpNode;

typedef struct {
    OpBase *operation;          // node operation
    struct OpNode **children;   // child operations
    int childCount;             // number of children
} OpNode;

OpNode* NewOpNode(OpBase *op);
void OpNode_Free(OpNode* op);

void OpNode_AddChild(OpNode* parent, OpNode* child);

typedef struct {
    OpNode *root;
    Graph *graph;
} ExecutionPlan;

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, RedisModuleString *graph, QueryExpressionNode *ast);

/* Prints execution plan */
char* ExecutionPlanPrint(const ExecutionPlan *plan);

/* Executes plan */
ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlanFree(ExecutionPlan *plan);

#endif
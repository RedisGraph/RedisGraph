#ifndef __EXECUTION_PLAN_H__
#define __EXECUTION_PLAN_H__

#include "./ops/op.h"
#include "../parser/ast.h"
#include "../resultset/resultset.h"
#include "../filter_tree/filter_tree.h"


/* StreamState
 * Different states in which stream can be at. */
typedef enum {
    StreamUnInitialized,
    StreamConsuming,
    StreamDepleted,
} StreamState;

struct OpNode;

struct OpNode {
    OpBase *operation;          /* Node operation. */
    struct OpNode **children;   /* Child operations. */
    int childCount;             /* Number of children. */
    struct OpNode **parents;    /* Parent operations. */
    int parentCount;            /* Number of parents. */
    StreamState state;          /* Stream state. */
};

typedef struct OpNode OpNode;

OpNode* NewOpNode(OpBase *op);
void OpNode_Free(OpNode* op);


typedef struct {
    OpNode *root;
    Graph *graph;
    FT_FilterNode *filter_tree;
    const char *graph_name;
    ResultSet *result_set;
} ExecutionPlan;

/* Creates a new execution plan from AST */
ExecutionPlan *NewExecutionPlan(RedisModuleCtx *ctx, const char *graph, AST_QueryExpressionNode *ast);

/* Prints execution plan */
char* ExecutionPlanPrint(const ExecutionPlan *plan);

/* Executes plan */
ResultSet* ExecutionPlan_Execute(ExecutionPlan *plan);

/* Free execution plan */
void ExecutionPlanFree(ExecutionPlan *plan);

#endif
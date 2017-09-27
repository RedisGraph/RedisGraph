#ifndef __OP_CREATE_H
#define __OP_CREATE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../resultset/resultset.h"
/* Creats new entities according to the CREATE clause. */

typedef struct {
    Edge *original_edge;
    Edge **original_edge_ref;
    char *src_node_alias;
    char *dest_node_alias;
} EdgeCreateCtx;

typedef struct {
    Node *original_node;
    Node **original_node_ref;
} NodeCreateCtx;

typedef struct {
    OpBase op;
    RedisModuleCtx *ctx;
    int request_refresh;
    Graph *graph;
    const char *graph_name;

    NodeCreateCtx *nodes_to_create;
    size_t node_count;

    EdgeCreateCtx *edges_to_create;
    size_t edge_count;

    Vector *created_nodes;
    Vector *created_edges;
    ResultSet *result_set;
} OpCreate;

OpBase* NewCreateOp(RedisModuleCtx *ctx, Graph *graph, const char *graph_name, int request_refresh, ResultSet *result_set);
OpCreate* NewCreate(RedisModuleCtx *ctx, Graph *graph, const char *graph_name, int request_refresh, ResultSet *result_set);

OpResult OpCreateConsume(OpBase *opBase, Graph* graph);
OpResult OpCreateReset(OpBase *ctx);
void OpCreateFree(OpBase *ctx);

#endif
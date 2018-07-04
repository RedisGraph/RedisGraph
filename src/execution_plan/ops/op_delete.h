#ifndef __OP_DELETE_H
#define __OP_DELETE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../resultset/resultset.h"
#include "../../util/triemap/triemap.h"
/* Delets entities specified within the DELETE clause. */

typedef struct {
    Node **src;
    Node **dest;
} EdgeEnds;

typedef struct {
    OpBase op;
    int request_refresh;
    Graph *g;
    QueryGraph *qg;

    Node ***nodes_to_delete;
    size_t node_count;

    EdgeEnds *edges_to_delete;
    size_t edge_count;

    TrieMap *deleted_nodes;
    TrieMap *deleted_edges;
    ResultSet *result_set;
} OpDelete;

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, ResultSet *result_set);
OpDelete* _NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, ResultSet *result_set);

OpResult OpDeleteConsume(OpBase *opBase, QueryGraph* graph);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);

#endif
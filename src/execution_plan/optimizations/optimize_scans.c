#include "optimizer.h"

/*
   In general, there will be far fewer label scans in this project iteration.
   Most ops will work on matrices, especially given that most queries will be traversals
   operating on AlgebraicExpressions.

   I think the right idea is probably to try to decompose the query parts and then use indices
   to quickly construct the initial matrices, where possible.

   MATCH (a)-[r]->(b) WHERE a.x < ? AND b.y < ?

   Allows for building a and b matrices from indices on x and y prior to the AlgebraicExpression
   (assuming indices exist)
 */

void _substituteIndexScans(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g, OpNode *op) {
  if (op == NULL) return;

  NodeByLabelScan *scan_op;
  OpNode *index_op;
  char *label;
  if (op->operation->type == OPType_FILTER) {
    if (op->childCount == 1 && op->children[0]->operation->type == OPType_NODE_BY_LABEL_SCAN) {
      scan_op = (NodeByLabelScan*)op->children[0]->operation;
      Vector *filters = FilterTree_CollectAliasConsts(plan->filter_tree,
          QueryGraph_GetNodeAlias(plan->graph, *scan_op->node));
      if (filters != NULL) {
        IndexCreateIter *iter = Index_IntersectFilters(ctx, graph_name, filters, (*scan_op->node)->label);
        Vector_Free(filters);
        if (iter != NULL) {
          index_op = NewOpNode(NewIndexScanOp(plan->graph, g, scan_op->node, iter));
          NodeByLabelScanFree((OpBase*)scan_op);
          op->children[0] = index_op;
        }
      }
    }

  }
  if (op->childCount <= 0) return;
  return _substituteIndexScans(ctx, plan, graph_name, g, op->children[0]);
}

void substituteIndexScans(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g) {
  return _substituteIndexScans(ctx, plan, graph_name, g, plan->root);
}

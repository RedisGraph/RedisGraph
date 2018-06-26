#include "optimizer.h"

void _substituteIndexScans(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, OpNode *op) {
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
        IndexIterator *iter = Index_IntersectFilters(ctx, graph_name, filters, (*scan_op->node)->label);
        Vector_Free(filters);
        if (iter != NULL) {
          index_op = NewOpNode(NewIndexScanOp(plan->graph, scan_op->node, iter));
          NodeByLabelScanFree((OpBase*)scan_op);
          op->children[0] = index_op;
        }
      }
    }

  }
  if (op->childCount <= 0) return;
  return _substituteIndexScans(ctx, plan, graph_name, op->children[0]);
}

void substituteIndexScans(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name) {
  return _substituteIndexScans(ctx, plan, graph_name, plan->root);
}

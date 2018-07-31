#include "utilize_indices.h"
#include "../ops/op_index_scan.h"

/* The utilizeIndices optimization finds Label Scan operations with Filter parents and, if
 * any constant predicate filter matches a viable index, replaces the Label Scan with an Index Scan.
 * This allows for the consideration of fewer candidate nodes and significantly increases the speed
 * of the operation.
 */
/*
void _utilizeIndices(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g, OpNode *op) {
  if (op == NULL) return;

  OpNode *scan_op;
  Node **scan_target;
  OpNode *index_op;
  char *label;
  if (op->operation->type == OPType_FILTER) {
    if (op->children[0]->operation->type == OPType_NODE_BY_LABEL_SCAN) {
      scan_op = op->children[0];
      scan_target = ((NodeByLabelScan*)scan_op->operation)->node;
      // TODO replace with better FT traversal
      Vector *filters = FilterTree_CollectAliasConsts(((Filter*)op->operation)->filterTree,
          QueryGraph_GetNodeAlias(plan->graph, *scan_target));
      if (filters != NULL) {
        IndexIter *iter = Index_IntersectFilters(ctx, graph_name, filters, (*scan_target)->label);
        Vector_Free(filters);
        if (iter != NULL) {
          index_op = NewOpNode(NewIndexScanOp(plan->graph, g, scan_target, iter));
          ExecutionPlan_ReplaceOp(scan_op, index_op);
        }
      }
    }
  }

  for (int i = 0; i < op->childCount; i ++) {
    _utilizeIndices(ctx, plan, graph_name, g, op->children[i]);
  }
}

void utilizeIndices(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g) {
  return _utilizeIndices(ctx, plan, graph_name, g, plan->root);
}
*/

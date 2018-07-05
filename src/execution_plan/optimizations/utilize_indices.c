#include "utilize_indices.h"
#include "../ops/op_index_scan.h"

/* The utilizeIndices optimization finds Label Scan operations with Filter parents and, if
 * any constant predicate filter matches a viable index, replaces the Label Scan with an Index Scan.
 * This allows for the consideration of fewer candidate nodes and significantly increases the speed
 * of the operation.
 */
void _utilizeIndices(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g, OpNode *op) {
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
  return _utilizeIndices(ctx, plan, graph_name, g, op->children[0]);
}

void utilizeIndices(RedisModuleCtx *ctx, ExecutionPlan *plan, const char *graph_name, Graph *g) {
  return _utilizeIndices(ctx, plan, graph_name, g, plan->root);
}

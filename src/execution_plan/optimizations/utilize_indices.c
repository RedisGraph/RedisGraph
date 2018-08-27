#include "utilize_indices.h"
#include "../ops/op_index_scan.h"

void _locateScanFilters(NodeByLabelScan *scanOp, Vector *filterOps) {
  /* We begin with a LabelScan, and want to find const filters that modify
   * the active entity. */
  OpBase *current = scanOp->op.parent;
  while(current->type == OPType_FILTER) {
    Filter *filterOp = (Filter*)current;
    FT_FilterNode *filterTree = filterOp->filterTree;

    /* filterTree will either be a predicate or a tree with an OR root.
     * We'll store ops on const predicate filters, and can otherwise safely ignore them -
     * no filter tree in this sequence can invalidate another. */
    /*
    if (IsNodeConstantPredicate(filterTree)) {
      Vector_Push(filterOps, current);
    }
    */

    // Advance to the next operation.
    current = current->parent;
  }
}

// Populate scanOps vector with execution plan scan operations.
void _locateScanOp(OpBase *root, Vector *scanOps) {

  // Is this a scan operation?
  if (root->type == OPType_NODE_BY_LABEL_SCAN) {
    Vector_Push(scanOps, root);
  }

  // Continue scanning.
  for (int i = 0; i < root->childCount; i++) {
    _locateScanOp(root->children[i], scanOps);
  }
}

void utilizeIndices(RedisModuleCtx *ctx, const char *graph_name, ExecutionPlan *plan) {
  // Collect all label scans
  Vector *scanOps = NewVector(NodeByLabelScan*, 0);
  _locateScanOp(plan->root, scanOps);

  // Collect all filters on scanned entities
  NodeByLabelScan *scanOp;
  Vector *filterOps = NewVector(OpBase*, 0);
  FT_FilterNode *ft;
  // char *label;

  IndexIter *iter = NULL;
  Index *idx = NULL;

  while (Vector_Pop(scanOps, &scanOp)) {
    /* Get the label string for the scan target.
     * The label will be used to retrieve the index. */
    // label = (*scanOp->node)->label;
    Vector_Clear(filterOps);
    _locateScanFilters(scanOp, filterOps);

    // No filters.
    if (Vector_Size(filterOps) == 0) continue;

    /* At this point we have all the filter ops (and thus, filter trees) associated
     * with the scanned entity. If there are valid indices on any filter and no
     * equal or higher precedence OR filters, we can switch to an index scan.
     *
     * We'll currently use the first matching index, but apply all the filters on
     * that property. A later optimization would be to find the index with the
     * most filters, or use some heuristic for trying to select the minimal range. */

    for (int i = 0; i < Vector_Size(filterOps); i ++) {
      OpBase *opFilter;
      Vector_Get(filterOps, i, &opFilter);
      ft = ((Filter *)opFilter)->filterTree;

      // If we've already selected an index on a different property, continue
      /*
      if (idx && strcmp(idx->property, ft->pred.Lop.property)) continue;

      // Try to retrieve an index if one has not been selected yet
      if (!idx) {
        idx = Index_Get(ctx, graph_name, label, ft->pred.Lop.property);
        if (!idx) continue;
        iter = IndexIter_Create(idx, ft->pred.constVal.type);
      }
      */

      // Tighten the iterator range if possible
      if (IndexIter_ApplyBound(iter, &ft->pred)) {
        // Remove filter operations that have been folded into the index scan iterator
        ExecutionPlan_RemoveOp(opFilter);
        OpBase_Free(opFilter);
      }
    }

    if (iter != NULL) {
      OpBase *indexOp = NewIndexScanOp(plan->graph, scanOp->g, scanOp->node, iter);
      ExecutionPlan_ReplaceOp((OpBase*)scanOp, indexOp);
    }
  }

  // Cleanup
  Vector_Free(filterOps);
  Vector_Free(scanOps);
}


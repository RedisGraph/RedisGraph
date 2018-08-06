#include "utilize_indices.h"
#include "../ops/op_index_scan.h"

void _locateScanFilters(NodeByLabelScan *scanOp, Vector *filterOps) {
  /* We begin with a LabelScan, and want to find const filters that modify
   * the active entity. */
  OpBase *current = scanOp->op.parent;
  while(current->type == OPType_FILTER) {
    /* We're only interested in filters which apply to a single entity,
     * Extract modified aliases from filter tree. */
    Filter *filterOp = (Filter*)current;
    FT_FilterNode *filterTree = filterOp->filterTree;
    Vector *filteredEntities = FilterTree_CollectAliases(filterTree);

    /* At this point we're promised that if a filter operation is
     * applied to a single entity E, then E is modified by given traverse op. */
    if (Vector_Size(filteredEntities) == 1) {
      Vector_Push(filterOps, current);
    }
    Vector_Free(filteredEntities);

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

void updateScanOps(RedisModuleCtx *ctx, const char *graph_name, ExecutionPlan *plan) {
  // Collect all label scans
  Vector *scanOps = NewVector(NodeByLabelScan*, 0);
  _locateScanOp(plan->root, scanOps);

  // Collect all filters on scanned entities
  NodeByLabelScan *scanOp;
  Vector *filterOps = NewVector(OpBase*, 0);
  Vector *filters = NewVector(FT_FilterNode*, 0);

  char *label;
  while (Vector_Pop(scanOps, &scanOp)) {
    /* Get the label string for the scan target.
     * The label will be used to retrieve the index. */
    label = (*scanOp->node)->label;
    Vector_Clear(filterOps);
    Vector_Clear(filters);
    _locateScanFilters(scanOp, filterOps);

    // No filters.
    if (Vector_Size(filterOps) == 0) continue;

    // Extract actual filter trees.
    // TODO should this be in this loop?
    Vector *filters = NewVector(FT_FilterNode*, Vector_Size(filterOps));
    for (int i = 0; i < Vector_Size(filterOps); i ++) {
      OpBase *opFilter;
      Vector_Get(filterOps, i, &opFilter);
      Filter *f = (Filter *)opFilter;
      Vector_Push(filters, f->filterTree);
    }

    /* At this point we have all the filter trees associated with the
     * scanned entity. If there are valid indices on any filter and no
     * equal or higher precedence OR filters, we can switch to an index scan. */

    IndexIter *iter = NULL;
    Index *idx = NULL;
    FT_FilterNode *ft;
    /* We'll currently use the first matching index, but apply all the filters on
     * that property. A later optimization would be to find the index with the
     * most filters, or use some heuristic for trying to select the minimal range. */
    // TODO can maybe be woven into above loop
    for (int i = 0; i < Vector_Size(filters); i ++) {
      Vector_Get(filters, i, &ft);
      /* ft will be a predicate or a tree with an OR root, which means
       * we can safely employ it if it's a const predicate and ignore it otherwise. */
      if (!IsNodeConstantPredicate(ft)) continue;

      // If we've already selected an index on a different property, continue
      if (idx && strcmp(idx->property, ft->pred.Lop.property)) continue;

      // Try to retrieve an index if one has not been selected yet
      if (!idx) {
        idx = Index_Get(ctx, graph_name, label, ft->pred.Lop.property);
        if (!idx) continue;
        iter = IndexIter_Create(idx, ft->pred.constVal.type);
      }

      // Tighten the iterator range if possible
      if (IndexIter_ApplyBound(iter, &ft->pred)) {
        // Remove filter operations that have been folded into the index scan iterator
        OpBase* filterOp;
        Vector_Get(filterOps, i, &filterOp);
        ExecutionPlan_RemoveOp(filterOp);
        OpBase_Free(filterOp);
      }
    }

    if (iter != NULL) {
      OpBase *indexOp = NewIndexScanOp(plan->graph, scanOp->g, scanOp->node, iter);
      ExecutionPlan_ReplaceOp((OpBase*)scanOp, indexOp);
    }

    // TODO remove *just* the filters we've replaced
    /*
    // Remove reduced filter operations from execution plan.
    for (int i = 0; i < Vector_Size(filterOps); i++) {
      OpBase* filterOp;
      Vector_Get(filterOps, i, &filterOp);
      ExecutionPlan_RemoveOp(filterOp);
      OpBase_Free(filterOp);
    }
    */
  }

  // Cleanup
  Vector_Free(filters);
  Vector_Free(filterOps);
  Vector_Free(scanOps);
}


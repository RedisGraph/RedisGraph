#include "utilize_indices.h"
#include "../ops/op_index_scan.h"

/* Reverse an inequality symbol so that indices can support
 * inequalities with right-hand variables. */
int _reverseOp(int op) {
    switch(op) {
        case LT:
            return GT;
        case LE:
            return GE;
        case GT:
            return LT;
        case GE:
            return LE;
        default:
            return op;
    }
}

void _locateScanFilters(NodeByLabelScan *scanOp, Vector *filterOps) {
  /* We begin with a LabelScan, and want to find predicate filters that modify
   * the active entity. */
  OpBase *current = scanOp->op.parent;
  while(current->type == OPType_FILTER) {
    Filter *filterOp = (Filter*)current;
    FT_FilterNode *filterTree = filterOp->filterTree;

    /* filterTree will either be a predicate or a tree with an OR root.
     * We'll store ops on const predicate filters, and can otherwise safely ignore them -
     * no filter tree in this sequence can invalidate another. */
    if (IsNodePredicate(filterTree)) {
      Vector_Push(filterOps, current);
    }

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
  char *label;

  // Variables to be used when comparing filters against available indices
  char *filterProp = NULL;
  SIValue constVal;
  int lhsType, rhsType;
  int op = 0;

  while (Vector_Pop(scanOps, &scanOp)) {
    IndexIter *iter = NULL;
    Index *idx = NULL;

    /* Get the label string for the scan target.
     * The label will be used to retrieve the index. */
    label = (*scanOp->node)->label;
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
      /* We'll only employ indices when we have filters of the form:
       * node.property [rel] constant or
       * constant [rel] node.property
       * If we are not comparing against a constant, then we cannot pre-define useful bounds
       * for the index iterator, which diminishes their utility. */
      lhsType = AR_EXP_GetOperandType(ft->pred.lhs);
      rhsType = AR_EXP_GetOperandType(ft->pred.rhs);
      if (lhsType == AR_EXP_VARIADIC && rhsType == AR_EXP_CONSTANT) {
        filterProp = ft->pred.lhs->operand.variadic.entity_prop;
        constVal = ft->pred.rhs->operand.constant;
        op = ft->pred.op;
      } else if (lhsType == AR_EXP_CONSTANT && rhsType == AR_EXP_VARIADIC) {
        constVal = ft->pred.lhs->operand.constant;
        filterProp = ft->pred.rhs->operand.variadic.entity_prop;
        // When the constant is on the left, reverse the relation in the inequality
        // to properly set the bounds.
        op = _reverseOp(ft->pred.op);
      }

      // If we've already selected an index on a different property, continue
      if (idx && strcmp(idx->property, filterProp)) continue;

      // Try to retrieve an index if one has not been selected yet
      if (!idx) {
        idx = Index_Get(ctx, graph_name, label, filterProp);
        if (!idx) continue;
        iter = IndexIter_Create(idx, constVal.type);
      }

      // Tighten the iterator range if possible
      if (IndexIter_ApplyBound(iter, &constVal, op)) {
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


#include "utilize_indices.h"
#include "../ops/op_index_scan.h"
#include "../../util/arr.h"

/* Reverse an inequality symbol so that indices can support
 * inequalities with right-hand variables. */
int _reverseOp(AST_Operator op) {
    switch(op) {
        case OP_LT:
            return OP_GT;
        case OP_LE:
            return OP_GE;
        case OP_GT:
            return OP_LT;
        case OP_GE:
            return OP_LE;
        default:
            return op;
    }
}

void _locateScanFilters(NodeByLabelScan *scanOp, OpBase ***filterOps) {
  /* We begin with a LabelScan, and want to find predicate filters that modify
   * the active entity. */
  OpBase *current = scanOp->op.parent;
  // TODO: Not sure if this while is necessary.
  while(current->type == OPType_FILTER) {
    Filter *filterOp = (Filter*)current;
    FT_FilterNode *filterTree = filterOp->filterTree;

    /* filterTree will either be a predicate or a tree with an OR root.
     * We'll store ops on const predicate filters, and can otherwise safely ignore them -
     * no filter tree in this sequence can invalidate another. */
    if (IsNodePredicate(filterTree)) {
      *filterOps = array_append(*filterOps, current);
    }

    // Advance to the next operation.
    current = current->parent;
  }
}

// Populate scanOps array with execution plan scan operations.
void _locateScanOp(OpBase *root, NodeByLabelScan ***scanOps) {

  // Is this a scan operation?
  if (root->type == OPType_NODE_BY_LABEL_SCAN) {
    *scanOps = array_append(*scanOps, (NodeByLabelScan*)root);
  }

  // Continue scanning.
  for (int i = 0; i < root->childCount; i++) {
    _locateScanOp(root->children[i], scanOps);
  }
}

void utilizeIndices(GraphContext *gc, ExecutionPlan *plan, AST *ast) {
  // Return immediately if the graph has no indices
  if (!GraphContext_HasIndices(gc)) return;

  // Collect all label scans
  NodeByLabelScan **scanOps = array_new(NodeByLabelScan*, 0);
  _locateScanOp(plan->root, &scanOps);

  // Collect all filters on scanned entities
  NodeByLabelScan *scanOp;
  OpBase **filterOps = array_new(OpBase*, 0);
  FT_FilterNode *ft;
  char *label;

  // Variables to be used when comparing filters against available indices
  char *filterProp = NULL;
  SIValue constVal;
  int lhsType, rhsType;
  AST_Operator op = OP_NULL;

  int scanOpCount = array_len(scanOps);
  for(int i = 0; i < scanOpCount; i++) {
  // while (Vector_Pop(scanOps, &scanOp)) {
    scanOp = scanOps[i];
    IndexIter *iter = NULL;
    Index *idx = NULL;

    /* Get the label string for the scan target.
     * The label will be used to retrieve the index. */
    label = scanOp->node->label;
    array_clear(filterOps);
    _locateScanFilters(scanOp, &filterOps);

    // No filters.
    if(array_len(filterOps) == 0) continue;

    /* At this point we have all the filter ops (and thus, filter trees) associated
     * with the scanned entity. If there are valid indices on any filter and no
     * equal or higher precedence OR filters, we can switch to an index scan.
     *
     * We'll currently use the first matching index, but apply all the filters on
     * that property. A later optimization would be to find the index with the
     * most filters, or use some heuristic for trying to select the minimal range. */

    int filterOpsCount = array_len(filterOps);
    for (int i = 0; i < filterOpsCount; i ++) {
      OpBase *opFilter = filterOps[i];
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
      } else {
        continue;
      }

      // If we've already selected an index on a different property, continue
      if (idx && strcmp(idx->attribute, filterProp)) continue;

      // Try to retrieve an index if one has not been selected yet
      if (!idx) {
        idx = GraphContext_GetIndex(gc, label, filterProp);
        if (!idx) continue;
        iter = IndexIter_Create(idx, SI_TYPE(constVal));
      }

      // Tighten the iterator range if possible
      if (IndexIter_ApplyBound(iter, &constVal, op)) {
        // Remove filter operations that have been folded into the index scan iterator
        ExecutionPlan_RemoveOp(plan, opFilter);
        OpBase_Free(opFilter);
      }
    }

    if (iter != NULL) {
      OpBase *indexOp = NewIndexScanOp(scanOp->g, scanOp->node, iter, ast);
      ExecutionPlan_ReplaceOp(plan, (OpBase*)scanOp, indexOp);
    }
  }

  // Cleanup
  array_free(filterOps);
  array_free(scanOps);
}


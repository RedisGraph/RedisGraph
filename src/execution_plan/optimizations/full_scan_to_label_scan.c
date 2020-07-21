#include "full_scan_to_label_scan.h"

static inline bool _isLabelFunc(const AR_ExpNode *exp) {
	return exp->type == AR_EXP_OP && !strcasecmp(exp->op.func_name, "labels");
}

static inline bool _isNodeOperand(const AR_ExpNode *exp) {
	return exp->type == AR_EXP_OPERAND && exp->operand.type == AR_EXP_VARIADIC &&
		   !exp->operand.variadic.entity_prop;
}

static inline bool _isMatchingLabelFunc(const AR_ExpNode *exp,
										const char *alias) {
	// We are only interested in labels() function calls.
	if(!_isLabelFunc(exp)) return false;

	// The labels() function's child should be a node operand.
	AR_ExpNode *label_func_child = exp->op.children[0];
	if(!_isNodeOperand(label_func_child)) return false;

	// If the node's alias matches the scan's alias, return true.
	return !strcmp(label_func_child->operand.variadic.entity_alias, alias);
}

static inline bool _isStringConstant(const AR_ExpNode *exp) {
	return exp->type == AR_EXP_OPERAND && exp->operand.type == AR_EXP_CONSTANT &&
		   exp->operand.constant.type == T_STRING;
}

static void _ReplaceOpsWithLabelScan(ExecutionPlan *plan, OpBase *filter,
									 AllNodeScan *scan,
									 const char *label) {
	// Update the scan's inner node with the label.
	QueryGraph_SetNodeLabel((QGNode *)scan->n, label);

	// Remove and free the redundant filter op.
	ExecutionPlan_RemoveOp(plan, filter);
	OpBase_Free(filter);

	// Replace the redundant scan op with the newly-constructed Label Scan.
	OpBase *label_scan = NewNodeByLabelScanOp(plan, scan->n);
	ExecutionPlan_ReplaceOp(plan, (OpBase *)scan, label_scan);
	OpBase_Free((OpBase *)scan);
}

static void _label_scan_from_filter(ExecutionPlan *plan, AllNodeScan *scan) {
	const char *alias = scan->n->alias;
	OpBase *parent = scan->op.parent;
	while(parent->type == OPType_FILTER) {
		OpFilter *filter = (OpFilter *)parent;
		FT_FilterNode *ft = filter->filterTree;

		// Update the parent operation for the next loop.
		parent = parent->parent;

		// Reject all filter nodes that aren't equality predicates.
		if(!IsNodePredicate(ft) || ft->pred.op != OP_EQUAL) continue;

		if(_isMatchingLabelFunc(ft->pred.lhs, alias) &&
		   _isStringConstant(ft->pred.rhs)) {
			/* If the LHS is a label functions and the RHS is a string constant,
			 * we can replace the AllNodeScan. */
			const char *label = ft->pred.lhs->operand.constant.stringval;
			// Replace the AllNodeScan and the Filter op with a label scan.
			_ReplaceOpsWithLabelScan(plan, (OpBase *)filter, scan, label);
			return;
		} else if(_isMatchingLabelFunc(ft->pred.rhs, alias) &&
				  _isStringConstant(ft->pred.lhs)) {
			/* If the RHS is a label functions and the LHS is a string constant,
			 * we can replace the AllNodeScan. */
			const char *label = ft->pred.lhs->operand.constant.stringval;
			// Replace the AllNodeScan and the Filter op with a label scan.
			_ReplaceOpsWithLabelScan(plan, (OpBase *)filter, scan, label);
			return;
		}
	}
}

void fullScantoLabelScan(ExecutionPlan *plan) {
	// Collect all AllNodeScan ops.
	OpBase **scan_ops = ExecutionPlan_CollectOps(plan->root, OPType_ALL_NODE_SCAN);

	uint scanOpCount = array_len(scan_ops);
	for(uint i = 0; i < scanOpCount; i++) {
		_label_scan_from_filter(plan, (AllNodeScan *)scan_ops[i]);
	}

	// Cleanup
	array_free(scan_ops);
}


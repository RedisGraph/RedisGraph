/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "seek_by_id.h"
#include "../../util/arr.h"
#include "../ops/op_filter.h"
#include "../ops/op_index_scan.h"
#include "../ops/op_all_node_scan.h"
#include "../ops/op_node_by_id_seek.h"
#include "../ops/op_node_by_label_scan.h"
#include "../../util/range/numeric_range.h"
#include "../../arithmetic/arithmetic_op.h"

static bool _idFilter(FT_FilterNode *f, AST_Operator *rel, EntityID *id, bool *reverse) {
	if(f->t != FT_N_PRED) return false;
	if(f->pred.op == OP_NEQUAL) return false;

	AR_OpNode *op;
	AR_OperandNode *operand;
	AR_ExpNode *lhs = f->pred.lhs;
	AR_ExpNode *rhs = f->pred.rhs;
	*rel = f->pred.op;

	/* Either ID(N) compare const
	 * OR
	 * const compare ID(N) */
	if(lhs->type == AR_EXP_OPERAND && rhs->type == AR_EXP_OP) {
		op = &rhs->op;
		operand = &lhs->operand;
		*reverse = true;
	} else if(lhs->type == AR_EXP_OP && rhs->type == AR_EXP_OPERAND) {
		op = &lhs->op;
		operand = &rhs->operand;
		*reverse = false;
	} else {
		return false;
	}

	// Make sure ID is compared to a constant.
	if(operand->type != AR_EXP_CONSTANT) return false;
	if(SI_TYPE(operand->constant) != T_INT64) return false;
	*id = SI_GET_NUMERIC(operand->constant);

	// Make sure applied function is ID.
	if(strcasecmp(op->func_name, "id")) return false;

	return true;
}

static void _reverseOp(AST_Operator *op) {
	switch(*op) {
	case OP_GE:
		*op = OP_LE;
		break;
	case OP_LE:
		*op = OP_GE;
		break;
	case OP_LT:
		*op = OP_GT;
		break;
	case OP_GT:
		*op = OP_LT;
		break;
	case OP_EQUAL:
		break;
	default:
		assert(false);
		break;
	}
}

static void _UseIdOptimization(ExecutionPlan *plan, OpBase *scan_op) {
	/* See if there's a filter of the form
	 * ID(n) op X
	 * where X is a constant and op in [EQ, GE, LE, GT, LT] */
	OpBase *parent = scan_op->parent;
	UnsignedRange *id_range = NULL;
	while(parent && parent->type == OPType_FILTER) {
		OpFilter *filter = (OpFilter *)parent;
		FT_FilterNode *f = filter->filterTree;

		AST_Operator op;
		EntityID id;
		bool reverse;
		if(_idFilter(f, &op, &id, &reverse)) {
			if(!id_range) id_range = UnsignedRange_New();
			if(reverse) op = ArithmeticOp_ReverseOp(op);
			UnsignedRange_TightenRange(id_range, op, id);

			// Free replaced operations.
			ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
			OpBase_Free((OpBase *)filter);
		}
		// Advance.
		parent = parent->parent;
	}
	if(id_range) {
		/* Don't replace label scan, but set it to have range query.
		 * Issue 818 https://github.com/RedisGraph/RedisGraph/issues/818
		 * This optimization caused a range query over the entire range of ids in the graph
		 * regardless to the label. */
		if(scan_op->type == OPType_NODE_BY_LABEL_SCAN) {
			NodeByLabelScan *label_scan = (NodeByLabelScan *) scan_op;
			NodeByLabelScanOp_SetIDRange(label_scan, id_range);
		} else {
			const QGNode *node = NULL;
			switch(scan_op->type) {
			case OPType_ALL_NODE_SCAN:
				node = ((AllNodeScan *)scan_op)->n;
				break;
			case OPType_INDEX_SCAN:
				node = ((IndexScan *)scan_op)->n;
				break;
			default:
				assert(false);
			}
			OpBase *opNodeByIdSeek = NewNodeByIdSeekOp(scan_op->plan, node, id_range);

			// Managed to reduce!
			ExecutionPlan_ReplaceOp(plan, scan_op, opNodeByIdSeek);
			OpBase_Free(scan_op);
		}
		UnsignedRange_Free(id_range);
	}
}

void seekByID(ExecutionPlan *plan) {
	assert(plan);
	OpBase **scan_ops = ExecutionPlan_LocateOps(plan->root,
												(OPType_ALL_NODE_SCAN | OPType_NODE_BY_LABEL_SCAN));

	for(int i = 0; i < array_len(scan_ops); i++) {
		_UseIdOptimization(plan, scan_ops[i]);
	}

	array_free(scan_ops);
}


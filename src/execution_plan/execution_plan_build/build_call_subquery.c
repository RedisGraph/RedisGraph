/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "../../ast/ast.h"
#include "../ops/op_join.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../execution_plan.h"
#include "../ops/op_project.h"
#include "../ops/op_argument.h"
#include "execution_plan_util.h"
#include "../ops/op_aggregate.h"
#include "execution_plan_modify.h"
#include "../ops/op_argument_list.h"
#include "../ops/op_call_subquery.h"

// returns an AST containing the body of a subquery as its body (stripped from
// the CALL {} clause)
static void _create_ast_from_call_subquery
(
	AST *subquery_ast,               // target AST to populate
	const cypher_astnode_t *clause,  // CALL {} ast-node
	const AST *orig_ast              // original AST with which to build new one
) {
	ASSERT(orig_ast != NULL);
	ASSERT(cypher_astnode_type(clause) == CYPHER_AST_CALL_SUBQUERY);

	// create an AST from the body of the subquery
	subquery_ast->root = cypher_ast_call_subquery_get_query(clause);
	subquery_ast->anot_ctx_collection = orig_ast->anot_ctx_collection;
}

// adds an empty projection as the child of parent, such that the records passed
// to parent are "filtered" to contain no bound vars
static OpBase *_add_empty_projection
(
	OpBase *parent
) {
	OpBase *empty_proj =
		NewProjectOp(parent->plan, array_new(AR_ExpNode *, 0));

	OPType type = OpBase_Type(parent);
	if(type == OPType_CALLSUBQUERY || type == OPType_FOREACH) {
		ExecutionPlan_AddOpInd(parent, empty_proj, 0);
	} else {
		ExecutionPlan_AddOp(parent, empty_proj);
	}

	return empty_proj;
}

// returns true if op is effectively a deepest op (i.e., no lhs)
static inline bool _is_deepest_call_foreach
(
	OpBase *op  // op to check
) {
	OPType type = OpBase_Type(op);
	return (type == OPType_CALLSUBQUERY || type == OPType_FOREACH) &&
			op->childCount == 1;
}

// finds the deepest operation starting from root, and appends it to deepest_ops
// if a call {} op with one child is found, it is appended to deepest_ops
static void _get_deepest
(
	OpBase *root,          // root op from which to look for the deepest op
	OpBase ***deepest_ops  // target array to which the deepest op is appended
) {
	OpBase *deepest = root;

	// check root
	if(_is_deepest_call_foreach(deepest)) {
		array_append(*deepest_ops, deepest);
		return;
	}

	// traverse children
	while(OpBase_ChildCount(deepest) > 0) {
		deepest = deepest->children[0];
		// in case of a CallSubquery op with no lhs, we want to stop
		// here, as the added op should be its first child (instead of
		// the current child, which will be moved to be the second)
		// Example:
		// "CALL {CALL {RETURN 1 AS one} RETURN one} RETURN one"
		OPType type = OpBase_Type(deepest);
		if(_is_deepest_call_foreach(deepest)){
			array_append(*deepest_ops, deepest);
			return;
		}
	}

	array_append(*deepest_ops, deepest);
}

// looks for a Join operation at root or root->children[0] and returns it, or
// NULL if not found
static OpBase *_get_join
(
	OpBase *root  // root op from which to look for the Join op
) {
	// check if there is a Join operation (from UNION or UNION ALL)
	OpBase *join_op = NULL;
	if(root->type == OPType_JOIN) {
		join_op = root;
	} else if(root->childCount > 0 && root->children[0]->type == OPType_JOIN) {
		join_op = root->children[0];
	}

	return join_op;
}

// returns an array with the deepest ops of an execution plan
// note: it's that caller's responsibility to free the array
static OpBase **_find_feeding_points
(
	const ExecutionPlan *plan
) {
	ASSERT(plan != NULL);

	// check root and its first child for a Join op. The join op's placement
	// may vary depending on whether there is a `UNION` or `UNION ALL` clause
	OpBase *join = _get_join(plan->root);

	// if not found, check for a Join op in the first child of the first child
	if(join == NULL) {
		if(OpBase_ChildCount(plan->root) > 0) {
			OpBase *child = OpBase_GetChild(plan->root, 0);
			if(OpBase_ChildCount(child) > 0) {
				OpBase *grandchild = OpBase_GetChild(child, 0);
				if(OpBase_Type(grandchild) == OPType_JOIN)  {
					join = grandchild;
				}
			}
		}
	}

	// get the deepest op(s)
	uint n_branches = 1;
	OpBase **deepest_ops = array_new(OpBase *, n_branches);

	if(join != NULL) {
		n_branches = OpBase_ChildCount(join);
		for(uint i = 0; i < n_branches; i++) {
			OpBase *branch = OpBase_GetChild(join, i);
			_get_deepest(branch, &deepest_ops);
		}
	} else {
		_get_deepest(plan->root, &deepest_ops);
	}

	ASSERT(array_len(deepest_ops) == n_branches);
	return deepest_ops;
}

// binds op to plan
static void _bind_returning_op
(
	OpBase *op,          // op to bind
	const ExecutionPlan *plan  // plan to bind op to
) {
	OPType type = OpBase_Type(op);
	if(type == OPType_PROJECT) {
		ProjectBindToPlan(op, plan);
	} else if(type == OPType_AGGREGATE) {
		AggregateBindToPlan(op, plan);
	} else if(type != OPType_JOIN){
		OpBase_UpdatePlan(op, plan);
	}
}

// binds the returning ops (effectively, all ops between the first
// Project\Aggregate and CallSubquery in every branch other than the Join op,
// inclusive) in embedded_plan to plan
static void _bind_returning_ops_to_plan
(
	const ExecutionPlan *embedded_plan,  // embedded plan
	const ExecutionPlan *plan            // plan to migrate ops to
) {
	OPType return_types[] = {OPType_PROJECT, OPType_AGGREGATE};

	// check if there is a Join operation (from UNION or UNION ALL)
	OpBase *root = embedded_plan->root;
	OpBase *join_op = _get_join(root);

	// if there is a Union operation, we need to look at all branches
	if(join_op == NULL) {
		// only one returning projection/aggregation
		OpBase *returning_op =
			ExecutionPlan_LocateOpMatchingType(root, return_types, 2);
		while(returning_op != NULL) {
			_bind_returning_op(returning_op, plan);
			returning_op = returning_op->parent;
		}
	} else {
		// multiple returning projections/aggregations
		for(uint i = 0; i < join_op->childCount; i++) {
			OpBase *child = join_op->children[i];
			OpBase *returning_op =
				ExecutionPlan_LocateOpMatchingType(child, return_types, 2);
			while(returning_op != NULL) {
				_bind_returning_op(returning_op, plan);
				returning_op = returning_op->parent;
			}
		}
	}
}

// add empty projections to the branches which do not contain an importing WITH
// clause, in order to 'reset' the bound-vars environment
static void _add_empty_projections
(
	AST *subquery_ast,               // subquery AST
	const cypher_astnode_t *clause,  // call subquery clause
	OpBase **deepest_ops             // deepest op in each of the UNION branches
) {
	uint n_branches = array_len(deepest_ops);
	for(uint i = 0; i < n_branches; i++) {
		if(OpBase_Type(deepest_ops[i]) != OPType_PROJECT) {
			deepest_ops[i] = _add_empty_projection(deepest_ops[i]);
		}
	}
}

// construct the execution-plan corresponding to a call {} clause
void buildCallSubqueryPlan
(
	ExecutionPlan *plan,            // execution plan to add plan to
	const cypher_astnode_t *clause  // call subquery clause
) {
	//--------------------------------------------------------------------------
	// build an AST from the subquery
	//--------------------------------------------------------------------------

	// save the original AST
	AST *orig_ast = QueryCtx_GetAST();

	// create an AST from the body of the subquery
	AST subquery_ast = {0};
	_create_ast_from_call_subquery(&subquery_ast, clause, orig_ast);

	//--------------------------------------------------------------------------
	// build the embedded execution plan corresponding to the subquery
	//--------------------------------------------------------------------------

	QueryCtx_SetAST(&subquery_ast);
	ExecutionPlan *embedded_plan = ExecutionPlan_FromTLS_AST();
	QueryCtx_SetAST(orig_ast);

	// characterize whether the query is eager or not
	OPType eager_types[] = {OPType_CREATE, OPType_UPDATE, OPType_FOREACH,
					  OPType_MERGE, OPType_SORT, OPType_AGGREGATE};

	bool is_eager =
	  ExecutionPlan_LocateOpMatchingType(embedded_plan->root, eager_types, 6)
		!= NULL;

	bool is_returning = (OpBase_Type(embedded_plan->root) == OPType_RESULTS);

	// find the deepest ops, to which we will add the projections and feeders
	OpBase **deepest_ops = _find_feeding_points(embedded_plan);

	// if no variables are imported, add an 'empty' projection so that the
	// records within the subquery will be cleared from the outer-context
	_add_empty_projections(&subquery_ast, clause, deepest_ops);

	//--------------------------------------------------------------------------
	// Bind returning projection(s)\aggregation(s) to the outer plan
	//--------------------------------------------------------------------------

	if(is_returning) {
		// remove the Results op from the execution-plan
		OpBase *results_op = embedded_plan->root;
		ASSERT(OpBase_Type(results_op) == OPType_RESULTS);
		ExecutionPlan_RemoveOp(embedded_plan, embedded_plan->root);
		OpBase_Free(results_op);

		_bind_returning_ops_to_plan(embedded_plan, plan);
	}

	// set Join op to not modify the ResultSet mapping
	OpBase *join_op = _get_join(embedded_plan->root);
	if(join_op != NULL) {
		JoinSetUpdateColumnMap(join_op, false);
	}

	// -------------------------------------------------------------------------
	// create an ArgumentList\Argument op according to the eagerness of the op,
	// and plant it as the deepest child of the embedded plan
	// -------------------------------------------------------------------------
	uint n_deepest_ops = array_len(deepest_ops);
	if(is_eager) {
		for(uint i = 0; i < n_deepest_ops; i++) {
			OpBase *argument_list = NewArgumentListOp(plan);
			ExecutionPlan_AddOp(deepest_ops[i], argument_list);
		}
	} else {
		for(uint i = 0; i < n_deepest_ops; i++) {
			OpBase *argument = NewArgumentOp(plan, NULL);
			ExecutionPlan_AddOp(deepest_ops[i], argument);
		}
	}

	array_free(deepest_ops);

	// -------------------------------------------------------------------------
	// create a CallSubquery op, set it to be the root of the main plan
	// -------------------------------------------------------------------------
	OpBase *call_op = NewCallSubqueryOp(plan, is_eager, is_returning);
	ExecutionPlan_UpdateRoot(plan, call_op);

	// bind the embedded plan to be the rhs branch of the Call-Subquery op
	ExecutionPlan_AddOp(call_op, embedded_plan->root);
}

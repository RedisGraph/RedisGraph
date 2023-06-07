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
#include "../ops/op_aggregate.h"
#include "execution_plan_modify.h"
#include "../ops/op_argument_list.h"
#include "../ops/op_call_subquery.h"

// returns an AST containing the body of a subquery as its body (stripped from
// the CALL {} clause)
static AST *_CreateASTFromCallSubquery
(
	const cypher_astnode_t *clause,  // CALL {} ast-node
	const AST *orig_ast              // original AST with which to build new one
) {
	ASSERT(orig_ast != NULL);
	ASSERT(cypher_astnode_type(clause) == CYPHER_AST_CALL_SUBQUERY);

	// create an AST from the body of the subquery
	uint *ref_count = rm_malloc(sizeof(uint));
	*ref_count = 1;

	AST *subquery_ast = rm_calloc(1, sizeof(AST));
	subquery_ast->free_root           = true;
	subquery_ast->ref_count           = ref_count;
	// TODO: Make sure you need this.
	subquery_ast->anot_ctx_collection = orig_ast->anot_ctx_collection;

	// build the query, to be the root of the temporary AST
	uint clause_count = cypher_astnode_nchildren(clause);
	cypher_astnode_t *clauses[clause_count];

	// explicitly collect all child nodes from the clause.
	for(uint i = 0; i < clause_count; i ++) {
		clauses[i] = (cypher_astnode_t *)cypher_astnode_get_child(clause, i);
	}
	struct cypher_input_range range = {0};

	cypher_astnode_t *query = cypher_ast_query(NULL,
								0,
								clauses,
								clause_count,
								clauses,
								clause_count,
								range);
	subquery_ast->root = query;

	return subquery_ast;
}

// adds an empty projection as the child of parent, such that the records passed
// to parent are "filtered" to contain no bound vars
static OpBase *_AddEmptyProjection
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
static bool _is_deepest_call_foreach
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
static OpBase *_getJoin
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
static OpBase **_FindDeepestOps
(
	const ExecutionPlan *plan
) {
	ASSERT(plan != NULL);

	OpBase *deepest = plan->root;
	OpBase **deepest_ops = array_new(OpBase *, 1);

	// check root and its first child for a Join op
	OpBase *join = _getJoin(deepest);

	// if didn't find, check for a Join op in the first child of the first child
	if(join == NULL) {
		join = OpBase_ChildCount(deepest) > 0 ?
			OpBase_ChildCount(OpBase_GetChild(deepest, 0)) > 0 ?
				OpBase_Type(OpBase_GetChild(OpBase_GetChild(deepest, 0), 0)) ==
				OPType_JOIN ?
					OpBase_GetChild(OpBase_GetChild(deepest, 0), 0) :
					NULL :
				NULL :
			NULL;
	}

	if(join != NULL) {
		uint n_branches = OpBase_ChildCount(join);
		for(uint i = 0; i < n_branches; i++) {
			deepest = OpBase_GetChild(join, i);
			_get_deepest(deepest, &deepest_ops);
		}
	} else {
		_get_deepest(deepest, &deepest_ops);
	}

	return deepest_ops;
}

// binds op to plan
static void _bind_returning_op
(
	OpBase *op,          // op to bind
	ExecutionPlan *plan  // plan to bind op to
) {
	OPType type = OpBase_Type(op);
	if(type == OPType_PROJECT) {
		ProjectBindToPlan(op, plan);
	} else if(type == OPType_AGGREGATE) {
		AggregateBindToPlan(op, plan);
	} else {
		OpBase_UpdatePlan(op, plan);
	}
}

// binds the returning ops (effectively, all ops between the first
// Project\Aggregate and CallSubquery in every branch, inclusive) in
// embedded_plan to plan
static void _BindReturningOpsToPlan
(
	ExecutionPlan *embedded_plan,  // plan containing the returning ops
	ExecutionPlan *plan            // plan to bind the returning ops to
) {
	OPType return_types[] = {OPType_PROJECT, OPType_AGGREGATE};

	// check if there is a Join operation (from UNION or UNION ALL)
	OpBase *root = embedded_plan->root;
	OpBase *join_op = _getJoin(root);

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
				OPType type = OpBase_Type(returning_op);

				_bind_returning_op(returning_op, plan);
				returning_op = returning_op->parent;
			}
		}
	}
}

// add empty projections in the branches which do not contain an importing WITH
// clause, in order to 'reset' the bound-vars environment
static void _add_empty_projections
(
	AST *subquery_ast,               // subquery AST
	const cypher_astnode_t *clause,  // call subquery clause
	OpBase **deepest_ops             // deepest op in each of the UNION branches
) {
	uint clause_count = cypher_ast_call_subquery_nclauses(clause);
	uint *union_indices = AST_GetClauseIndices(subquery_ast,
		CYPHER_AST_UNION);
	array_append(union_indices, clause_count);
	uint n_union_branches = array_len(union_indices);
	uint first_ind = 0;
	const cypher_astnode_t *first_clause;
	OpBase **deepest;

	for(uint i = 0; i < n_union_branches; i++) {
		// find first clause in the relevant branch
		first_clause = cypher_ast_call_subquery_get_clause(clause, first_ind);
		if(cypher_astnode_type(first_clause) != CYPHER_AST_WITH) {
			deepest = array_elem(deepest_ops, i);
			*deepest = _AddEmptyProjection(*deepest);
		}

		first_ind = union_indices[i] + 1;
	}

	array_free(union_indices);
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
	AST *subquery_ast = _CreateASTFromCallSubquery(clause, orig_ast);

	//--------------------------------------------------------------------------
	// build the embedded execution plan corresponding to the subquery
	//--------------------------------------------------------------------------

	QueryCtx_SetAST(subquery_ast);
	ExecutionPlan *embedded_plan = NewExecutionPlan();
	QueryCtx_SetAST(orig_ast);

	// find the deepest ops, to which we will add the projections and feeders
	OpBase **deepest_ops = _FindDeepestOps(embedded_plan);

	// if no variables are imported, add an 'empty' projection so that the
	// records within the subquery will not carry unnecessary entries
	_add_empty_projections(subquery_ast, clause, deepest_ops);
	AST_Free(subquery_ast);

	// characterize whether the query is eager or not
	OPType eager_types[] = {OPType_CREATE, OPType_UPDATE, OPType_FOREACH,
					  OPType_MERGE, OPType_SORT, OPType_AGGREGATE};

	bool is_eager =
	  ExecutionPlan_LocateOpMatchingType(embedded_plan->root, eager_types, 6)
		!= NULL;

	bool is_returning = OpBase_Type(embedded_plan->root) == OPType_RESULTS;

	// -------------------------------------------------------------------------
	// Get rid of the Results op if it exists.
	// Bind returning projection\aggregation to the outer-scope execution-plan
	// -------------------------------------------------------------------------
	if(is_returning) {
		// remove the Results op from the execution-plan
		OpBase *results_op = embedded_plan->root;
		ExecutionPlan_RemoveOp(embedded_plan, embedded_plan->root);
		OpBase_Free(results_op);

		_BindReturningOpsToPlan(embedded_plan, plan);
	}

	// set Join op to not modify the ResultSet mapping
	OpBase *join_op = _getJoin(embedded_plan->root);
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

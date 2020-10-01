#include "execution_plan_construct.h"
#include "execution_plan_modify.h"
#include "../execution_plan.h"
#include "../ops/ops.h"
#include "../../query_ctx.h"
#include "../../ast/ast_build_ar_exp.h"
#include "../../ast/ast_build_filter_tree.h"

// Given a WITH/RETURN * clause, generate the array of expressions to populate.
static AR_ExpNode **_PopulateProjectAll(const cypher_astnode_t *clause) {
	// Retrieve the relevant aliases from the AST.
	const char **aliases = AST_GetProjectAll(clause);
	uint count = array_len(aliases);

	AR_ExpNode **project_exps = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		// Build an expression for each alias.
		AR_ExpNode *exp = AR_EXP_NewVariableOperandNode(aliases[i], NULL);
		exp->resolved_name = aliases[i];
		project_exps = array_append(project_exps, exp);
	}

	return project_exps;
}

// Handle ORDER entities
static AR_ExpNode **_BuildOrderExpressions(AR_ExpNode **projections,
										   const cypher_astnode_t *order_clause) {
	AST *ast = QueryCtx_GetAST();
	uint projection_count = array_len(projections);
	uint count = cypher_ast_order_by_nitems(order_clause);
	AR_ExpNode **order_exps = array_new(AR_ExpNode *, count);

	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *item = cypher_ast_order_by_get_item(order_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_sort_item_get_expression(item);
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);
		// Build a string representation of the ORDER identity.
		char *constructed_name = AR_EXP_BuildResolvedName(exp);
		// If the constructed name refers to a QueryGraph entity, use its canonical name.
		char *canonical_name = raxFind(ast->canonical_entity_names, (unsigned char *)constructed_name,
									   strlen(constructed_name));
		if(canonical_name == raxNotFound) {
			// Otherwise, introduce a new canonical name.
			canonical_name = constructed_name;
			raxInsert(ast->canonical_entity_names, (unsigned char *)constructed_name, strlen(constructed_name),
					  constructed_name, NULL);
		} else {
			rm_free(constructed_name);
		}

		exp->resolved_name = canonical_name;
		AST_AttachName(ast, item, exp->resolved_name);

		order_exps = array_append(order_exps, exp);
	}

	return order_exps;
}

// Handle RETURN entities
// (This function is not static because it is relied upon by unit tests)
AR_ExpNode **_BuildReturnExpressions(const cypher_astnode_t *ret_clause) {
	// Query is of type "RETURN *"
	if(cypher_ast_return_has_include_existing(ret_clause)) return _PopulateProjectAll(ret_clause);

	uint count = cypher_ast_return_nprojections(ret_clause);
	AR_ExpNode **return_expressions = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(ret_clause, i);
		// The AST expression can be an identifier, function call, or constant
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this return entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);

		// Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
		// the entity.prop combination ("a.val"), or the function call ("MAX(a.val)")
		const char *identifier = NULL;
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node) {
			// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
			identifier = cypher_ast_identifier_get_name(alias_node);
		} else {
			// This expression did not have an alias, so it must be an identifier
			assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
			// Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
			identifier = cypher_ast_identifier_get_name(ast_exp);
		}

		exp->resolved_name = identifier;
		return_expressions = array_append(return_expressions, exp);
	}

	return return_expressions;
}

static AR_ExpNode **_BuildWithExpressions(const cypher_astnode_t *with_clause) {
	// Clause is of type "WITH *"
	if(cypher_ast_with_has_include_existing(with_clause)) return _PopulateProjectAll(with_clause);

	uint count = cypher_ast_with_nprojections(with_clause);
	AR_ExpNode **with_expressions = array_new(AR_ExpNode *, count);
	for(uint i = 0; i < count; i++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, i);
		const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

		// Construction an AR_ExpNode to represent this entity.
		AR_ExpNode *exp = AR_EXP_FromExpression(ast_exp);

		// Find the resolved name of the entity - its alias, its identifier if referring to a full entity,
		// the entity.prop combination ("a.val"), or the function call ("MAX(a.val)").
		// The WITH clause requires that the resolved name be an alias or identifier.
		const char *identifier = NULL;
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
		if(alias_node) {
			// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
			/// TODO should issue syntax failure in the latter 2 cases
			identifier = cypher_ast_identifier_get_name(alias_node);
		} else {
			// This expression did not have an alias, so it must be an identifier
			const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);
			assert(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
			// Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
			identifier = cypher_ast_identifier_get_name(ast_exp);
		}

		exp->resolved_name = identifier;

		with_expressions = array_append(with_expressions, exp);
	}

	return with_expressions;

}


// Merge all order expressions into the projections array without duplicates,
static void _combine_projection_arrays(AR_ExpNode ***exps_ptr, AR_ExpNode **order_exps) {
	rax *projection_names = raxNew();
	AR_ExpNode **project_exps = *exps_ptr;
	uint order_count = array_len(order_exps);
	uint project_count = array_len(project_exps);

	// Add all WITH/RETURN projection names to rax.
	for(uint i = 0; i < project_count; i ++) {
		const char *name = project_exps[i]->resolved_name;
		raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
	}

	// Merge non-duplicate order expressions into projection array.
	for(uint i = 0; i < order_count; i ++) {
		const char *name = order_exps[i]->resolved_name;
		int new_name = raxTryInsert(projection_names, (unsigned char *)name, strlen(name), NULL, NULL);
		// If it is a new projection, add a clone to the array.
		if(new_name) project_exps = array_append(project_exps, AR_EXP_Clone(order_exps[i]));
	}

	raxFree(projection_names);
	*exps_ptr = project_exps;
}

// Build an aggregate or project operation and any required modifying operations.
// This logic applies for both WITH and RETURN projections.
static inline void _buildProjectionOps(ExecutionPlan *plan, AR_ExpNode **projections,
									   AR_ExpNode **order_exps, int *sort_directions, bool aggregate, bool distinct) {

	// Merge order expressions into the projections array.
	if(order_exps) _combine_projection_arrays(&projections, order_exps);

	// Our fundamental operation will be a projection or aggregation.
	OpBase *op;
	if(aggregate) {
		// An aggregate op's caching policy depends on whether its results will be sorted.
		bool sorting_after_aggregation = (order_exps != NULL);
		op = NewAggregateOp(plan, projections, sorting_after_aggregation);
	} else {
		op = NewProjectOp(plan, projections);
	}
	ExecutionPlan_UpdateRoot(plan, op);

	/* Add modifier operations in order such that the final execution plan will follow the sequence:
	 * Limit -> Skip -> Sort -> Distinct -> Project/Aggregate */
	if(distinct) {
		OpBase *op = NewDistinctOp(plan);
		ExecutionPlan_UpdateRoot(plan, op);
	}

	AST *ast = QueryCtx_GetAST();

	if(sort_directions) {
		// The sort operation will obey a specified limit, but must account for skipped records
		OpBase *op = NewSortOp(plan, order_exps, sort_directions);
		ExecutionPlan_UpdateRoot(plan, op);
	}

	if(AST_GetSkipExpr(ast)) {
		OpBase *op = NewSkipOp(plan);
		ExecutionPlan_UpdateRoot(plan, op);
	}

	if(AST_GetLimitExpr(ast)) {
		OpBase *op = NewLimitOp(plan);
		ExecutionPlan_UpdateRoot(plan, op);
	}
}

// The RETURN logic is identical to WITH-culminating segments,
// though a different set of API endpoints must be used for each.
void buildReturnOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **projections = _BuildReturnExpressions(clause);

	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_return_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_return_get_skip(clause);

	int *sort_directions = NULL;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_return_get_order_by(clause);
	if(order_clause) {
		AST_PrepareSortOp(order_clause, &sort_directions);
		order_exps = _BuildOrderExpressions(projections, order_clause);
	}
	_buildProjectionOps(plan, projections, order_exps, sort_directions, aggregate, distinct);
}

void buildWithOps(ExecutionPlan *plan, const cypher_astnode_t *clause) {
	AR_ExpNode **projections = _BuildWithExpressions(clause);
	bool aggregate = AST_ClauseContainsAggregation(clause);
	bool distinct = cypher_ast_with_is_distinct(clause);

	const cypher_astnode_t *skip_clause = cypher_ast_with_get_skip(clause);

	int *sort_directions = NULL;
	AR_ExpNode **order_exps = NULL;
	const cypher_astnode_t *order_clause = cypher_ast_with_get_order_by(clause);
	if(order_clause) {
		AST_PrepareSortOp(order_clause, &sort_directions);
		order_exps = _BuildOrderExpressions(projections, order_clause);
	}
	_buildProjectionOps(plan, projections, order_exps, sort_directions, aggregate, distinct);
}


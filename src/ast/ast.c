/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "ast.h"
#include <pthread.h>

#include "../RG.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/qsort.h"
#include "../procedures/procedure.h"
#include "../arithmetic/arithmetic_expression.h"
#include "../arithmetic/arithmetic_expression_construct.h"

// TODO duplicated logic, find shared place for it
static inline void _prepareIterateAll(rax *map, raxIterator *iter) {
	raxStart(iter, map);
	raxSeek(iter, "^", NULL, 0);
}

// Note each function call within given expression
// Example: given the expression: "abs(max(min(a), abs(k)))"
// referred_funcs will include: "abs", "max" and "min".
static void _consume_function_call_expression(const cypher_astnode_t *node,
											  rax *referred_funcs) {
	cypher_astnode_type_t type = cypher_astnode_type(node);

	if(type == CYPHER_AST_APPLY_OPERATOR ||
	   type == CYPHER_AST_APPLY_ALL_OPERATOR) {
		// Expression is an Apply or Apply All operator.
		bool apply_all = (type == CYPHER_AST_APPLY_ALL_OPERATOR);

		// Retrieve the function name and add to rax.
		const cypher_astnode_t *func = (!apply_all) ?
			cypher_ast_apply_operator_get_func_name(node) :
			cypher_ast_apply_all_operator_get_func_name(node);

		const char *func_name = cypher_ast_function_name_get_value(func);
		raxInsert(referred_funcs, (unsigned char *)func_name, strlen(func_name),
				NULL, NULL);

		if(apply_all) return;  // Apply All operators have no arguments.
	}

	uint child_count = cypher_astnode_nchildren(node);
	for(int i = 0; i < child_count; i++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(node, i);
		_consume_function_call_expression(child, referred_funcs);
	}
}

/* This function returns the actual root of the query.
 * As cypher_parse_result_t can have multiple roots such as comments, only a root with type
 * CYPHER_AST_STATEMENT is considered as the actual root. Comment roots are ignored. */
static const cypher_astnode_t *_AST_parse_result_root(const cypher_parse_result_t *parse_result) {
	uint nroots = cypher_parse_result_nroots(parse_result);
	for(uint i = 0; i < nroots; i++) {
		const cypher_astnode_t *root = cypher_parse_result_get_root(parse_result, i);
		cypher_astnode_type_t root_type = cypher_astnode_type(root);
		if(root_type != CYPHER_AST_STATEMENT) {
			continue;
		} else {
			return root;
		}
	}
	ASSERT("_AST_parse_result_root: Parse result should have a valid root" && false);
	return NULL;
}

/* This method extracts the query given parameters values, convert them into
 * constant arithmetic expressions and store them in a map of <name, value>
 * in the query context. */
static void _AST_Extract_Params(const cypher_parse_result_t *parse_result) {
	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = _AST_parse_result_root(parse_result);
	uint noptions = cypher_ast_statement_noptions(statement);
	if(noptions == 0) return;
	rax *params = QueryCtx_GetParams();
	for(uint i = 0; i < noptions; i++) {
		const cypher_astnode_t *option = cypher_ast_statement_get_option(statement, i);
		uint nparams = cypher_ast_cypher_option_nparams(option);
		for(uint j = 0; j < nparams; j++) {
			const cypher_astnode_t *param = cypher_ast_cypher_option_get_param(option, j);
			const char *paramName = cypher_ast_string_get_value(cypher_ast_cypher_option_param_get_name(param));
			const cypher_astnode_t *paramValue = cypher_ast_cypher_option_param_get_value(param);
			AR_ExpNode *exp = AR_EXP_FromASTNode(paramValue);
			raxInsert(params, (unsigned char *) paramName, strlen(paramName), (void *)exp, NULL);
		}
	}
}

static void AST_IncreaseRefCount(AST *ast) {
	ASSERT(ast);
	__atomic_fetch_add(ast->ref_count, 1, __ATOMIC_RELAXED);
}

static int AST_DecRefCount(AST *ast) {
	ASSERT(ast);
	return __atomic_sub_fetch(ast->ref_count, 1, __ATOMIC_RELAXED);
}

bool AST_ReadOnly(const cypher_astnode_t *root) {
	// Check for empty query
	if(root == NULL) return true;
	cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type == CYPHER_AST_CREATE                      ||
	   type == CYPHER_AST_MERGE                  ||
	   type == CYPHER_AST_DELETE                 ||
	   type == CYPHER_AST_SET                    ||
	   type == CYPHER_AST_CREATE_NODE_PROPS_INDEX ||
	   type == CYPHER_AST_DROP_NODE_PROPS_INDEX) {
		return false;
	}
	// In case of procedure call which modifies the graph/indices.
	if(type == CYPHER_AST_CALL) {
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(root));
		ProcedureCtx *proc = Proc_Get(proc_name);
		bool read_only = Procedure_IsReadOnly(proc);
		Proc_Free(proc);
		if(!read_only) return false;
	}
	uint num_children = cypher_astnode_nchildren(root);
	for(uint i = 0; i < num_children; i ++) {
		const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
		if(!AST_ReadOnly(child)) return false;
	}
	return true;
}

inline bool AST_ContainsClause(const AST *ast, cypher_astnode_type_t clause) {
	return AST_GetClause(ast, clause, NULL) != NULL;
}

// Checks to see if an AST tree contains specified node type.
bool AST_TreeContainsType(const cypher_astnode_t *root, cypher_astnode_type_t search_type) {
	cypher_astnode_type_t type = cypher_astnode_type(root);
	if(type == search_type) return true;
	uint childCount = cypher_astnode_nchildren(root);
	for(uint i = 0; i < childCount; i++) {
		if(AST_TreeContainsType(cypher_astnode_get_child(root, i), search_type)) return true;
	}
	return false;
}

// Recursively collect the names of all function calls beneath a node
void AST_ReferredFunctions(const cypher_astnode_t *root, rax *referred_funcs) {
	cypher_astnode_type_t root_type = cypher_astnode_type(root);
	if(root_type == CYPHER_AST_APPLY_OPERATOR || root_type == CYPHER_AST_APPLY_ALL_OPERATOR) {
		_consume_function_call_expression(root, referred_funcs);
	} else {
		uint child_count = cypher_astnode_nchildren(root);
		for(int i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(root, i);
			AST_ReferredFunctions(child, referred_funcs);
		}
	}
}

// Retrieve the first instance of the specified clause in the AST segment, if any.
const cypher_astnode_t *AST_GetClause(const AST *ast,
		cypher_astnode_type_t clause_type, uint *clause_idx) {
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *child = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(child) == clause_type) {
			if(clause_idx) *clause_idx = i;
			return child;
		}
	}

	return NULL;
}

const cypher_astnode_t *AST_GetClauseByIdx(const AST *ast, uint i) {
	ASSERT(ast != NULL);
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	ASSERT(i < clause_count);

	const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, i);

	return clause;
}

uint *AST_GetClauseIndices(const AST *ast, cypher_astnode_type_t clause_type) {
	uint *clause_indices = array_new(uint, 1);
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		if(cypher_astnode_type(cypher_ast_query_get_clause(ast->root, i)) == clause_type) {
			clause_indices = array_append(clause_indices, i);
		}
	}
	return clause_indices;
}

uint AST_GetClauseCount(const AST *ast, cypher_astnode_type_t clause_type) {
	uint clause_count = cypher_ast_query_nclauses(ast->root);
	uint num_found = 0;
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *child = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(child) == clause_type) num_found ++;
	}
	return num_found;
}

/* Collect references to all clauses of the specified type in the query. Since clauses
 * cannot be nested, we only need to check the immediate children of the query node. */
const cypher_astnode_t **AST_GetClauses(const AST *ast, cypher_astnode_type_t type) {
	const cypher_astnode_t **clauses = NULL;

	uint clause_count = cypher_ast_query_nclauses(ast->root);
	for(uint i = 0; i < clause_count; i ++) {
		const cypher_astnode_t *child = cypher_ast_query_get_clause(ast->root, i);
		if(cypher_astnode_type(child) != type) continue;

		if(clauses == NULL) clauses = array_new(const cypher_astnode_t *, 1);
		clauses = array_append(clauses, child);
	}

	return clauses;
}

static void _AST_GetTypedNodes(const cypher_astnode_t  ***nodes, const cypher_astnode_t *root,
							   cypher_astnode_type_t type) {
	if(cypher_astnode_type(root) == type) *nodes = array_append(*nodes, root);
	uint nchildren = cypher_astnode_nchildren(root);
	for(uint i = 0; i < nchildren; i ++) {
		_AST_GetTypedNodes(nodes, cypher_astnode_get_child(root, i), type);
	}
}

const cypher_astnode_t **AST_GetTypedNodes(const cypher_astnode_t *root,
										   cypher_astnode_type_t type) {
	const cypher_astnode_t **nodes = array_new(const cypher_astnode_t *, 0);
	_AST_GetTypedNodes(&nodes, root, type);
	return nodes;
}

void AST_CollectAliases(const char ***aliases, const cypher_astnode_t *entity) {
	if(entity == NULL) return;

	const  cypher_astnode_t **identifier_nodes =  AST_GetTypedNodes(entity, CYPHER_AST_IDENTIFIER);
	uint nodes_count = array_len(identifier_nodes);
	for(uint i = 0 ; i < nodes_count; i ++) {
		const char *identifier = cypher_ast_identifier_get_name(identifier_nodes[i]);
		*aliases = array_append(*aliases, identifier);
	}

	array_free(identifier_nodes);
}

AST *AST_Build(cypher_parse_result_t *parse_result) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->ref_count = rm_malloc(sizeof(uint));
	ast->free_root = false;
	ast->params_parse_result = NULL;
	ast->referenced_entities = NULL;
	ast->parse_result = parse_result;
	ast->canonical_entity_names = raxNew();
	ast->anot_ctx_collection = AST_AnnotationCtxCollection_New();

	*(ast->ref_count) = 1;
	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = _AST_parse_result_root(parse_result);
	// We are parsing with the CYPHER_PARSE_ONLY_STATEMENTS flag,
	// and double-checking this in AST validations
	ASSERT(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);
	ast->root = cypher_ast_statement_get_body(statement);

	// Empty queries should be captured by AST validations
	ASSERT(ast->root);

	// Set thread-local AST.
	QueryCtx_SetAST(ast);

	// Augment the AST with annotations for naming entities and populating WITH/RETURN * projections.
	AST_Enrich(ast);

	return ast;
}

AST *AST_NewSegment(AST *master_ast, uint start_offset, uint end_offset) {
	AST *ast = rm_malloc(sizeof(AST));
	ast->anot_ctx_collection = master_ast->anot_ctx_collection;
	ast->canonical_entity_names = master_ast->canonical_entity_names;
	ast->free_root = true;
	ast->ref_count = rm_malloc(sizeof(uint));
	ast->parse_result = NULL;
	ast->params_parse_result = NULL;
	uint n = end_offset - start_offset;

	*(ast->ref_count) = 1;
	const cypher_astnode_t *clauses[n];
	for(uint i = 0; i < n; i ++) {
		clauses[i] = cypher_ast_query_get_clause(master_ast->root, i + start_offset);
	}
	struct cypher_input_range range = {};
	ast->root = cypher_ast_query(NULL, 0, (cypher_astnode_t *const *)clauses, n,
								 (cypher_astnode_t **)clauses, n, range);

	// TODO This overwrites the previously-held AST pointer, which could lead to inconsistencies
	// in the future if we expect the variable to hold a different AST.
	QueryCtx_SetAST(ast);

	// If the segments are split, the next clause is either RETURN or WITH,
	// and its references should be included in this segment's map.
	const cypher_astnode_t *project_clause = NULL;
	uint clause_count = cypher_ast_query_nclauses(master_ast->root);
	if(end_offset == clause_count) end_offset = clause_count - 1;

	project_clause = cypher_ast_query_get_clause(master_ast->root, end_offset);
	/* Last clause is not necessarily a projection clause,
	 * [MATCH (a) RETURN a UNION] MATCH (a) RETURN a
	 * In this case project_clause = UNION, which is not a projection clause. */
	cypher_astnode_type_t project_type = cypher_astnode_type(project_clause);
	if(project_type != CYPHER_AST_WITH && project_type != CYPHER_AST_RETURN) project_clause = NULL;

	// Build the map of referenced entities in this AST segment.
	AST_BuildReferenceMap(ast, project_clause);

	return ast;
}

void AST_SetParamsParseResult(AST *ast, cypher_parse_result_t *params_parse_result) {
	// When setting this value in AST, the ast should no hold invalid pointers or leftovers from previous executions.
	ASSERT(ast->params_parse_result == NULL);
	ast->params_parse_result = params_parse_result;
}

AST *AST_ShallowCopy(AST *orig) {
	AST_IncreaseRefCount(orig);
	size_t ast_size = sizeof(AST);
	AST *shallow_copy = rm_malloc(ast_size);
	memcpy(shallow_copy, orig, ast_size);
	shallow_copy->params_parse_result = NULL;
	return shallow_copy;
}

inline bool AST_AliasIsReferenced(AST *ast, const char *alias) {
	return (raxFind(ast->referenced_entities, (unsigned char *)alias, strlen(alias)) != raxNotFound);
}

bool AST_IdentifierIsAlias(const cypher_astnode_t *root, const char *identifier) {
	if(cypher_astnode_type(root) == CYPHER_AST_PROJECTION) {
		const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(root);
		// If this projection is aliased, check the alias.
		if(alias_node) {
			const char *alias = cypher_ast_identifier_get_name(alias_node);
			if(!strcmp(alias, identifier)) return true; // The identifier is an alias.
		} else {
			if(cypher_astnode_type(root) == CYPHER_AST_IDENTIFIER) {
				// If the projection itself is the identifier, it is not an alias.
				const char *current_identifier = cypher_ast_identifier_get_name(alias_node);
				if(!strcmp(current_identifier, identifier)) return false;
			}
		}
	}

	// Recursively visit children.
	uint child_count = cypher_astnode_nchildren(root);
	for(uint i = 0; i < child_count; i ++) {
		bool alias_found = AST_IdentifierIsAlias(cypher_astnode_get_child(root, i), identifier);
		if(alias_found) return true;
	}
	return false;
}

// TODO Consider augmenting libcypher-parser so that we don't need to perform this
// work in-module.
inline long AST_ParseIntegerNode(const cypher_astnode_t *int_node) {
	ASSERT(int_node);

	const char *value_str = cypher_ast_integer_get_valuestr(int_node);
	return strtol(value_str, NULL, 0);
}

bool AST_ClauseContainsAggregation(const cypher_astnode_t *clause) {
	ASSERT(clause);

	bool aggregated = false;

	// Retrieve all user-specified functions in clause.
	rax *referred_funcs = raxNew();
	AST_ReferredFunctions(clause, referred_funcs);

	char funcName[32];
	raxIterator it;
	_prepareIterateAll(referred_funcs, &it);
	while(raxNext(&it)) {
		size_t len = it.key_len;
		ASSERT(len < 32);
		// Copy the triemap key so that we can safely add a terinator character
		memcpy(funcName, it.key, len);
		funcName[len] = 0;

		if(AR_FuncIsAggregate(funcName)) {
			aggregated = true;
			break;
		}
	}
	raxStop(&it);
	raxFree(referred_funcs);

	return aggregated;
}

const char *AST_GetEntityName(const AST *ast, const cypher_astnode_t *entity) {
	AnnotationCtx *name_ctx = AST_AnnotationCtxCollection_GetNameCtx(ast->anot_ctx_collection);
	return cypher_astnode_get_annotation(name_ctx, entity);
}

const char **AST_GetProjectAll(const cypher_astnode_t *projection_clause) {
	AST *ast = QueryCtx_GetAST();
	AnnotationCtx *project_all_ctx = AST_AnnotationCtxCollection_GetProjectAllCtx(
										 ast->anot_ctx_collection);
	return cypher_astnode_get_annotation(project_all_ctx, projection_clause);
}

const char **AST_BuildReturnColumnNames(const cypher_astnode_t *return_clause) {
	const char **columns;
	if(cypher_ast_return_has_include_existing(return_clause)) {
		// If this is a RETURN *, the column names should be retrieved from the clause annotation.
		const char **projection_names = AST_GetProjectAll(return_clause);
		array_clone(columns, projection_names);
		return columns;
	}

	// Collect every alias from the RETURN projections.
	uint projection_count = cypher_ast_return_nprojections(return_clause);
	columns = array_new(const char *, projection_count);
	for(uint i = 0; i < projection_count; i++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause, i);
		const cypher_astnode_t *ast_alias = cypher_ast_projection_get_alias(projection);
		// If the projection was not aliased, the projection itself is an identifier.
		if(ast_alias == NULL) ast_alias = cypher_ast_projection_get_expression(projection);
		const char *alias = cypher_ast_identifier_get_name(ast_alias);
		columns = array_append(columns, alias);
	}

	return columns;
}

const char **AST_BuildCallColumnNames(const cypher_astnode_t *call_clause) {
	const char **proc_output_columns = NULL;
	uint yield_count = cypher_ast_call_nprojections(call_clause);
	if(yield_count > 0) {
		proc_output_columns = array_new(const char *, yield_count);
		for(uint i = 0; i < yield_count; i ++) {
			const cypher_astnode_t *projection = cypher_ast_call_get_projection(call_clause, i);
			const cypher_astnode_t *ast_exp = cypher_ast_projection_get_expression(projection);

			const char *identifier = NULL;
			const cypher_astnode_t *alias_node = cypher_ast_projection_get_alias(projection);
			if(alias_node) {
				// The projection either has an alias (AS), is a function call, or is a property specification (e.name).
				identifier = cypher_ast_identifier_get_name(alias_node);
			} else {
				// This expression did not have an alias, so it must be an identifier
				ASSERT(cypher_astnode_type(ast_exp) == CYPHER_AST_IDENTIFIER);
				// Retrieve "a" from "RETURN a" or "RETURN a AS e" (theoretically; the latter case is already handled)
				identifier = cypher_ast_identifier_get_name(ast_exp);
			}
			proc_output_columns = array_append(proc_output_columns, identifier);
		}
	} else {
		// If the procedure call is missing its yield part, include procedure outputs.
		const char *proc_name = cypher_ast_proc_name_get_value(cypher_ast_call_get_proc_name(call_clause));
		ProcedureCtx *proc = Proc_Get(proc_name);
		ASSERT(proc);
		unsigned int output_count = Procedure_OutputCount(proc);
		proc_output_columns = array_new(const char *, output_count);
		for(uint i = 0; i < output_count; i++) {
			proc_output_columns = array_append(proc_output_columns, Procedure_GetOutput(proc, i));
		}
		Proc_Free(proc);
	}
	return proc_output_columns;
}

const char *_AST_ExtractQueryString(const cypher_parse_result_t *partial_result) {
	// Retrieve the AST root node from a parsed query.
	const cypher_astnode_t *statement = _AST_parse_result_root(partial_result);
	// We are parsing with the CYPHER_PARSE_ONLY_PARAMETERS flag.
	// Given that, only the parameters were processed. extract the actual query and return to caller.
	ASSERT(cypher_astnode_type(statement) == CYPHER_AST_STATEMENT);
	const cypher_astnode_t *body = cypher_ast_statement_get_body(statement);
	ASSERT(cypher_astnode_type(body) == CYPHER_AST_STRING);
	return cypher_ast_string_get_value(body);
}

inline AST_AnnotationCtxCollection *AST_GetAnnotationCtxCollection(AST *ast) {
	return ast->anot_ctx_collection;
}

void AST_Free(AST *ast) {
	if(ast == NULL) return;

	int ref_count = AST_DecRefCount(ast);

	/* free and nullify parameters parse result if needed,
	 * after execution, as they are only save for the execution lifetime */
	if(ast->params_parse_result) {
		parse_result_free(ast->params_parse_result);
	}

	// check if the ast has additional copies
	if(ref_count == 0) {
		// no valid references, the struct can be disposed completely
		if(ast->free_root) {
			// this is a generated AST, free its root node
			cypher_astnode_free((cypher_astnode_t *) ast->root);
		} else {
			/* this is the master AST,
			 * free the annotation contexts that have been constructed */
			AST_AnnotationCtxCollection_Free(ast->anot_ctx_collection);
			raxFreeWithCallback(ast->canonical_entity_names, rm_free);
			parse_result_free(ast->parse_result);
		}

		if(ast->referenced_entities) raxFree(ast->referenced_entities);

		rm_free(ast->ref_count);
	}

	rm_free(ast);
}

cypher_parse_result_t *parse_query(const char *query) {
	cypher_parse_result_t *result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_STATEMENTS);
	if(!result) return NULL;
	if(AST_Validate_Query(result) != AST_VALID) {
		parse_result_free(result);
		return NULL;
	}
	return result;
}


cypher_parse_result_t *parse_params(const char *query, const char **query_body) {
	cypher_parse_result_t *result = cypher_parse(query, NULL, NULL, CYPHER_PARSE_ONLY_PARAMETERS);
	if(!result) return NULL;
	if(AST_Validate_QueryParams(result) != AST_VALID) {
		parse_result_free(result);
		return NULL;
	}
	_AST_Extract_Params(result);
	if(query_body) *query_body = _AST_ExtractQueryString(result);
	return result;
}

void parse_result_free(cypher_parse_result_t *parse_result) {
	if(parse_result) cypher_parse_result_free(parse_result);
}


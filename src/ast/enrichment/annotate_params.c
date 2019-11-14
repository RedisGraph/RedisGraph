/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../ast.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../arithmetic/arithmetic_expression.h"

//------------------------------------------------------------------------------
//  Annotation context - query parameters
//------------------------------------------------------------------------------

// AST annotation callback routine for freeing arithmetic expressions.
static void _FreeParamAnnotationCallback(void *unused, const cypher_astnode_t *node,
										 void *annotation) {
	AR_EXP_Free((AR_ExpNode *)annotation);
}

static AnnotationCtx *_AST_NewParamsContext(void) {
	AnnotationCtx *param_ctx = cypher_ast_annotation_context();
	cypher_ast_annotation_context_release_handler_t handler = &_FreeParamAnnotationCallback;
	cypher_ast_annotation_context_set_release_handler(param_ctx, handler, NULL);
	return param_ctx;
}

/* This function collects every AST node which is a named parameter place holder. If the same name
 * of a parameter apears in more then one place it is append to array of all the placeholders with
 * the same name.
 * The function returns a rax with parameter name as key, and the array of placeholders as values. */
static void _collect_query_params_map(const cypher_astnode_t *ast_exp, rax *parameter_map) {
	cypher_astnode_type_t type = cypher_astnode_type(ast_exp);
	// In case of parameter.
	if(type == CYPHER_AST_PARAMETER) {
		const char *identifier = cypher_ast_parameter_get_name(ast_exp);
		const cypher_astnode_t **exp_arr = raxFind(parameter_map, (unsigned char *)identifier,
												   strlen(identifier));
		// Use array in case of same parameter is used in multiple locations.
		if(exp_arr == raxNotFound) exp_arr = array_new(const cypher_astnode_t *, 1);
		exp_arr = array_append(exp_arr, ast_exp);
		raxInsert(parameter_map, (unsigned char *)identifier, strlen(identifier), (void *)exp_arr, NULL);
	} else {
		// Recurse over children.
		uint child_count = cypher_astnode_nchildren(ast_exp);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(ast_exp, i);
			// Recursively continue mapping.
			_collect_query_params_map(child, parameter_map);
		}
	}
}

static void _annotate_params(AST *ast) {
	// Check for number of parameters, if there aren't any, return;
	rax *params_values = QueryCtx_GetParams();
	if(!params_values || raxSize(params_values) == 0) return;
	AnnotationCtx *params_ctx = AST_AnnotationCtxCollection_GetParamsCtx(ast->anot_ctx_collection);
	rax *query_params_map = raxNew();
	_collect_query_params_map(ast->root, query_params_map);
	raxIterator iter;
	raxStart(&iter, query_params_map);
	raxSeek(&iter, "^", NULL, 0);
	while(raxNext(&iter)) {
		const char *key = (const char *)iter.key;
		const cypher_astnode_t **exp_arr = iter.data;
		const cypher_astnode_t *param_value = raxFind(params_values, (unsigned char *) key, iter.key_len);
		assert(param_value != raxNotFound);
		uint array_length = array_len(exp_arr);
		for(uint i = 0; i < array_length; i++) {
			cypher_astnode_attach_annotation(params_ctx, exp_arr[i], (void *)param_value, NULL);
		}

	}
	raxStop(&iter);
	raxFreeWithCallback(query_params_map, array_free);
}

void AST_AnnotateParams(AST *ast) {
	AST_AnnotationCtxCollection_SetParamsCtx(ast->anot_ctx_collection, _AST_NewParamsContext());
	_annotate_params(ast);
}

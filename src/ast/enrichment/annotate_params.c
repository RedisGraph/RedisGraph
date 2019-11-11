/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../ast.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
//------------------------------------------------------------------------------
//  Annotation context - query parameters
//------------------------------------------------------------------------------

static AnnotationCtx *_AST_NewParamsContext(void) {
	AnnotationCtx *project_all_ctx = cypher_ast_annotation_context();
	return project_all_ctx;
}

static void _collect_query_params_map(const cypher_astnode_t *ast_exp, rax *parameter_map) {
	cypher_astnode_type_t type = cypher_astnode_type(ast_exp);
	// In case of identifier.
	if(type == CYPHER_AST_PARAMETER) {
		const char *identifier = cypher_ast_parameter_get_name(ast_exp);
		const cypher_astnode_t **exp_arr = raxFind(parameter_map, (unsigned char *)identifier,
												   strlen(identifier));
		// Use array in case of multiple projections for the same named path.
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
	if(raxSize(params_values) == 0) return;
	rax *query_params_map = raxNew();
	_collect_query_params_map(ast->root, query_params_map);
	raxIterator iter;
	raxStart(&iter, query_params_map);
	raxSeek(&iter, "^", NULL, 0);
	while(raxNext(&iter)) {
		const char *key = (const char *)iter.key;
		const cypher_astnode_t **exp_arr = iter.data;
		const cypher_astnode_t *param_value = raxFind(params_values, (unsigned char *) key, strlen(key));
		if(param_value != raxNotFound) {
			uint array_length = array_len(exp_arr);
			for(uint i = 0; i < array_length; i++) {
				cypher_astnode_attach_annotation(ast->params_ctx, exp_arr[i], (void *)param_value, NULL);
			}
		}
	}
	raxStop(&iter);
	raxFreeWithCallback(query_params_map, array_free);
}

void AST_AnnotateParams(AST *ast) {
	ast->params_ctx = _AST_NewParamsContext();
	_annotate_params(ast);
}

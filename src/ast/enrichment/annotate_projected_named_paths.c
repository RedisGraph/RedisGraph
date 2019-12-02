/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "annotate_projected_named_paths.h"
#include "../../util/arr.h"

//------------------------------------------------------------------------------
//  Annotation context - Attach named path projection identifiers their respective ast structure
//------------------------------------------------------------------------------

static AnnotationCtx *_AST_NewProjectNamedPathContext(void) {
	AnnotationCtx *project_all_ctx = cypher_ast_annotation_context();
	return project_all_ctx;
}

static void _annotate_relevant_projected_named_path_identifier(AST *ast,
															   rax *identifier_map, uint scope_start, uint scope_end) {
	for(uint clause_iter = scope_start; clause_iter < scope_end; clause_iter++) {
		AnnotationCtx *named_paths_ctx = AST_AnnotationCtxCollection_GetNamedPathsCtx(
											 ast->anot_ctx_collection);
		const cypher_astnode_t *clause = cypher_ast_query_get_clause(ast->root, clause_iter);
		const cypher_astnode_type_t clause_type = cypher_astnode_type(clause);
		// Match.
		if(clause_type == CYPHER_AST_MATCH) {
			const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(clause);
			uint path_count = cypher_ast_pattern_npaths(pattern);
			for(uint i = 0; i < path_count; i++) {
				const cypher_astnode_t *path = cypher_ast_pattern_get_path(pattern, i);
				if(cypher_astnode_type(path) == CYPHER_AST_NAMED_PATH) {
					const cypher_astnode_t *path_identifier = cypher_ast_named_path_get_identifier(path);
					const char *path_name = cypher_ast_identifier_get_name(path_identifier);
					const cypher_astnode_t **exp_arr = raxFind(identifier_map, (unsigned char *)path_name,
															   strlen(path_name));
					if(exp_arr != raxNotFound) {
						uint arrayLen = array_len(exp_arr);
						for(uint i = 0; i < arrayLen; i++)
							cypher_astnode_attach_annotation(named_paths_ctx, exp_arr[i], (void *)path, NULL);
					}
				}
			}
			// Merge.
		} else if(clause_type == CYPHER_AST_MERGE) {
			const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(clause);
			if(cypher_astnode_type(path) == CYPHER_AST_NAMED_PATH) {
				const cypher_astnode_t *path_identifier = cypher_ast_named_path_get_identifier(path);
				const char *path_name = cypher_ast_identifier_get_name(path_identifier);
				const cypher_astnode_t **exp_arr = raxFind(identifier_map, (unsigned char *)path_name,
														   strlen(path_name));
				if(exp_arr != raxNotFound) {
					uint arrayLen = array_len(exp_arr);
					for(uint i = 0; i < arrayLen; i++)
						cypher_astnode_attach_annotation(named_paths_ctx, exp_arr[i], (void *)path, NULL);
				}
			}
		}
	}
}

void _collect_projected_identifier(const cypher_astnode_t *ast_exp, rax *identifier_map) {
	cypher_astnode_type_t type = cypher_astnode_type(ast_exp);
	// In case of identifier.
	if(type == CYPHER_AST_IDENTIFIER) {
		const char *identifier = cypher_ast_identifier_get_name(ast_exp);
		const cypher_astnode_t **exp_arr = raxFind(identifier_map, (unsigned char *)identifier,
												   strlen(identifier));
		// Use array in case of multiple projections for the same named path.
		if(exp_arr == raxNotFound) exp_arr = array_new(const cypher_astnode_t *, 1);
		exp_arr = array_append(exp_arr, ast_exp);
		raxInsert(identifier_map, (unsigned char *)identifier, strlen(identifier), (void *)exp_arr, NULL);
	} else {
		// Recurse over children.
		uint child_count = cypher_astnode_nchildren(ast_exp);
		for(uint i = 0; i < child_count; i++) {
			const cypher_astnode_t *child = cypher_astnode_get_child(ast_exp, i);
			// Recursively continue mapping.
			_collect_projected_identifier(child, identifier_map);
		}
	}
}

static void _annotate_with_clause_projected_named_path(AST *ast,
													   const cypher_astnode_t *with_clause, uint scope_start, uint scope_end) {
	// Iterate over the projections and find their identifiers.
	rax *identifier_map = raxNew();
	uint with_projection_count = cypher_ast_with_nprojections(with_clause);
	for(uint projection_iter = 0; projection_iter < with_projection_count; projection_iter++) {
		const cypher_astnode_t *projection = cypher_ast_with_get_projection(with_clause, projection_iter);
		_collect_projected_identifier(projection, identifier_map);
	}
	_annotate_relevant_projected_named_path_identifier(ast, identifier_map, scope_start, scope_end);
	raxFreeWithCallback(identifier_map, array_free);
}

static void _annotate_return_clause_projected_named_path(AST *ast,
														 const cypher_astnode_t *return_clause, uint scope_start, uint scope_end) {
	rax *identifier_map = raxNew();
	uint return_projection_count = cypher_ast_return_nprojections(return_clause);
	for(uint projection_iter = 0; projection_iter < return_projection_count; projection_iter++) {
		const cypher_astnode_t *projection = cypher_ast_return_get_projection(return_clause,
																			  projection_iter);
		_collect_projected_identifier(projection, identifier_map);
	}
	_annotate_relevant_projected_named_path_identifier(ast, identifier_map, scope_start, scope_end);
	raxFreeWithCallback(identifier_map, array_free);
}

static void _annotate_projected_named_path(AST *ast) {
	uint *with_clause_indices = AST_GetClauseIndices(ast, CYPHER_AST_WITH);
	uint with_clause_count = array_len(with_clause_indices);
	uint scope_start = 0;
	uint scope_end;
	// Handle with clauses.
	for(uint i = 0; i < with_clause_count; i ++) {
		scope_end = with_clause_indices[i];
		const cypher_astnode_t *with_clause = cypher_ast_query_get_clause(ast->root, scope_end);
		_annotate_with_clause_projected_named_path(ast, with_clause, scope_start, scope_end);
		scope_start = scope_end;
	}
	array_free(with_clause_indices);

	uint *return_clause_indices = AST_GetClauseIndices(ast, CYPHER_AST_RETURN);
	uint return_clause_count = array_len(return_clause_indices);
	scope_start = 0;
	for(uint i = 0; i < return_clause_count; i++) {
		scope_end = return_clause_indices[i];
		const cypher_astnode_t *return_clause = cypher_ast_query_get_clause(ast->root, scope_end);
		_annotate_return_clause_projected_named_path(ast, return_clause, scope_start, scope_end);
		scope_start = scope_end;
	}
	array_free(return_clause_indices);
}

void AST_AnnotateNamedPaths(AST *ast) {
	AST_AnnotationCtxCollection_SetNamedPathsCtx(ast->anot_ctx_collection,
												 _AST_NewProjectNamedPathContext());
	_annotate_projected_named_path(ast);
}

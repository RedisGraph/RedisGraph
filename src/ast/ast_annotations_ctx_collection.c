/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast_annotations_ctx_collection.h"
#include "../util/rmalloc.h"

static void _free_to_string_annotation
(
	void *userdata,
	const cypher_astnode_t *node,
	void *annotation
) {
	rm_free(annotation);
}

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New() {
	AST_AnnotationCtxCollection *anotCtxCollection = rm_malloc(sizeof(AST_AnnotationCtxCollection));
	anotCtxCollection->named_paths_ctx = NULL;
	anotCtxCollection->project_all_ctx = NULL;
	anotCtxCollection->to_string_ctx   = cypher_ast_annotation_context();
	// set release handler to free allocated string
	cypher_ast_annotation_context_set_release_handler(anotCtxCollection->to_string_ctx, _free_to_string_annotation, NULL);
	anotCtxCollection->anon_count      = 0;
	return anotCtxCollection;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetNamedPathsCtx(const AST_AnnotationCtxCollection
																   *anot_ctx_collection) {
	return anot_ctx_collection->named_paths_ctx;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetProjectAllCtx(const AST_AnnotationCtxCollection
																   *anot_ctx_collection) {
	return anot_ctx_collection->project_all_ctx;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetToStringCtx(const AST_AnnotationCtxCollection
																   *anot_ctx_collection) {
	return anot_ctx_collection->to_string_ctx;
}

inline void AST_AnnotationCtxCollection_SetNamedPathsCtx(AST_AnnotationCtxCollection
														 *anot_ctx_collection,
														 AnnotationCtx *named_paths_ctx) {
	anot_ctx_collection->named_paths_ctx = named_paths_ctx;
}

inline void AST_AnnotationCtxCollection_SetProjectAllCtx(AST_AnnotationCtxCollection
														 *anotCtxCollection,
														 AnnotationCtx *project_all_ctx) {
	anotCtxCollection->project_all_ctx = project_all_ctx;
}

void AST_AnnotationCtxCollection_Free(AST_AnnotationCtxCollection *anotCtxCollection) {
	if(anotCtxCollection) {
		if(anotCtxCollection->named_paths_ctx) cypher_ast_annotation_context_free(
				anotCtxCollection->named_paths_ctx);
		if(anotCtxCollection->project_all_ctx) cypher_ast_annotation_context_free(
				anotCtxCollection->project_all_ctx);
		cypher_ast_annotation_context_free(anotCtxCollection->to_string_ctx);
		rm_free(anotCtxCollection);
	}
}

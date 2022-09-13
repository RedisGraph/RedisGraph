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
	free(annotation);
}

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New() {
	AST_AnnotationCtxCollection *anotCtxCollection = rm_malloc(sizeof(AST_AnnotationCtxCollection));
	anotCtxCollection->to_string_ctx   = cypher_ast_annotation_context();
	// set release handler to free allocated string
	cypher_ast_annotation_context_set_release_handler(anotCtxCollection->to_string_ctx, _free_to_string_annotation, NULL);
	anotCtxCollection->anon_count      = 0;
	return anotCtxCollection;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetToStringCtx(const AST_AnnotationCtxCollection
																   *anot_ctx_collection) {
	return anot_ctx_collection->to_string_ctx;
}

void AST_AnnotationCtxCollection_Free(AST_AnnotationCtxCollection *anotCtxCollection) {
	if(anotCtxCollection) {
		cypher_ast_annotation_context_free(anotCtxCollection->to_string_ctx);
		rm_free(anotCtxCollection);
	}
}

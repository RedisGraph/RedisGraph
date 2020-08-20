/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast_annotations_ctx_collection.h"
#include "../util/rmalloc.h"

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New() {
	AST_AnnotationCtxCollection *anotCtxCollection = rm_malloc(sizeof(AST_AnnotationCtxCollection));
	anotCtxCollection->name_ctx = NULL;
	anotCtxCollection->named_paths_ctx = NULL;
	anotCtxCollection->project_all_ctx = NULL;
	return anotCtxCollection;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetNameCtx(const AST_AnnotationCtxCollection
															 *anot_ctx_collection) {
	return anot_ctx_collection->name_ctx;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetNamedPathsCtx(const AST_AnnotationCtxCollection
																   *anot_ctx_collection) {
	return anot_ctx_collection->named_paths_ctx;
}

inline AnnotationCtx *AST_AnnotationCtxCollection_GetProjectAllCtx(const AST_AnnotationCtxCollection
																   *anot_ctx_collection) {
	return anot_ctx_collection->project_all_ctx;
}

inline void AST_AnnotationCtxCollection_SetNameCtx(AST_AnnotationCtxCollection *anot_ctx_collection,
												   AnnotationCtx *name_ctx) {
	anot_ctx_collection->name_ctx = name_ctx;
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
		if(anotCtxCollection->name_ctx) cypher_ast_annotation_context_free(anotCtxCollection->name_ctx);
		if(anotCtxCollection->named_paths_ctx) cypher_ast_annotation_context_free(
				anotCtxCollection->named_paths_ctx);
		if(anotCtxCollection->project_all_ctx) cypher_ast_annotation_context_free(
				anotCtxCollection->project_all_ctx);
		rm_free(anotCtxCollection);
	}
}

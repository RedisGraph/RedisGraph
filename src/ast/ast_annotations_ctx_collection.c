/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast_annotations_ctx_collection.h"
#include "../util/rmalloc.h"

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New() {
	AST_AnnotationCtxCollection *anotCtxCollection = rm_malloc(sizeof(AST_AnnotationCtxCollection));
	anotCtxCollection->name_ctx = NULL;
	anotCtxCollection->params_ctx = NULL;
	anotCtxCollection->named_paths_ctx = NULL;
	anotCtxCollection->project_all_ctx = NULL;
	return anotCtxCollection;
}

AnnotationCtx *AST_AnnotationCtxCollection_GetNameCtx(AST_AnnotationCtxCollection
													  *anotCtxCollection) {
	return anotCtxCollection->name_ctx;
}

AnnotationCtx *AST_AnnotationCtxCollection_GetParamsCtx(AST_AnnotationCtxCollection
														*anotCtxCollection) {
	return anotCtxCollection->params_ctx;
}

AnnotationCtx *AST_AnnotationCtxCollection_GetNamedPathsCtx(AST_AnnotationCtxCollection
															*anotCtxCollection) {
	return anotCtxCollection->named_paths_ctx;
}

AnnotationCtx *AST_AnnotationCtxCollection_GetProjectAllCtx(AST_AnnotationCtxCollection
															*anotCtxCollection) {
	return anotCtxCollection->project_all_ctx;
}

void AST_AnnotationCtxCollection_SetNameCtx(AST_AnnotationCtxCollection *anotCtxCollection,
											AnnotationCtx *name_ctx) {
	anotCtxCollection->name_ctx = name_ctx;
}

void AST_AnnotationCtxCollection_SetParamsCtx(AST_AnnotationCtxCollection *anotCtxCollection,
											  AnnotationCtx *params_ctx) {
	anotCtxCollection->params_ctx = params_ctx;
}

void AST_AnnotationCtxCollection_SetNamedPathsCtx(AST_AnnotationCtxCollection *anotCtxCollection,
												  AnnotationCtx *named_paths_ctx) {
	anotCtxCollection->named_paths_ctx = named_paths_ctx;
}

void AST_AnnotationCtxCollection_SetProjectAllCtx(AST_AnnotationCtxCollection *anotCtxCollection,
												  AnnotationCtx *project_all_ctx) {
	anotCtxCollection->project_all_ctx = project_all_ctx;
}

void AST_AnnotationCtxCollection_Free(AST_AnnotationCtxCollection *anotCtxCollection) {
	if(anotCtxCollection) {
		if(anotCtxCollection->name_ctx) cypher_ast_annotation_context_free(anotCtxCollection->name_ctx);
		if(anotCtxCollection->params_ctx) cypher_ast_annotation_context_free(anotCtxCollection->params_ctx);
		if(anotCtxCollection->named_paths_ctx) cypher_ast_annotation_context_free(
				anotCtxCollection->named_paths_ctx);
		if(anotCtxCollection->project_all_ctx) cypher_ast_annotation_context_free(
				anotCtxCollection->project_all_ctx);
		rm_free(anotCtxCollection);
	}
}

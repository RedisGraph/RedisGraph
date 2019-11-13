/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include "cypher-parser.h"

typedef cypher_ast_annotation_context_t AnnotationCtx;

typedef struct {
	AnnotationCtx *name_ctx;        // Annotation context for naming graph entities and ORDER items.
	AnnotationCtx *params_ctx;      // Annotation context for query parameters.
	AnnotationCtx *project_all_ctx; // Context containing aliases for WITH/RETURN * projections.
	AnnotationCtx *named_paths_ctx; // Annotation context for named paths projections.
} AST_AnnotationCtxCollection;

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New();

AnnotationCtx *AST_AnnotationCtxCollection_GetNameCtx(AST_AnnotationCtxCollection
													  *anotCtxCollection);
AnnotationCtx *AST_AnnotationCtxCollection_GetParamsCtx(AST_AnnotationCtxCollection
														*anotCtxCollection);
AnnotationCtx *AST_AnnotationCtxCollection_GetNamedPathsCtx(AST_AnnotationCtxCollection
															*anotCtxCollection);
AnnotationCtx *AST_AnnotationCtxCollection_GetProjectAllCtx(AST_AnnotationCtxCollection
															*anotCtxCollection);

void AST_AnnotationCtxCollection_SetNameCtx(AST_AnnotationCtxCollection *anotCtxCollection,
											AnnotationCtx *name_ctx);
void AST_AnnotationCtxCollection_SetParamsCtx(AST_AnnotationCtxCollection *anotCtxCollection,
											  AnnotationCtx *params_ctx);
void AST_AnnotationCtxCollection_SetNamedPathsCtx(AST_AnnotationCtxCollection *anotCtxCollection,
												  AnnotationCtx *named_paths_ctx);
void AST_AnnotationCtxCollection_SetProjectAllCtx(AST_AnnotationCtxCollection *anotCtxCollection,
												  AnnotationCtx *project_all_ctx);

void AST_AnnotationCtxCollection_Free(AST_AnnotationCtxCollection *anotCtxCollection);



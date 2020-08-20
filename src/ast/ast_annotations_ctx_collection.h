/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once
#include "cypher-parser.h"

typedef cypher_ast_annotation_context_t AnnotationCtx;

/* This struct holds the AST annotations context. Each annotation context is used in
 * specific AST nodes annotation use case. For each context there is a dedicated inlined
 * getter and setter. */
typedef struct {
	AnnotationCtx *name_ctx;        // Annotation context for naming graph entities and ORDER items.	
	AnnotationCtx *project_all_ctx; // Context containing aliases for WITH/RETURN * projections.
	AnnotationCtx *named_paths_ctx; // Annotation context for named paths projections.
} AST_AnnotationCtxCollection;

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New();

AnnotationCtx *AST_AnnotationCtxCollection_GetNameCtx(const AST_AnnotationCtxCollection *anot_ctx_collection);

AnnotationCtx *AST_AnnotationCtxCollection_GetNamedPathsCtx(const AST_AnnotationCtxCollection *anot_ctx_collection);

AnnotationCtx *AST_AnnotationCtxCollection_GetProjectAllCtx(const AST_AnnotationCtxCollection *anot_ctx_collection);

void AST_AnnotationCtxCollection_SetNameCtx(AST_AnnotationCtxCollection *anot_ctx_collection, AnnotationCtx *name_ctx);

void AST_AnnotationCtxCollection_SetNamedPathsCtx(AST_AnnotationCtxCollection *anot_ctx_collection, AnnotationCtx *named_paths_ctx);

void AST_AnnotationCtxCollection_SetProjectAllCtx(AST_AnnotationCtxCollection *anot_ctx_collection, AnnotationCtx *project_all_ctx);

void AST_AnnotationCtxCollection_Free(AST_AnnotationCtxCollection *anot_ctx_collection);

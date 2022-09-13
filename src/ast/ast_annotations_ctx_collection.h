/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
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
	AnnotationCtx *to_string_ctx;   // Annotation context for AST_ToString of astnode.
	uint anon_count;                // Counter of anonymous entities already created.
} AST_AnnotationCtxCollection;

AST_AnnotationCtxCollection *AST_AnnotationCtxCollection_New();

AnnotationCtx *AST_AnnotationCtxCollection_GetToStringCtx(const AST_AnnotationCtxCollection *anot_ctx_collection);

void AST_AnnotationCtxCollection_Free(AST_AnnotationCtxCollection *anot_ctx_collection);

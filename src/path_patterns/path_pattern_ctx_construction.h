#pragma once

#include "path_pattern_ctx.h"
#include "../ast/ast.h"

/* Builds named path patterns from ast */
PathPatternCtx *PathPatternCtx_Build(AST *ast, size_t required_dim);

void PathPatternCtx_BuildAndAddPathPattern(
	const char *name,
	EBNFBase *root,
	PathPatternCtx *ctx
);
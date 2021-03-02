#pragma once

#include "path_pattern_ctx.h"
#include "../ast/ast.h"

PathPatternCtx *PathPatternCtx_Build(AST *ast, size_t required_dim);

// It builds path pattern and its transposed version and adds them into path pattern context.
void PathPatternCtx_BuildAndAddPathPattern(
	const char *name,
	EBNFBase *root,
	PathPatternCtx *ctx
);
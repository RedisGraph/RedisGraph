#include "path_pattern_ctx.h"
#include "../util/arr.h"
#include "../arithmetic/algebraic_expression/utils.h"

PathPattern* _PathPatternCtx_FindPathPattern(PathPattern **patterns, AlgExpReference ref) {
    for (int i = 0; i < array_len(patterns); ++i) {
        PathPattern *pattern = patterns[i];
        assert(pattern->reference.name);
        if (strcmp(pattern->reference.name, ref.name) == 0 && pattern->reference.transposed == ref.transposed) {
			return pattern;
		}
    }
    return NULL;
}

void _PathPatternCtx_DFS(PathPatternCtx *ctx, AlgebraicExpression *root, PathPattern ***visited) {
    if (root->type == AL_OPERATION) {
        for (int i = 0; i < AlgebraicExpression_ChildCount(root); ++i) {
            _PathPatternCtx_DFS(ctx, CHILD_AT(root, i), visited);
        }
    } else {
        AlgExpReference reference = root->operand.reference;
        if (AlgebraicExpression_OperandIsReference(root) && (_PathPatternCtx_FindPathPattern(*visited, reference) == NULL)) {
            PathPattern *next = PathPatternCtx_GetPathPattern(ctx, reference);
            assert(next != NULL && "Unresolved path reference");

            *visited = array_append(*visited, next);
            _PathPatternCtx_DFS(ctx, next->ae, visited);
        }
    }
}

PathPatternCtx *PathPatternCtx_New(size_t required_matrix_dim) {
    PathPatternCtx *ctx = rm_malloc(sizeof(PathPatternCtx));
    ctx->patterns = array_new(PathPattern *, 1);
	ctx->required_matrix_dim = required_matrix_dim;
    ctx->anon_patterns_cnt = 0;
    return ctx;
}

void PathPatternCtx_AddPathPattern(PathPatternCtx *ctx, PathPattern *pattern) {
    ctx->patterns = array_append(ctx->patterns, pattern);
}

char *PathPatternCtx_GetNextAnonName(PathPatternCtx *ctx) {
	char *name = rm_malloc(10 * sizeof(char));
	sprintf(name, "anon_%d", ctx->anon_patterns_cnt++);
	return name;
}

PathPattern* PathPatternCtx_GetPathPattern(PathPatternCtx *ctx, AlgExpReference reference) {
    return _PathPatternCtx_FindPathPattern(ctx->patterns, reference);
}

PathPattern **PathPatternCtx_GetDependencies(PathPatternCtx *ctx, AlgebraicExpression *expr) {
    PathPattern **visited = array_new(PathPattern*,1);
    _PathPatternCtx_DFS(ctx, expr, &visited);
    return visited;
}

void PathPatternCtx_Free(PathPatternCtx *pathPatternCtx) {
    if (pathPatternCtx != NULL) {
        for (int i = 0; i < array_len(pathPatternCtx->patterns); ++i) {
            PathPattern_Free(pathPatternCtx->patterns[i]);
        }
        array_free(pathPatternCtx->patterns);
        rm_free(pathPatternCtx);
    }
}

void PathPatternCtx_Show(PathPatternCtx *pathPatternCtx) {
	printf("PathPatternCtx: [%d]\n", array_len(pathPatternCtx->patterns));
	for (int i = 0; i < array_len(pathPatternCtx->patterns); ++i) {
		printf("PATH PATTERN %s:%d, AlgExp: %s, EBNF: %s\n",
		 	pathPatternCtx->patterns[i]->reference.name,
		 	pathPatternCtx->patterns[i]->reference.transposed,
			AlgebraicExpression_ToStringDebug(pathPatternCtx->patterns[i]->ae),
			EBNFBase_ToStr(pathPatternCtx->patterns[i]->ebnf_root));

//		EBNFBase_ToStr(pathPatternCtx->patterns[i]->ebnf_root),
// 		printf("Src: (%p)\n", pathPatternCtx->patterns[i]->src);
//		GxB_print(pathPatternCtx->patterns[i]->src, GxB_COMPLETE);
//		printf("M (%p):\n", pathPatternCtx->patterns[i]->m);
//		GxB_print(pathPatternCtx->patterns[i]->m, GxB_COMPLETE);
	}
}

void PathPatternCtx_ShowMatrices(PathPatternCtx *pathPatternCtx) {
	printf("PathPatternCtx: [%d]\n", array_len(pathPatternCtx->patterns));
	for (int i = 0; i < array_len(pathPatternCtx->patterns); ++i) {
		printf("PATH PATTERN %s\n", pathPatternCtx->patterns[i]->reference.name);
		printf("--src:\n");
		GxB_print(pathPatternCtx->patterns[i]->src, GxB_COMPLETE);
		printf("--m:\n");
		GxB_print(pathPatternCtx->patterns[i]->m, GxB_COMPLETE);
	}
}

PathPatternCtx *PathPatternCtx_Clone(PathPatternCtx *pathCtx) {
    PathPatternCtx *clone = PathPatternCtx_New(pathCtx->required_matrix_dim);
    clone->anon_patterns_cnt = pathCtx->anon_patterns_cnt;
    for (int i = 0; i < array_len(pathCtx->patterns); ++i) {
        clone->patterns = array_append(clone->patterns,
									   PathPattern_Clone(pathCtx->patterns[i]));
    }
    return clone;
}

void PathPatternCtx_ClearMatrices(PathPatternCtx *pathCtx) {
	for (int i = 0; i < array_len(pathCtx->patterns); ++i) {
		PathPattern *pathPattern = pathCtx->patterns[i];
		GrB_Matrix_clear(pathPattern->m);
		GrB_Matrix_clear(pathPattern->src);
	}
}

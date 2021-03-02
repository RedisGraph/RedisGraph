#include "path_pattern.h"
#include "../util/rmalloc.h"
#include "../arithmetic/algebraic_expression.h"
#include "../arithmetic/algebraic_expression/utils.h"

PathPattern *PathPattern_New(const char *name, EBNFBase *ebnf, size_t reqiured_mdim, bool transpose) {
    PathPattern *pattern = rm_malloc(sizeof(PathPattern));

	GrB_Matrix_new(&pattern->m, GrB_BOOL, reqiured_mdim, reqiured_mdim);
	GrB_Matrix_new(&pattern->src, GrB_BOOL, reqiured_mdim, reqiured_mdim);

    pattern->reference = AlgExpReference_New(name, transpose);
    pattern->ebnf_root = ebnf;


    AlgebraicExpression *ae = AlgebraicExpression_FromEbnf(ebnf);
    AlgebraicExpression_MultiplyToTheLeft(&ae, pattern->src);
    AlgebraicExpression_Optimize(&ae);
	AlgebraicExpression_ReplaceTransposedReferences(ae);

	pattern->ae = ae;
    return pattern;
}

char *PathPattern_ToString(PathPattern *pattern) {
    char *buf = rm_malloc(sizeof(char) * 1024);
    sprintf(buf, "%s = %s\n\t %s", pattern->reference.name, EBNFBase_ToStr(pattern->ebnf_root),
			AlgebraicExpression_ToStringDebug(pattern->ae));
    return buf;
}

void PathPattern_Free(PathPattern *pattern) {
    if (pattern->ebnf_root != NULL) {
        EBNFBase_Free(pattern->ebnf_root);
        pattern->ebnf_root = NULL;
    }
    if (pattern->ae != NULL) {
        AlgebraicExpression_Free(pattern->ae);
        pattern->ae = NULL;
    }
    if (pattern->m != GrB_NULL) {
        GrB_Matrix_free(&pattern->m);
        pattern->m = GrB_NULL;
    }
    if (pattern->src != GrB_NULL) {
    	GrB_Matrix_free(&pattern->src);
    	pattern->src = GrB_NULL;
    }
    AlgExpReference_Free(pattern->reference);
    rm_free(pattern);
}

PathPattern *PathPattern_Clone(PathPattern *other) {
	PathPattern *clone = rm_malloc(sizeof(PathPattern));

	clone->reference = AlgExpReference_Clone(&other->reference);
	clone->ebnf_root = EBNFBase_Clone(other->ebnf_root);

	GrB_Matrix_dup(&clone->m, other->m);
	GrB_Matrix_dup(&clone->src, other->src);

	AlgebraicExpression *ae = AlgebraicExpression_FromEbnf(clone->ebnf_root);
	AlgebraicExpression_MultiplyToTheLeft(&ae, clone->src);
	AlgebraicExpression_Optimize(&ae);

	AlgebraicExpression_ReplaceTransposedReferences(ae);

	clone->ae = ae;
    return clone;
}

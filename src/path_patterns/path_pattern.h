#pragma once

#include "ebnf.h"
#include "../arithmetic/algebraic_expression_structs.h"


typedef struct {
    AlgExpReference reference;
    EBNFBase *ebnf_root;
    AlgebraicExpression *ae;

    GrB_Matrix m;
    GrB_Matrix src;
} PathPattern;

/* Create new PathPattern with NULL algebraic expression. */
PathPattern *PathPattern_New(const char *name, EBNFBase *ebnf, size_t reqiured_mdim, bool transpose);

PathPattern *PathPattern_Clone(PathPattern *pathPattern);

char * PathPattern_ToString(PathPattern *pattern);

void PathPattern_Free(PathPattern *pattern);

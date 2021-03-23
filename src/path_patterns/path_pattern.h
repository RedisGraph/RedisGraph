#pragma once

#include "ebnf.h"
#include "../arithmetic/algebraic_expression_structs.h"

/* Represents named path pattern that can be found in
 * PATH PATTERN clause. The main part here
 * is algebraic expression, that will later be evaluated
 * into a matrix that specifies the relation corresponding
 * to this named path pattern. */
typedef struct {
    AlgExpReference reference; // Name of path pattern.
    EBNFBase *ebnf_root; 	   // Intermediate representation of path pattern.
    AlgebraicExpression *ae;   // Algebraic expression of path pattern.

    GrB_Matrix m;			   /* Matrix, that corresponds to the relation given by path pattern.
 								* This matrix is evaluated by transitive closure during
 								* CondTraverse operation execution. */
    GrB_Matrix src;			   /* Diagonal matrix representing the starting vertices from which
     						    * we are interested in finding all paths satisfying the pattern.
     						    * This matrix participates in evaluation of algebraic expression
     						    * and transitive closure of CondTraverse operation. */

} PathPattern;

/* Creates new PathPattern with NULL algebraic expression. */
PathPattern *PathPattern_New(const char *name, EBNFBase *ebnf, size_t reqiured_mdim, bool transpose);

/* Clones path pattern */
PathPattern *PathPattern_Clone(PathPattern *pathPattern);

/* Returns path pattern string representation */
char * PathPattern_ToString(PathPattern *pattern);

/* Frees path pattern */
void PathPattern_Free(PathPattern *pattern);

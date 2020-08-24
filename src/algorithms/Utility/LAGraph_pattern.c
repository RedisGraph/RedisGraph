//------------------------------------------------------------------------------
// LAGraph_pattern: return the pattern of a matrix (spones(A) in MATLAB)
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_pattern: return the pattern of a matrix (spones(A) in MATLAB)
// as a boolean matrix. Contributed by Tim Davis and Scott Kolodziej, Texas A&M.

// SPEC: to do this in general for any user-defined types requires either (a)
// the user to create an operator z=f(x)=1, where z is boolean and x is the
// user type (LAGraph_TRUE_BOOL_ComplexFP64, for example), or (b)
// extractTuples(&I,&J,&X,A).  The latter requires X to be allocated of the
// right size, and then freed.  SuiteSparse allows X to be NULL but this is an
// extension to the spec. Determining the right size of X is difficult since
// there is no GrB_Type_size (see GxB_Type_size in SuiteSparse:GraphBLAS).

// As a result of these limitations, this method does not handle user-defined
// types, other than LAGraph_ComplexFP64 (this function uses option (a) above).

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL \
    GrB_free (C);

GrB_Info LAGraph_pattern    // return GrB_SUCCESS if successful
(
    GrB_Matrix *C,          // a boolean matrix with the pattern of A
    GrB_Matrix A,
    GrB_Type T
)
{
    GrB_Info info;
    GrB_Type type;
    GrB_Index nrows, ncols;

    // check inputs
    if (C == NULL)
    {
        // error: required parameter, result, is NULL
        return (GrB_NULL_POINTER);
    }
    (*C) = NULL ;

    if (T == GrB_NULL)
    {
        T = GrB_BOOL;
    }

    // GxB_fprint (A, GxB_COMPLETE, stdout) ;

    // get the type and size of A
    LAGRAPH_OK (GxB_Matrix_type  (&type,  A));
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows, A));
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols, A));

    // C = boolean matrix, the same size as A
    // T = GrB_BOOL by default
    LAGr_Matrix_new (C, T, nrows, ncols);

    // C = spones (A), typecasting to bool
    if (type == LAGraph_ComplexFP64)
    {
        // the LAGraph_TRUE_BOOL_ComplexFP64 operator returns boolean true,
        // and has an input of type LAGraph_ComplexFP64 (which it ignores).
        LAGr_apply(*C, NULL, NULL, LAGraph_TRUE_BOOL_ComplexFP64, A, NULL);
    }
    else
    {
        // this works for all built-in types, which are first typecasted to
        // boolean ... and then ignored by the operator anyway ...
        LAGr_apply(*C, NULL, NULL, LAGraph_TRUE_BOOL, A, NULL);
    }

    // free workspace and return result
    return (GrB_SUCCESS);
}


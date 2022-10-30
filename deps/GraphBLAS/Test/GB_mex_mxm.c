//------------------------------------------------------------------------------
// GB_mex_mxm: C<M> = accum(C,A*B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"
#include "GB_stringify.h"
#include "GB_Descriptor_get.h"

#define USAGE "C = GB_mex_mxm (C, M, accum, semiring, A, B, desc, macrofy)"

#define FREE_ALL                                    \
{                                                   \
    GrB_Matrix_free_(&A) ;                          \
    GrB_Matrix_free_(&B) ;                          \
    GrB_Matrix_free_(&C) ;                          \
    GrB_Matrix_free_(&M) ;                          \
    if (semiring != Complex_plus_times)             \
    {                                               \
        if (semiring != NULL)                       \
        {                                           \
            GrB_Monoid_free_(&(semiring->add)) ;    \
        }                                           \
        GrB_Semiring_free_(&semiring) ;             \
    }                                               \
    GrB_Descriptor_free_(&desc) ;                   \
    GB_mx_put_global (true) ;                       \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;
    GrB_Matrix M = NULL ;
    GrB_Semiring semiring = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 6 || nargin > 8)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;
    #define FREE_DEEP_COPY GrB_Matrix_free_(&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }

    // get M (shallow copy)
    M = GB_mx_mxArray_to_Matrix (pargin [1], "M", false, false) ;
    if (M == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("M failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [4], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get B (shallow copy)
    B = GB_mx_mxArray_to_Matrix (pargin [5], "B input", false, true) ;
    if (B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("B failed") ;
    }

    bool user_complex = (Complex != GxB_FC64) && (C->type == Complex) ;

    // get semiring
    if (!GB_mx_mxArray_to_Semiring (&semiring, pargin [3], "semiring",  
        C->type, user_complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("semiring failed") ;
    }

    // get accum, if present
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        C->type, user_complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (6), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // create the GB_mxm_scode.h file, if requested
    if (nargin == 8)
    {

        GrB_Desc_Value Mask_desc = GxB_DEFAULT ;
        if (desc != NULL) Mask_desc = desc->mask ;
        bool Mask_comp = (Mask_desc == GrB_COMP)
                      || (Mask_desc == GrB_COMP + GrB_STRUCTURE) ;
        bool Mask_struct = (Mask_desc == GrB_STRUCTURE)
                        || (Mask_desc == GrB_STRUCTURE + GrB_COMP) ;
        bool flipxy = (bool) mxGetScalar (pargin [7]) ;

        bool C_iso = C->iso ;
        int C_sparsity = GB_sparsity (C) ;
        GrB_Type ctype = C->type ;

        GB_debugify_mxm (C_iso, C_sparsity, ctype, M,
            Mask_struct, Mask_comp, semiring, flipxy, A, B) ;
    }

    // C<M> = accum(C,A*B)
    METHOD (GrB_mxm (C, M, accum, semiring, A, B, desc)) ;

    // return C as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output from GrB_mxm", true) ;
    FREE_ALL ;
}


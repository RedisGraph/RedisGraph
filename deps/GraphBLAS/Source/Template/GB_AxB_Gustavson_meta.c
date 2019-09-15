//------------------------------------------------------------------------------
// GB_AxB_Gustavson_meta: C=A*B and C<M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{
    // Ax and Bx are not used if the multiply operator is SECOND or FIRST,
    // respectively.
    #include "GB_unused.h"

    const GB_ATYPE *restrict Ax = A_is_pattern ? NULL : A->x ;
    const GB_BTYPE *restrict Bx = B_is_pattern ? NULL : B->x ;

    bool A_is_hyper = GB_IS_HYPER (A) ;
    bool C_is_hyper = GB_IS_HYPER (C) ;
    bool M_is_hyper = GB_IS_HYPER (M) ;
    if (A_is_hyper || GB_IS_HYPER (B) || C_is_hyper || M_is_hyper)
    {
        #define GB_HYPER_CASE
        if (M != NULL)
        { 
            // C<M> = A*B where M is pattern of C
            #include "GB_AxB_Gustavson_mask.c"
        }
        else
        { 
            // C = A*B with pattern of C as defined on input
            #include "GB_AxB_Gustavson_nomask.c"
        }
        #undef GB_HYPER_CASE
    }
    else
    {
        if (M != NULL)
        { 
            // C<M> = A*B where M is pattern of C
            #include "GB_AxB_Gustavson_mask.c"
        }
        else
        { 
            // C = A*B with pattern of C as defined on input
            #include "GB_AxB_Gustavson_nomask.c"
        }
    }
}

//------------------------------------------------------------------------------
// GB_AxB_Gustavson_meta: C=A*B and C<M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{
    const GB_atype *restrict Ax = A->x ;
    const GB_btype *restrict Bx = B->x ;

    bool A_is_hyper = GB_IS_HYPER (A) ;
    if (A_is_hyper || GB_IS_HYPER (B) || GB_IS_HYPER (C) || GB_IS_HYPER (M))
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

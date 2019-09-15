//------------------------------------------------------------------------------
// GB_AxB_heap_meta: compute C<M>=A*B or C=A*B using a heap-based method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{

    const GB_ATYPE *restrict Ax = A_is_pattern ? NULL : A->x ;
    const GB_BTYPE *restrict Bx = B_is_pattern ? NULL : B->x ;

    if (M != NULL)
    { 
        // C<M> = A*B via a heap
        #define GB_MASK_CASE
        #include "GB_AxB_heap_mask.c"
        #undef GB_MASK_CASE
    }
    else
    { 
        // C = A*B via the heap
        #include "GB_AxB_heap_mask.c"
    }
}


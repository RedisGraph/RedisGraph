//------------------------------------------------------------------------------
// GB_AxB_heap_flipxy: compute C<M>=A*B or C=A*B using a heap-based method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

    #if GB_HANDLE_FLIPXY
    if (flipxy)
    { 
        // A and B have been swapped on input, and the mult(x,y) operator is
        // not commutative
        #define GB_atype GB_YTYPE
        #define GB_btype GB_XTYPE
        #define GB_MULTIPLY(z,x,y) GB_MULTOP(z,y,x)
        #include "GB_AxB_heap_meta.c"
        #undef GB_MULTIPLY
        #undef GB_atype
        #undef GB_btype
    }
    else
    #endif
    { 
        // A and B have not not been swapped on input, or the mult(x,y)
        // operator is commutative and thus the flipxy doesn't matter
        #define GB_atype GB_XTYPE
        #define GB_btype GB_YTYPE
        #define GB_MULTIPLY(z,x,y) GB_MULTOP(z,x,y)
        #include "GB_AxB_heap_meta.c"
        #undef GB_MULTIPLY
        #undef GB_atype
        #undef GB_btype
    }


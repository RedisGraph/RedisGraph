//SPDX-License-Identifier: Apache-2.0

#define chunksize 128 

#define ASSERT
//#define GB_GETA( aval, ax, p) aval = (T_Z)ax[ ( p )]
//#define GB_GETB( bval, bx, p) bval = (T_Z)bx[ ( p )]
#define GB_ADD_F( f , s)  f = GB_ADD ( f, s ) 
#define GB_C_MULT( c, a, b)  c = GB_MULT( (a), (b) )
#define GB_MULTADD( c, a ,b ) GB_ADD_F( (c), GB_MULT( (a),(b) ) )
#define GB_DOT_TERMINAL ( c )   
//# if ( c == TERMINAL_VALUE) break;

#include "GB_imin.h"
#include "GB_zombie.h"
#include "GB_nnz.h"
#include "GB_partition.h"
#include "GB_binary_search.h"
#include "GB_search_for_vector_template.c"

#undef GB_DOT_MERGE
// cij += A(k,i) * B(k,j), for merge operation
#define GB_DOT_MERGE                                                \
{                                                                   \
    GB_GETA ( aki= (T_Z)Ax[pA]) ;       /* aki = A(k,i) */          \
    GB_GETB ( bkj= (T_Z)Bx[pB]) ;       /* bkj = B(k,j) */          \
    if (cij_exists)                                                 \
    {                                                               \
        GB_MULTADD (cij, aki, bkj) ;    /* cij += aki * bkj */      \
    }                                                               \
    else                                                            \
    {                                                               \
        /* cij = A(k,i) * B(k,j), and add to the pattern    */      \
        cij_exists = true ;                                         \
        GB_C_MULT (cij, aki, bkj) ;     /* cij  = aki * bkj */      \
    }                                                               \
}


typedef void (*GxB_binary_function) (void *, const void *, const void *) ;

#include "GB_opaque.h"

typedef enum
{
    // for all GrB_Descriptor fields:
    GxB_DEFAULT = 0,    // default behavior of the method

    // for GrB_OUTP only:
    GrB_REPLACE = 1,    // clear the output before assigning new values to it

    // for GrB_MASK only:
    GrB_COMP = 2,       // use the structural complement of the input
    GrB_SCMP = 2,       // same as GrB_COMP (deprecated; use GrB_COMP instead)
    GrB_STRUCTURE = 4,  // use the only pattern of the mask, not its values

    // for GrB_INP0 and GrB_INP1 only:
    GrB_TRAN = 3,       // use the transpose of the input

    // for GxB_GPU_CONTROL only:
    GxB_GPU_ALWAYS  = 4,
    GxB_GPU_NEVER   = 5,

    // for GxB_AxB_METHOD only:
    GxB_AxB_GUSTAVSON = 1001,   // gather-scatter saxpy method
    GxB_AxB_DOT       = 1003,   // dot product
    GxB_AxB_HASH      = 1004,   // hash-based saxpy method
    GxB_AxB_SAXPY     = 1005    // saxpy method (any kind)
}
GrB_Desc_Value ;


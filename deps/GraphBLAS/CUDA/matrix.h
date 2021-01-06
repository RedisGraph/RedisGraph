//SPDX-License-Identifier: Apache-2.0

#define chunksize 128 

#define ASSERT
#define GB_RESTRICT __restrict__
//#define GB_GETA( aval, ax, p) aval = (T_Z)ax[ ( p )]
//#define GB_GETB( bval, bx, p) bval = (T_Z)bx[ ( p )]
#define GB_ADD_F( f , s)  f = GB_ADD ( f, s ) 
#define GB_C_MULT( c, a, b)  c = GB_MULT( (a), (b) )
#define GB_MULTADD( c, a ,b ) GB_ADD_F( (c), GB_MULT( (a),(b) ) )
#define GB_DOT_TERMINAL ( c )   
//# if ( c == TERMINAL_VALUE) break;

// TODO: make sure this works:
#include "GB_imin.h"
#if 0
#define GB_IMIN( A, B) ( (A) < (B) ) ?  (A) : (B)
#define GB_IMAX( A, B) ( (A) > (B) ) ?  (A) : (B)
#endif

// TODO: make sure this works:
#include "GB_zombie.h"
#if 0
#define GB_FLIP(i)             (-(i)-2)
#define GB_IS_FLIPPED(i)       ((i) < 0)
#define GB_IS_ZOMBIE(i)        ((i) < 0)
#define GB_IS_NOT_FLIPPED(i)   ((i) >= 0)
#define GB_IS_NOT_ZOMBIE(i)    ((i) >= 0)
#define GB_UNFLIP(i)           (((i) < 0) ? GB_FLIP(i) : (i))
#endif

// TODO: make sure this works:
#include "GB_nnz.h"
#if 0
#define GB_NNZ(A) (((A)->nzmax > 0) ? ((A)->p [(A)->nvec] - (A)->p [0]) : 0 )
#endif

// TODO: make sure this works:
#include "GB_partition.h"
#if 0
// GB_PART and GB_PARTITION:  divide the index range 0:n-1 uniformly
// for nthreads.  GB_PART(tid,n,nthreads) is the first index for thread tid.
#define GB_PART(tid,n,nthreads)  \
    GB_IMIN( ((tid) * ((double)(n))/((double)(nthreads))), n)

// thread tid will operate on the range k1:(k2-1)
#define GB_PARTITION(k1,k2,n,tid,nthreads)                                  \
    k1 = ((tid) ==  0          ) ?  0  : GB_PART ((tid),  n, nthreads) ;    \
    k2 = ((tid) == (nthreads)-1) ? (n) : GB_PART ((tid)+1,n, nthreads) ;
#endif

// TODO: make sure this works:
#include "GB_binary_search.h"
#if 0
//------------------------------------------------------------------------------
// GB_BINARY_SEARCH
//------------------------------------------------------------------------------

// search for integer i in the list X [pleft...pright]; no zombies.
// The list X [pleft ... pright] is in ascending order.  It may have
// duplicates.

#define GB_TRIM_BINARY_SEARCH(i,X,pleft,pright)                             \
{                                                                           \
    /* binary search of X [pleft ... pright] for integer i */               \
    while (pleft < pright)                                                  \
    {                                                                       \
        int64_t pmiddle = (pleft + pright) / 2 ;                            \
        if (X [pmiddle] < i)                                                \
        {                                                                   \
            /* if in the list, it appears in [pmiddle+1..pright] */         \
            pleft = pmiddle + 1 ;                                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* if in the list, it appears in [pleft..pmiddle] */            \
            pright = pmiddle ;                                              \
        }                                                                   \
    }                                                                       \
    /* binary search is narrowed down to a single item */                   \
    /* or it has found the list is empty */                                 \
    /*ASSERT (pleft == pright || pleft == pright + 1) ;*/                   \
}

//------------------------------------------------------------------------------
// GB_SPLIT_BINARY_SEARCH: binary search, and then partition the list
//------------------------------------------------------------------------------

// If found is true then X [pleft] == i.  If duplicates appear then X [pleft]
//    is any one of the entries with value i in the list.
// If found is false then
//    X [original_pleft ... pleft-1] < i and
//    X [pleft ... original_pright] > i holds, and pleft-1 == pright
// If X has no duplicates, then whether or not i is found,
//    X [original_pleft ... pleft-1] < i and
//    X [pleft ... original_pright] >= i holds.
#define GB_SPLIT_BINARY_SEARCH(i,X,pleft,pright,found)                      \
{                                                                           \
    GB_BINARY_SEARCH (i, X, pleft, pright, found)                           \
    if (!found && (pleft == pright))                                        \
    {                                                                       \
        if (i > X [pleft])                                                  \
        {                                                                   \
            pleft++ ;                                                       \
        }                                                                   \
        else                                                                \
        {                                                                   \
            pright++ ;                                                      \
        }                                                                   \
    }                                                                       \
}


//------------------------------------------------------------------------------
// GB_BINARY_SEARCH: binary search and check if found
//------------------------------------------------------------------------------

// If found is true then X [pleft == pright] == i.  If duplicates appear then
// X [pleft] is any one of the entries with value i in the list.
// If found is false then
//    X [original_pleft ... pleft-1] < i and
//    X [pleft+1 ... original_pright] > i holds.
// The value X [pleft] may be either < or > i.
#define GB_BINARY_SEARCH(i,X,pleft,pright,found)                            \
{                                                                           \
    GB_TRIM_BINARY_SEARCH (i, X, pleft, pright) ;                           \
    found = (pleft == pright && X [pleft] == i) ;                           \
}

#include "../Source/Template/GB_lookup_template.c"
#endif

#include "../Source/Template/GB_search_for_vector_template.c"

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

//------------------------------------------------------------------------------
// FIXED: by Tim but not tested:
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


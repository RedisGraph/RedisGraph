//------------------------------------------------------------------------------
// GB_AxB_heap: compute C<M> = A*B using a heap-based method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_heap.h"
#ifndef GBCOMPACT
#include "GB_AxB__semirings.h"
#endif

GrB_Info GB_AxB_heap                // C<M>=A*B or C=A*B using a heap
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix for C<M>=A*B
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const int64_t bjnz_max,         // max # entries in any vector of B
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_NULL (GB_check (M, "M for heap A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for heap A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for heap A*B", GB0)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for heap A*B", GB0)) ;
    ASSERT (A->vdim == B->vlen) ;

    if (flipxy)
    { 
        // z=fmult(b,a) will be computed
        ASSERT (GB_Type_compatible (A->type, semiring->multiply->ytype)) ;
        ASSERT (GB_Type_compatible (B->type, semiring->multiply->xtype)) ;
    }
    else
    { 
        // z=fmult(a,b) will be computed
        ASSERT (GB_Type_compatible (A->type, semiring->multiply->xtype)) ;
        ASSERT (GB_Type_compatible (B->type, semiring->multiply->ytype)) ;
    }
    ASSERT (semiring->multiply->ztype == semiring->add->op->ztype) ;

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    // int64_t List [0..bjnz_max-1] ;
    // GB_pointer_pair pA_pair [0..bjnz_max-1] ;
    // GB_Element Heap [0..bjnz_max] ;              // Heap [0] unused

    int64_t *List = NULL ;
    GB_MALLOC_MEMORY (List, bjnz_max, sizeof (int64_t)) ;

    GB_pointer_pair *pA_pair = NULL ;
    GB_MALLOC_MEMORY (pA_pair, bjnz_max, sizeof (GB_pointer_pair)) ;

    GB_Element *Heap = NULL ;
    GB_MALLOC_MEMORY (Heap, bjnz_max + 1, sizeof (GB_Element)) ;

    #define GB_HEAP_FREE_WORK                                           \
    {                                                                   \
        GB_FREE_MEMORY (List, bjnz_max, sizeof (int64_t)) ;             \
        GB_FREE_MEMORY (pA_pair, bjnz_max, sizeof (GB_pointer_pair)) ;  \
        GB_FREE_MEMORY (Heap, bjnz_max + 1, sizeof (GB_Element)) ;      \
    }

    if (List == NULL || pA_pair == NULL || Heap == NULL)
    { 
        // out of memory
        double memory = GBYTES (bjnz_max, sizeof (int64_t)) +
                        GBYTES (bjnz_max, sizeof (GB_pointer_pair)) +
                        GBYTES (bjnz_max + 1, sizeof (GB_Element)) ;
        GB_HEAP_FREE_WORK ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // esimate nnz(C) and allocate C
    //--------------------------------------------------------------------------

    int64_t cvlen = A->vlen ;
    int64_t cvdim = B->vdim ;
    GrB_Type ctype = semiring->add->op->ztype ;

    GrB_Info info = GB_AxB_alloc (Chandle, ctype, cvlen, cvdim, M, A, B, true,
        15 + GB_NNZ (A) + GB_NNZ (B), Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_HEAP_FREE_WORK ;
        return (info) ;
    }

    GrB_Matrix C = (*Chandle) ;

    //--------------------------------------------------------------------------
    // C = A*B with a heap and builtin semiring
    //--------------------------------------------------------------------------

    bool done = false ;

#ifndef GBCOMPACT

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_AheapB(add,mult,xyname) GB_AheapB_ ## add ## mult ## xyname

    #define GB_AxB_WORKER(add,mult,xyname)                          \
    {                                                               \
        info = GB_AheapB (add,mult,xyname) (Chandle, M, A, B,       \
            flipxy, List, pA_pair, Heap, bjnz_max, Context) ;       \
        done = true ;                                               \
    }                                                               \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xycode, zcode ;

    if (GB_semiring_builtin (A, B, semiring, flipxy,
        &mult_opcode, &add_opcode, &xycode, &zcode))
    { 
        #include "GB_AxB_factory.c"
    }

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_HEAP_FREE_WORK ;
        return (info) ;
    }

#endif

    //--------------------------------------------------------------------------
    // user semirings created at compile time
    //--------------------------------------------------------------------------

    if (semiring->object_kind == GB_USER_COMPILED)
    {

        // determine the required type of A and B for the user semiring
        GrB_Type atype_required, btype_required ;

        if (flipxy)
        { 
            // A is passed as y, and B as x, in z = mult(x,y)
            atype_required = semiring->multiply->ytype ;
            btype_required = semiring->multiply->xtype ;
        }
        else
        { 
            // A is passed as x, and B as y, in z = mult(x,y)
            atype_required = semiring->multiply->xtype ;
            btype_required = semiring->multiply->ytype ;
        }

        if (A->type == atype_required && B->type == btype_required)
        {
            info = GB_AxB_user (GxB_AxB_HEAP, semiring, Chandle, M, A, B,
                flipxy, List, pA_pair, Heap, bjnz_max, NULL, Context) ;
            done = true ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory or invalid semiring
                GB_HEAP_FREE_WORK ;
                return (info) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // C = A*B, with a heap, and typecasting
    //--------------------------------------------------------------------------

    if (!done)
    {

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A, B, C, and M
        //----------------------------------------------------------------------

        // get the semiring operators
        GrB_BinaryOp multiply = semiring->multiply ;
        GrB_Monoid add = semiring->add ;

        GxB_binary_function fmult = multiply->function ;
        GxB_binary_function fadd  = add->op->function ;

        size_t csize = C->type->size ;
        size_t asize = A->type->size ;
        size_t bsize = B->type->size ;

        size_t xsize = multiply->xtype->size ;
        size_t ysize = multiply->ytype->size ;

        // scalar workspace
        // flipxy false: aik = (xtype) A(i,k) and bkj = (ytype) B(k,j)
        // flipxy true:  aik = (ytype) A(i,k) and bkj = (xtype) B(k,j)
        char aik [flipxy ? ysize : xsize] ;
        char bkj [flipxy ? xsize : ysize] ;
        char zwork [csize] ;

        const GB_void *restrict Ax = A->x ;
        const GB_void *restrict Bx = B->x ;
        GB_void *restrict Cx = C->x ;
        GB_void *cij = Cx ;        // advances through each entry of C

        GB_void *identity = add->identity ;

        GB_cast_function cast_A, cast_B ;
        if (flipxy)
        { 
            // A is typecasted to y, and B is typecasted to x
            cast_A = GB_cast_factory (multiply->ytype->code, A->type->code) ;
            cast_B = GB_cast_factory (multiply->xtype->code, B->type->code) ;
        }
        else
        { 
            // A is typecasted to x, and B is typecasted to y
            cast_A = GB_cast_factory (multiply->xtype->code, A->type->code) ;
            cast_B = GB_cast_factory (multiply->ytype->code, B->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = A*B via the heap, function pointers, and typecasting
        //----------------------------------------------------------------------

        // bkj = B(k,j), located in Bx [pB]
        #define GB_CIJ_GETB(pB)                                         \
        {                                                               \
            cast_B (bkj, Bx +((pB)*bsize), bsize) ;                     \
        }

        #define GB_MULTOP(z,x,y) fmult (z, x, y) ;

        // C(i,j) = A(i,k) * bkj
        #define GB_CIJ_MULT(pA)                                         \
        {                                                               \
            /* aik = A(i,k), located in Ax [pA] */                      \
            cast_A (aik, Ax +((pA)*asize), asize) ;                     \
            /* cij = aik*bkj, reversing them if flipxy is true */       \
            GB_MULTIPLY (cij, aik, bkj) ;                               \
        }

        // C(i,j) += A(i,k) * B(k,j)
        #define GB_CIJ_MULTADD(pA,pB)                                   \
        {                                                               \
            /* aik = A(i,k), located in Ax [pA] */                      \
            cast_A (aik, Ax +((pA)*asize), asize) ;                     \
            /* bkj = B(k,j), located in Bx [pB] */                      \
            GB_CIJ_GETB (pB) ;                                          \
            /* zwork = aik*bkj, reversing them if flipxy is true */     \
            GB_MULTIPLY (zwork, aik, bkj) ;                             \
            /* cij = cij + zwork */                                     \
            fadd (cij, cij, zwork) ; /* (z x alias) */                  \
        }

        // C->x has moved so the pointer to cij needs to be recomputed
        #define GB_CIJ_REACQUIRE                                        \
        {                                                               \
            cij = Cx + cnz * csize ;                                    \
        }

        // cij = identity
        #define GB_CIJ_CLEAR  memcpy (cij, identity, csize) ;

        // save the value of C(i,j) by advancing the cij pointer to next value
        #define GB_CIJ_SAVE   cij += csize ;

        #define GB_HANDLE_FLIPXY true
        #define GB_XTYPE GB_void
        #define GB_YTYPE GB_void
        #include "GB_AxB_heap_flipxy.c"
    }

    //--------------------------------------------------------------------------
    // trim the size of C: this cannot fail
    //--------------------------------------------------------------------------

    GB_HEAP_FREE_WORK ;
    info = GB_ix_realloc (C, GB_NNZ (C), true, Context) ;
    ASSERT (info == GrB_SUCCESS) ;
    ASSERT_OK (GB_check (C, "heap: C = A*B output", GB0)) ;
    ASSERT (*Chandle == C) ;
    return (GrB_SUCCESS) ;
}


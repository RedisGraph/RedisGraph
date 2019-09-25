//------------------------------------------------------------------------------
// GB_AxB_heap: compute C<M> = A*B using a heap-based method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.

#include "GB_mxm.h"
#include "GB_heap.h"
#include "GB_jappend.h"
#include "GB_bracket.h"
#include "GB_iterator.h"
#ifndef GBCOMPACT
#include "GB_AxB__include.h"
#endif

GrB_Info GB_AxB_heap                // C<M>=A*B or C=A*B using a heap
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // mask matrix for C<M>=A*B
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const int64_t bjnz_max          // max # entries in any vector of B
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    // only one thread does this entire function
    GB_Context Context = NULL ;
    #endif
    ASSERT (Chandle != NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M_in, "M_in for heap A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for heap A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for heap A*B", GB0)) ;
    ASSERT (!GB_PENDING (M_in)) ; ASSERT (!GB_ZOMBIES (M_in)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for heap A*B", GB0)) ;
    ASSERT (A->vdim == B->vlen) ;
    ASSERT (mask_applied != NULL) ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;

    bool op_is_first  = mult->opcode == GB_FIRST_opcode ;
    bool op_is_second = mult->opcode == GB_SECOND_opcode ;
    bool A_is_pattern = false ;
    bool B_is_pattern = false ;

    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        A_is_pattern = op_is_first  ;
        B_is_pattern = op_is_second ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->ytype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->xtype))) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        A_is_pattern = op_is_second ;
        B_is_pattern = op_is_first  ;
        ASSERT (GB_IMPLIES (!A_is_pattern,
            GB_Type_compatible (A->type, mult->xtype))) ;
        ASSERT (GB_IMPLIES (!B_is_pattern,
            GB_Type_compatible (B->type, mult->ytype))) ;
    }

    (*Chandle) = NULL ;

    // the heap method does not handle a complemented mask
    GrB_Matrix M = (Mask_comp ? NULL : M_in) ;

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
        GB_HEAP_FREE_WORK ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // esimate nnz(C) and allocate C (both pattern and values)
    //--------------------------------------------------------------------------

    int64_t cvlen = A->vlen ;
    int64_t cvdim = B->vdim ;
    GrB_Type ctype = semiring->add->op->ztype ;

    GrB_Info info = GB_AxB_alloc (Chandle, ctype, cvlen, cvdim, M, A, B, true,
        15) ;

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

    #define GB_AxB_WORKER(add,mult,xyname)                  \
    {                                                       \
        info = GB_AheapB (add,mult,xyname) (Chandle, M,     \
            A, A_is_pattern, B, B_is_pattern,               \
            List, pA_pair, Heap, bjnz_max) ;                \
        done = (info != GrB_NO_VALUE) ;                     \
    }                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xycode, zcode ;

    if (GB_AxB_semiring_builtin (A, A_is_pattern, B, B_is_pattern, semiring,
        flipxy, &mult_opcode, &add_opcode, &xycode, &zcode))
    { 
        #include "GB_AxB_factory.c"
    }

    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))
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
            atype_required = mult->ytype ;
            btype_required = mult->xtype ;
        }
        else
        { 
            // A is passed as x, and B as y, in z = mult(x,y)
            atype_required = mult->xtype ;
            btype_required = mult->ytype ;
        }

        if (A->type == atype_required && B->type == btype_required)
        {
            info = GB_AxB_user (GxB_AxB_HEAP, semiring, Chandle, M, A, B,
                flipxy,
                /* heap: */ List, pA_pair, Heap, bjnz_max,
                /* Gustavson: */ NULL,
                /* dot: */ NULL, 1, 1, 1, NULL,
                /* dot3: */ NULL, 0) ;
            done = true ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
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

        GxB_binary_function fmult = mult->function ;
        GxB_binary_function fadd  = add->op->function ;

        size_t csize = C->type->size ;
        size_t asize = A_is_pattern ? 0 : A->type->size ;
        size_t bsize = B_is_pattern ? 0 : B->type->size ;

        size_t xsize = mult->xtype->size ;
        size_t ysize = mult->ytype->size ;

        // scalar workspace
        // flipxy false: aik = (xtype) A(i,k) and bkj = (ytype) B(k,j)
        // flipxy true:  aik = (ytype) A(i,k) and bkj = (xtype) B(k,j)
        char aik [flipxy ? ysize : xsize] ;
        char bkj [flipxy ? xsize : ysize] ;
        char t [csize] ;

        GB_void *restrict Cx = C->x ;
        GB_void *cij = Cx ;        // advances through each entry of C

        // GB_void *identity = add->identity ;

        GB_cast_function cast_A, cast_B ;
        if (flipxy)
        { 
            // A is typecasted to y, and B is typecasted to x
            cast_A = A_is_pattern ? NULL : 
                     GB_cast_factory (mult->ytype->code, A->type->code) ;
            cast_B = B_is_pattern ? NULL : 
                     GB_cast_factory (mult->xtype->code, B->type->code) ;
        }
        else
        { 
            // A is typecasted to x, and B is typecasted to y
            cast_A = A_is_pattern ? NULL :
                     GB_cast_factory (mult->xtype->code, A->type->code) ;
            cast_B = B_is_pattern ? NULL :
                     GB_cast_factory (mult->ytype->code, B->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = A*B via the heap, function pointers, and typecasting
        //----------------------------------------------------------------------

        // aik = A(i,k), of size asize
        #define GB_GETA(aik,Ax,pA)                                          \
            if (!A_is_pattern) cast_A (aik, Ax +((pA)*asize), asize) ;

        // bkj = B(k,j), of size bsize
        #define GB_GETB(bkj,Bx,pB)                                          \
            if (!B_is_pattern) cast_B (bkj, Bx +((pB)*bsize), bsize) ;

        // C(i,j) = A(i,k) * B(k,j)
        #define GB_MULT(cij, aik, bkj)                                      \
            GB_MULTIPLY (cij, aik, bkj) ;                                   \

        // C(i,j) += A(i,k) * B(k,j)
        #define GB_MULTADD(cij, aik, bkj)                                   \
            GB_MULTIPLY (t, aik, bkj) ;                                     \
            fadd (cij, cij, t) ;

        // C->x or cnz has moved so the pointer to cij needs to be recomputed
        #define GB_CIJ_REACQUIRE(cij, cnz)  cij = Cx + cnz * csize ;

        // save the value of C(i,j) by advancing cij pointer to next value
        #define GB_CIJ_SAVE(cij,p)          cij += csize ;

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void

        if (flipxy)
        { 
            #define GB_MULTIPLY(z,x,y) fmult (z,y,x)
            #include "GB_AxB_heap_meta.c"
            #undef GB_MULTIPLY
        }
        else
        { 
            #define GB_MULTIPLY(z,x,y) fmult (z,x,y)
            #include "GB_AxB_heap_meta.c"
            #undef GB_MULTIPLY
        }
    }

    //--------------------------------------------------------------------------
    // trim the size of C: this cannot fail
    //--------------------------------------------------------------------------

    GB_HEAP_FREE_WORK ;
    info = GB_ix_realloc (C, GB_NNZ (C), true, NULL) ;
    ASSERT (info == GrB_SUCCESS) ;
    ASSERT_OK (GB_check (C, "heap: C = A*B output", GB0)) ;
    ASSERT (*Chandle == C) ;
    (*mask_applied) = (M != NULL) ;
    return (GrB_SUCCESS) ;
}


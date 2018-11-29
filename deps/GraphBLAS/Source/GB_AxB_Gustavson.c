//------------------------------------------------------------------------------
// GB_AxB_Gustavson: C=A*B or C<M>=A*B, gather/scatter-based saxpy method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This method is agnostic to the CSR/CSC format.  The format of C is set
// to CSC but this is a placeholder that will be changed in GB_AxB_meta.

// This function is called only by GB_AxB_meta.

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_heap.h"
#include "GB_AxB__semirings.h"
#endif

// C=A*B is successful, just free temporary matrices
#define GB_GUS_FREE_WORK                    \
{                                           \
    GB_MATRIX_FREE (&A2) ;                  \
    GB_MATRIX_FREE (&B2) ;                  \
}

// C=A*B failed, free everything, including C, and the Sauna
#define GB_FREE_ALL                         \
{                                           \
    GB_GUS_FREE_WORK ;                      \
    GB_MATRIX_FREE (Chandle) ;              \
    GB_Sauna_free (Sauna_Handle) ;          \
}

#define GB_OK(method)                       \
{                                           \
    info = method ;                         \
    if (info != GrB_SUCCESS)                \
    {                                       \
        GB_FREE_ALL ;                       \
        return (info) ;                     \
    }                                       \
}

GrB_Info GB_AxB_Gustavson           // C=A*B or C<M>=A*B, Gustavson's method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // optional matrix
    const GrB_Matrix A_in,          // input matrix A
    const GrB_Matrix B_in,          // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Sauna *Sauna_Handle,         // handle to sparse accumulator
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_NULL (GB_check (M, "M for numeric C<M>=A*B", GB0)) ;
    ASSERT_OK (GB_check (A_in, "A for Gustavson C=A*B", GB0)) ;
    ASSERT_OK (GB_check (B_in, "B for Gustavson C=A*B", GB0)) ;
    ASSERT (!GB_PENDING (M))    ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A_in)) ; ASSERT (!GB_ZOMBIES (A_in)) ;
    ASSERT (!GB_PENDING (B_in)) ; ASSERT (!GB_ZOMBIES (B_in)) ;
    ASSERT (A_in->vdim == B_in->vlen) ;
    ASSERT_OK (GB_check (semiring, "semiring for numeric A*B", GB0)) ;
    ASSERT (Sauna_Handle != NULL) ;

    //--------------------------------------------------------------------------
    // determine size and hypersparsity of C
    //--------------------------------------------------------------------------

    GrB_Info info ;

    (*Chandle) = NULL ;

    GrB_Matrix A2 = NULL ;
    GrB_Matrix B2 = NULL ;
    GrB_Matrix A = A_in ;
    GrB_Matrix B = B_in ;

    int64_t cvlen = A->vlen ;
    int64_t cvdim = B->vdim ;

    //--------------------------------------------------------------------------
    // get the semiring operators
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;

    // flipxy: if true, then compute z = fmult (B(k,j), A(i,k)) instead of the
    // usual z = fmult (A(i,k), B(k,j)), since A and B have been swapped on
    // input.

    // these conditions have already been checked in the caller
    if (flipxy)
    { 
        // z = fmult (b,a) will be computed
        ASSERT (GB_Type_compatible (A->type, mult->ytype)) ;
        ASSERT (GB_Type_compatible (B->type, mult->xtype)) ;
    }
    else
    { 
        // z = fmult (a,b) will be computed
        ASSERT (GB_Type_compatible (A->type, mult->xtype)) ;
        ASSERT (GB_Type_compatible (B->type, mult->ytype)) ;
    }

    // these asserts hold for any valid semiring:
    ASSERT (mult->ztype == add->op->ztype) ;
    ASSERT (add->op->ztype == add->op->xtype) ;
    ASSERT (add->op->ztype == add->op->ytype) ;

    size_t zsize = mult->ztype->size ;

    //--------------------------------------------------------------------------
    // allocate the Sauna
    //--------------------------------------------------------------------------

    GB_Sauna Sauna = (*Sauna_Handle) ;

    if (Sauna == NULL || Sauna->Sauna_n < cvlen || Sauna->Sauna_size < zsize)
    { 
        // the Sauna either does not exist, or is too small
        GB_Sauna_free (Sauna_Handle) ;
        GB_OK (GB_Sauna_alloc (Sauna_Handle, cvlen, zsize, Context)) ;
    }

    Sauna = (*Sauna_Handle) ;

    int64_t *restrict Sauna_Mark = Sauna->Sauna_Mark ;

    // Sauna_Mark [0..cvlen-1] < hiwater holds
    ASSERT_SAUNA_IS_RESET ;

    //--------------------------------------------------------------------------
    // estimate nnz(C) and allocate C (just the pattern)
    //--------------------------------------------------------------------------

    GB_OK (GB_AxB_alloc (Chandle, GrB_BOOL, cvlen, cvdim, M, A, B, false,
        cvlen + GB_NNZ (A) + GB_NNZ (B), Context)) ;

    GrB_Matrix C = (*Chandle) ;
    ASSERT (C != NULL) ;
    ASSERT (C->x == NULL) ;

    //==========================================================================
    // symbolic analysis when no mask is present
    //==========================================================================

    if (M == NULL)
    {
        bool A_is_hyper = GB_IS_HYPER (A) ;
        if (A_is_hyper || GB_IS_HYPER (B) || GB_IS_HYPER (C))
        { 
            // symbolic analysis when one or more matrix is hypersparse
            #define GB_HYPER_CASE
            #include "GB_AxB_Gustavson_symbolic.c"
            #undef GB_HYPER_CASE
        }
        else
        { 
            // symbolic analysis when no matrix is hypersparse
            #include "GB_AxB_Gustavson_symbolic.c"
        }
    }

    //==========================================================================
    // numerical phase
    //==========================================================================

    //--------------------------------------------------------------------------
    // allocate C->x
    //--------------------------------------------------------------------------

    // C has the same type as z for z=fmult(x,y).  The type is also the
    // same as the monoid of the semiring.

    C->type = mult->ztype ;
    C->type_size = zsize ;

    char zwork [zsize] ;

    GB_MALLOC_MEMORY (C->x, C->nzmax, zsize) ;
    if (C->x == NULL)
    { 
        // out of memory
        double memory = GBYTES (C->nzmax, zsize) ;
        GB_FREE_ALL ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    C->x_shallow = false ;

    // Sauna_Work has size cvlen, each entry of size zsize.  Not initialized.
    GB_void *restrict Sauna_Work = Sauna->Sauna_Work ;

    if (M != NULL)
    { 
        ASSERT (M->vlen == C->vlen && M->vdim == C->vdim) ;
    }

    //--------------------------------------------------------------------------
    // determine the type of A2 and B2, for typecasting
    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------
    // cast A and B to x and y for z=mult(x,y), if needed
    //--------------------------------------------------------------------------

    // Shallow casting creates a new matrix that has shallow pointers to the
    // prior A_in->p and A_in->i, but it constructs a new array A2->x for the
    // numerical values.  The types A_in->type and A2->type can differ, as can
    // A_in->x and A2->x, but all other content is the same.  If the types of
    // A_in and A2 are the same, then A2->x is a shallow copy of A->x and no
    // data is moved.  The CSR/CSC format of A2 and B2 is not relevant, so
    // they are kept the same as A_in and B_in.

    GB_OK (GB_shallow_cast (&A2, atype_required, A_in->is_csc, A_in, Context)) ;
    GB_OK (GB_shallow_cast (&B2, btype_required, B_in->is_csc, B_in, Context)) ;

    A = A2 ;
    B = B2 ;

    // A and B are now the right types for the multiply operator.
    // no further typecasting is needed.
    ASSERT (A->type == atype_required) ;
    ASSERT (B->type == btype_required) ;

    //--------------------------------------------------------------------------
    // compute C = A*B for built-in types and operators
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A->type, "A type for builtin", GB0)) ;
    ASSERT_OK (GB_check (B->type, "B type for builtin", GB0)) ;
    ASSERT_OK (GB_check (C->type, "C type for builtin", GB0)) ;
    ASSERT_OK (GB_check (semiring, "semiring for builtin", GB0)) ;

#ifndef GBCOMPACT

    // If the GB_AxB_Gustavson_builtin function has a worker for the particular
    // semiring, then it does the computation and returns done = true.
    // Otherwise, it returns done as false, and the generic worker below does
    // the work.

    // If GBCOMPACT is enabled at compile-time, then no built-in workers are
    // created, and this function is not used.  All C=A*B computations are done
    // with the generic worker below.

    bool done = false ;
    info = GB_AxB_Gustavson_builtin (C, M, A, B, semiring, flipxy, &done,
        Sauna, Context) ;
    ASSERT (info == GrB_SUCCESS) ;
    if (done)
    { 
        // C = A*B has been done via a hard-coded case
        ASSERT_OK (GB_check (C, "C hard-coded for numeric C=A*B", GB0)) ;
        ASSERT (*Chandle == C) ;
        GB_GUS_FREE_WORK ;
        ASSERT_SAUNA_IS_RESET ;
        return (GrB_SUCCESS) ;
    }

#endif

    //--------------------------------------------------------------------------
    // user semirings created at compile time
    //--------------------------------------------------------------------------

    if (semiring->object_kind == GB_USER_COMPILED)
    { 
        info = GB_AxB_user (GxB_AxB_GUSTAVSON, semiring, Chandle, M, A, B,
            flipxy, NULL, NULL, NULL, 0, Sauna, Context) ;
        GB_GUS_FREE_WORK ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // generic Gustavson C=A*B for any valid semiring, built-in or user-defined
    //--------------------------------------------------------------------------

    // Define operations for GB_AxB_Gustavson_mask and GB_AxB_Gustavson_nomask

    #define GB_IDENTITY \
        identity

    // x [i] = y
    #define GB_COPY_SCALAR_TO_ARRAY(x,i,y,s)        \
        memcpy (x +((i)*s), y, s) ;

    // x = y [i]
    #define GB_COPY_ARRAY_TO_SCALAR(x,y,i,s)        \
        memcpy (x, y +((i)*s), s) ;

    // x [i] = y [j]
    #define GB_COPY_ARRAY_TO_ARRAY(x,i,y,j,s)       \
        memcpy (x +((i)*s), y +((j)*s), s) ;

    #define GB_MULTOP(z,x,y) fmult (z, x, y) ;

    // generic multiply-add operation (with no mask).  fadd: (z x alias)
    #define GB_MULTADD_NOMASK                                           \
    {                                                                   \
        /* Sauna_Work [i] += A(i,k) * B(k,j) */                         \
        GB_MULTIPLY (zwork, Ax +(pA*asize), bkj) ;                      \
        fadd (Sauna_Work +(i*zsize), Sauna_Work +(i*zsize), zwork) ;    \
    }

    // generic multiply-add operation (with mask)
    #define GB_MULTADD_WITH_MASK                                        \
    {                                                                   \
        /* Sauna_Work [i] += A(i,k) * B(k,j) */                         \
        if (mark == hiwater)                                            \
        {                                                               \
            /* first time C(i,j) seen */                                \
            /* Sauna_Work [i] = A(i,k) * bkj */                         \
            GB_MULTIPLY (Sauna_Work +(i*zsize), Ax +(pA*asize), bkj) ;  \
            Sauna_Mark [i] = hiwater + 1 ;                              \
        }                                                               \
        else                                                            \
        {                                                               \
            /* C(i,j) seen before, update it */                         \
            /* Sauna_Work [i] += A(i,k) * B(k,j) */                     \
            GB_MULTADD_NOMASK ;                                         \
        }                                                               \
    }

    // asize is the size of x, or y if flipxy is true, for z=mult(x,y)
    // bsize is the size of y, or x if flipxy is true, for z=mult(x,y)
    size_t asize = atype_required->size ;
    size_t bsize = btype_required->size ;

    char bkj [bsize] ;

    GxB_binary_function fmult = mult->function ;
    GxB_binary_function fadd  = add->op->function ;

    GB_void *restrict identity = add->identity ;

    GB_void *restrict Cx = C->x ;

    #define GB_HANDLE_FLIPXY true
    #define GB_XTYPE GB_void
    #define GB_YTYPE GB_void
    #include "GB_AxB_Gustavson_flipxy.c"

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_GUS_FREE_WORK ;     // free A2 and B2
    ASSERT_SAUNA_IS_RESET ;

    // cannot fail since C->plen is the upper bound: # non-empty columns of B
    ASSERT (info == GrB_SUCCESS) ;
    // if it could fail, do this:
    // GB_OK (info) ;     // check result and return if an error occurred

    ASSERT_OK (GB_check (C, "C output for numeric C=A*B", GB0)) ;
    ASSERT (*Chandle == C) ;
    return (GrB_SUCCESS) ;
}


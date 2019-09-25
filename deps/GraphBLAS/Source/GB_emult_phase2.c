//------------------------------------------------------------------------------
// GB_emult_phase2: C=A.*B or C<M>=A.*+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_emult_phase2 computes C=A.*B or C<M>=A.*B.  It is preceded first by
// GB_emult_phase0, which computes the list of vectors of C to compute (Ch) and
// their location in M, A, and B (C_to_[MAB]).  Next, GB_emult_phase1 counts
// the entries in each vector C(:,j) and computes Cp.

// GB_emult_phase2 computes the pattern and values of each vector of C(:,j),
// fully in parallel.

// C, M, A, and B can be standard sparse or hypersparse, as determined by
// GB_emult_phase0.  If present, the mask M is not complemented.

// This function either frees Cp or transplants it into C, as C->p.  Either
// way, the caller must not free it.

#include "GB_emult.h"
#ifndef GBCOMPACT
#include "GB_binop__include.h"
#endif

GrB_Info GB_emult_phase2                // C=A.*B or C<M>=A.*B
(
    GrB_Matrix *Chandle,                // output matrix
    const GrB_Type ctype,               // type of output matrix C
    const bool C_is_csc,                // format of output matrix C
    const GrB_BinaryOp op,              // op to perform C = op (A,B)
    // from phase1:
    const int64_t *restrict Cp,         // vector pointers for C
    const int64_t Cnvec_nonempty,       // # of non-empty vectors in C
    // tasks from phase0b:
    const GB_task_struct *restrict TaskList,  // array of structs
    const int ntasks,                         // # of tasks
    const int nthreads,                       // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *restrict Ch,         // Ch is NULL, or a shallow pointer
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    // original input:
    const GrB_Matrix M,                 // optional mask, may be NULL
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cp != NULL) ;
    ASSERT_OK (GB_check (op, "op for emult phase2", GB0)) ;
    ASSERT_OK (GB_check (A, "A for emult phase2", GB0)) ;
    ASSERT_OK (GB_check (B, "B for emult phase2", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for emult phase2", GB0)) ;
    ASSERT (A->vdim == B->vdim) ;
    ASSERT (GB_Type_compatible (ctype,   op->ztype)) ;
    ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    ASSERT (GB_Type_compatible (B->type, op->ytype)) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    int64_t cnz = Cp [Cnvec] ;
    (*Chandle) = NULL ;

    bool C_is_hyper = (Ch != NULL) ;

    // allocate the result C (but do not allocate C->p or C->h)
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, ctype, A->vlen, A->vdim, GB_Ap_null, C_is_csc,
        GB_SAME_HYPER_AS (C_is_hyper), A->hyper_ratio, Cnvec, cnz, true,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory; caller must free C_to_M, C_to_A, C_to_B
        // Ch must not be freed since Ch is always shallow
        GB_FREE_MEMORY (Cp, GB_IMAX (2, Cnvec+1), sizeof (int64_t)) ;
        return (info) ;
    }

    // transplant Cp into C as the vector pointers, from GB_emult_phase1.
    C->p = (int64_t *) Cp ;

    // add Ch as the the hypersparse list for C, from GB_emult_phase0
    if (C_is_hyper)
    { 
        // C->h is currently shallow; a copy is made at the end
        C->h = (int64_t *) Ch ;
        C->h_shallow = true ;
        C->nvec = Cnvec ;
    }

    C->nvec_nonempty = Cnvec_nonempty ;
    C->magic = GB_MAGIC ;

    GB_Type_code ccode = ctype->code ;

    //--------------------------------------------------------------------------
    // using a built-in binary operator
    //--------------------------------------------------------------------------

    bool done = false ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_AemultB(mult,xyname) GB_AemultB_ ## mult ## xyname

    #define GB_BINOP_WORKER(mult,xyname)                            \
    {                                                               \
        info = GB_AemultB(mult,xyname) (C, M, A, B,                 \
            C_to_M, C_to_A, C_to_B, TaskList, ntasks, nthreads) ;   \
        done = (info != GrB_NO_VALUE) ;                             \
    }                                                               \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        GB_Opcode opcode ;
        GB_Type_code xycode, zcode ;

        if (GB_binop_builtin (A, false, B, false, op,
            false, &opcode, &xycode, &zcode) && ccode == zcode)
        { 
            #include "GB_binop_factory.c"
            ASSERT (done) ;
        }

    #endif

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    if (!done)
    { 
        GxB_binary_function fmult ;
        size_t csize, asize, bsize, xsize, ysize, zsize ;
        GB_cast_function cast_A_to_X, cast_B_to_Y, cast_Z_to_C ;

        // C = A .* B with optional typecasting
        fmult = op->function ;
        csize = ctype->size ;
        asize = A->type->size ;
        bsize = B->type->size ;
        xsize = op->xtype->size ;
        ysize = op->ytype->size ;
        zsize = op->ztype->size ;
        cast_A_to_X = GB_cast_factory (op->xtype->code, A->type->code) ;
        cast_B_to_Y = GB_cast_factory (op->ytype->code, B->type->code) ;
        cast_Z_to_C = GB_cast_factory (ccode,           op->ztype->code) ;

        // aij = (xtype) A(i,j), located in Ax [pA]
        #define GB_GETA(aij,Ax,pA)                                          \
            GB_void aij [xsize] ;                                           \
            cast_A_to_X (aij, Ax +((pA)*asize), asize) ;

        // bij = (ytype) B(i,j), located in Bx [pB]
        #define GB_GETB(bij,Bx,pB)                                          \
            GB_void bij [ysize] ;                                           \
            cast_B_to_Y (bij, Bx +((pB)*bsize), bsize) ;

        // C(i,j) = (ctype) (A(i,j) + B(i,j))
        // not used if op is null
        #define GB_BINOP(cij, aij, bij)                                     \
            ASSERT (op != NULL) ;                                           \
            GB_void z [zsize] ;                                             \
            fmult (z, aij, bij) ;                                           \
            cast_Z_to_C (cij, z, csize) ;

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        #define GB_PHASE_2_OF_2

        #include "GB_emult_template.c"
    }

    //--------------------------------------------------------------------------
    // construct the final C->h
    //--------------------------------------------------------------------------

    if (C_is_hyper)
    {
        // create new Cp_new and Ch_new arrays, with no empty vectors
        int64_t *restrict Cp_new = NULL ;
        int64_t *restrict Ch_new = NULL ;
        int64_t nvec_new ;
        info = GB_hyper_prune (&Cp_new, &Ch_new, &nvec_new, C->p, C->h, Cnvec,
            Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (&C) ;
            return (info) ;
        }
        // transplant the new hyperlist into C
        GB_FREE_MEMORY (C->p, Cnvec+1, sizeof (int64_t)) ;
        // do not free C->h since it is a shallow copy
        ASSERT (C->h_shallow) ;
        C->p = Cp_new ;
        C->h = Ch_new ;
        C->nvec = nvec_new ;
        C->plen = nvec_new ;
        C->h_shallow = false ;
        ASSERT (C->nvec == C->nvec_nonempty) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // caller must free C_to_M, C_to_A, and C_to_B, but not Cp or Ch
    ASSERT_OK (GB_check (C, "C output for emult phase2", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}


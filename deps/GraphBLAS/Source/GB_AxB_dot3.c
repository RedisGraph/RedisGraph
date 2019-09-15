//------------------------------------------------------------------------------
// GB_AxB_dot3: compute C<M> = A'*B in parallel
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This function only computes C<M>=A'*B.  The mask must be present, and not
// complemented.  The mask is always applied.

#include "GB_mxm.h"
#ifndef GBCOMPACT
#include "GB_AxB__include.h"
#endif

#define GB_FREE_WORK                                                    \
{                                                                       \
    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;  \
}

#define GB_FREE_ALL                                                     \
{                                                                       \
    GB_FREE_WORK ;                                                      \
    GrB_free (Chandle) ;                                                \
}

GrB_Info GB_AxB_dot3                // C<M> = A'*B using dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;
    ASSERT_OK (GB_check (M, "M for dot3 A'*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for dot3 A'*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for dot3 A'*B", GB0)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for numeric A'*B", GB0)) ;
    ASSERT (A->vlen == B->vlen) ;

    int ntasks, max_ntasks = 0, nthreads ;
    GB_task_struct *TaskList = NULL ;

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

    //--------------------------------------------------------------------------
    // get M, A, and B
    //--------------------------------------------------------------------------

    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mh = M->h ;
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = M->x ;
    const size_t msize = M->type->size ;
    const int64_t mvlen = M->vlen ;
    const int64_t mvdim = M->vdim ;
    const int64_t mnz = GB_NNZ (M) ;
    const int64_t mnvec = M->nvec ;
    const bool M_is_hyper = M->is_hyper ;
    GB_cast_function cast_M = GB_cast_factory (GB_BOOL_code, M->type->code) ;

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ah = A->h ;
    // const int64_t *restrict Ai = A->i ;
    // const int64_t avlen = A->vlen ;
    // const int64_t avdim = A->vdim ;
    // const int64_t anz = GB_NNZ (A) ;
    const int64_t anvec = A->nvec ;
    const bool A_is_hyper = A->is_hyper ;

    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bh = B->h ;
    // const int64_t *restrict Bi = B->i ;
    // const int64_t bvlen = B->vlen ;
    // const int64_t bvdim = B->vdim ;
    // const int64_t bnz = GB_NNZ (B) ;
    const int64_t bnvec = B->nvec ;
    const bool B_is_hyper = B->is_hyper ;

    //--------------------------------------------------------------------------
    // allocate C, the same size and # of entries as M
    //--------------------------------------------------------------------------

    GrB_Type ctype = add->op->ztype ;
    int64_t cvlen = mvlen ;
    int64_t cvdim = mvdim ;
    int64_t cnz = mnz ;
    int64_t cnvec = mnvec ;

    GB_CREATE (Chandle, ctype, cvlen, cvdim, GB_Ap_malloc, true,
        GB_SAME_HYPER_AS (M_is_hyper), M->hyper_ratio, cnvec,
        cnz+1,  // add one to cnz for GB_cumsum
        true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (info) ;
    }

    GrB_Matrix C = (*Chandle) ;

    int64_t *restrict Cp = C->p ;
    int64_t *restrict Ch = C->h ;
    int64_t *restrict Cwork = C->i ;    // use C->i as workspace
    // printf ("Ch is %p\n", (void *) Ch) ;

    //--------------------------------------------------------------------------
    // determine the # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // copy Mp and Mh into C
    //--------------------------------------------------------------------------

    // FUTURE:: C->p and C->h could be shallow copies of M->p and M->h, which
    // could same some time and memory if C is then, say, transposed by
    // GB_accum_mask later on.

    nthreads = GB_nthreads (cnvec, chunk, nthreads_max) ;
    GB_memcpy (Cp, Mp, (cnvec+1) * sizeof (int64_t), nthreads) ;
    if (M_is_hyper)
    { 
        GB_memcpy (Ch, Mh, cnvec * sizeof (int64_t), nthreads) ;
    }
    C->magic = GB_MAGIC ;
    C->nvec_nonempty = M->nvec_nonempty ;
    C->nvec = M->nvec ;

    //--------------------------------------------------------------------------
    // construct the tasks for the first phase
    //--------------------------------------------------------------------------

    nthreads = GB_nthreads (cnz, chunk, nthreads_max) ;
    GB_OK (GB_AxB_dot3_one_slice (&TaskList, &max_ntasks, &ntasks, &nthreads,
        M, Context)) ;

    //--------------------------------------------------------------------------
    // phase1: estimate the work to compute each entry in C
    //--------------------------------------------------------------------------

    // The work to compute C(i,j) is held in Cwork [p], if C(i,j) appears in
    // as the pth entry in C.

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (int taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        // GB_GET_TASK_DESCRIPTOR ;
        int64_t kfirst = TaskList [taskid].kfirst ;
        int64_t klast  = TaskList [taskid].klast ;
        bool fine_task = (klast == -1) ;
        if (fine_task)
        { 
            // a fine task operates on a slice of a single vector
            klast = kfirst ;
        }

        int64_t bpleft = 0 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get j, the kth vector of C and M
            //------------------------------------------------------------------

            int64_t j = (Mh == NULL) ? k : Mh [k] ;
            GB_GET_VECTOR (pM, pM_end, pM, pM_end, Mp, k) ;

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            int64_t pB, pB_end ;
            GB_lookup (B_is_hyper, Bh, Bp, &bpleft, bnvec-1, j, &pB, &pB_end) ;
            int64_t bjnz = pB_end - pB ;

            //------------------------------------------------------------------
            // estimate the work to compute each entry of C(:,j)
            //------------------------------------------------------------------

            // A decent estimate of the work to compute the dot product C(i,j)
            // = A(:,i)'*B(:,j) is min (|A(:,i)|, |B(:,j)|) + 1.  This is a
            // lower bound.  The actual work could require a binary search of
            // either A(:,i) or B(:,j), or a merge of the two vectors.  Or it
            // could require no work at all if all entries in A(:,i) appear
            // before all entries in B(:,j), or visa versa.  No work is done if
            // M(i,j)=0.  A more accurate estimate is possible to compute,
            // following the different methods used in
            // Template/GB_AxB_dot_cij.c.

            if (bjnz == 0)
            {
                // B(:,j) is empty, so C(:,j) is empty as well.  No work is to
                // be done, but it still takes unit work to flag each C(:,j) as
                // a zombie
                for ( ; pM < pM_end ; pM++)
                { 
                    Cwork [pM] = 1 ;
                }
            }
            else
            {
                int64_t apleft = 0 ;
                for ( ; pM < pM_end ; pM++)
                {
                    int64_t work = 1 ;
                    bool mij ;
                    cast_M (&mij, Mx +(pM*msize), 0) ;
                    if (mij)
                    { 
                        int64_t pA, pA_end, i = Mi [pM] ;
                        GB_lookup (A_is_hyper, Ah, Ap, &apleft, anvec-1, i,
                            &pA, &pA_end) ;
                        int64_t ajnz = pA_end - pA ;
                        work += GB_IMIN (ajnz, bjnz) ;
                    }
                    Cwork [pM] = work ;
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // free the current tasks and construct the tasks for the second phase
    //--------------------------------------------------------------------------

    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;
    GB_OK (GB_AxB_dot3_slice (&TaskList, &max_ntasks, &ntasks, &nthreads,
        C, Context)) ;

    // if (ntasks > 1) printf ("ntasks %d\n", ntasks) ;

    //--------------------------------------------------------------------------
    // C<M> = A'*B, via masked dot product method and built-in semiring
    //--------------------------------------------------------------------------

    bool done = false ;

#ifndef GBCOMPACT

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_Adot3B(add,mult,xyname) GB_Adot3B_ ## add ## mult ## xyname

    #define GB_AxB_WORKER(add,mult,xyname)                              \
    {                                                                   \
        info = GB_Adot3B (add,mult,xyname) (C, M,                       \
            A, A_is_pattern, B, B_is_pattern,                           \
            TaskList, ntasks, nthreads) ;                               \
        done = (info != GrB_NO_VALUE) ;                                 \
    }                                                                   \
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
            info = GB_AxB_user (GxB_AxB_DOT, semiring, Chandle, M, A, B,
                flipxy,
                /* heap: */ NULL, NULL, NULL, 0,
                /* Gustavson: */ NULL,
                /* dot: */ NULL, nthreads, 0, 0, NULL,
                /* dot3: */ TaskList, ntasks) ;
            done = true ;
        }
    }

    //--------------------------------------------------------------------------
    // C<M> = A'*B, via masked dot product method and typecasting
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

        // scalar workspace: because of typecasting, the x/y types need not
        // be the same as the size of the A and B types.
        // flipxy false: aki = (xtype) A(k,i) and bkj = (ytype) B(k,j)
        // flipxy true:  aki = (ytype) A(k,i) and bkj = (xtype) B(k,j)
        size_t aki_size = flipxy ? ysize : xsize ;
        size_t bkj_size = flipxy ? xsize : ysize ;

        // GB_void *restrict identity = add->identity ;
        GB_void *restrict terminal = add->terminal ;

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
        // C<M> = A'*B via dot products, function pointers, and typecasting
        //----------------------------------------------------------------------

        // aki = A(k,i), located in Ax [pA]
        #define GB_GETA(aki,Ax,pA)                                          \
            GB_void aki [aki_size] ;                                        \
            if (!A_is_pattern) cast_A (aki, Ax +((pA)*asize), asize) ;

        // bkj = B(k,j), located in Bx [pB]
        #define GB_GETB(bkj,Bx,pB)                                          \
            GB_void bkj [bkj_size] ;                                        \
            if (!B_is_pattern) cast_B (bkj, Bx +((pB)*bsize), bsize) ;

        // break if cij reaches the terminal value
        #define GB_DOT_TERMINAL(cij)                                        \
            if (terminal != NULL && memcmp (cij, terminal, csize) == 0)     \
            {                                                               \
                break ;                                                     \
            }

        // C(i,j) = A(i,k) * B(k,j)
        #define GB_MULT(cij, aki, bkj)                                      \
            GB_MULTIPLY (cij, aki, bkj) ;                                   \

        // C(i,j) += A(i,k) * B(k,j)
        #define GB_MULTADD(cij, aki, bkj)                                   \
            GB_void zwork [csize] ;                                         \
            GB_MULTIPLY (zwork, aki, bkj) ;                                 \
            fadd (cij, cij, zwork) ;

        // define cij for each task
        #define GB_CIJ_DECLARE(cij)                                         \
            GB_void cij [csize] ;

        // address of Cx [p]
        #define GB_CX(p) Cx +((p)*csize)

        // save the value of C(i,j)
        #define GB_CIJ_SAVE(cij,p)                                          \
            memcpy (GB_CX (p), cij, csize) ;

        #define GB_ATYPE GB_void
        #define GB_BTYPE GB_void
        #define GB_CTYPE GB_void

        // loops with function pointers cannot be vectorized
        #define GB_DOT_SIMD ;

        if (flipxy)
        { 
            #define GB_MULTIPLY(z,x,y) fmult (z,y,x)
            #include "GB_AxB_dot3_template.c"
            #undef GB_MULTIPLY
        }
        else
        { 
            #define GB_MULTIPLY(z,x,y) fmult (z,x,y)
            #include "GB_AxB_dot3_template.c"
            #undef GB_MULTIPLY
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    if (C->nzombies > 0)
    {
        // C has been created with zombies, so place it in the queue
        GB_CRITICAL (GB_queue_insert (C)) ;
    }

    GB_FREE_WORK ;
    ASSERT_OK (GB_check (C, "dot3: C<M> = A'*B output", GB0)) ;
    ASSERT (*Chandle == C) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    return (GrB_SUCCESS) ;
}


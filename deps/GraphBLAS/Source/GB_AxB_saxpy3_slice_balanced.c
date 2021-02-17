//------------------------------------------------------------------------------
// GB_AxB_saxpy3_slice_balanced: construct balanced tasks for GB_AxB_saxpy3
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_AxB_saxpy3.h"

// control parameters for generating parallel tasks
#define GB_NTASKS_PER_THREAD 2
#define GB_COSTLY 1.2
#define GB_FINE_WORK 2
#define GB_MWORK_ALPHA 0.01
#define GB_MWORK_BETA 0.10

#define GB_FREE_WORK            \
{                               \
    GB_FREE (Fine_fl) ;         \
    GB_FREE (Coarse_Work) ;     \
    GB_FREE (Coarse_initial) ;  \
    GB_FREE (Fine_slice) ;      \
}

#define GB_FREE_ALL             \
{                               \
    GB_FREE_WORK ;              \
    GB_FREE (TaskList) ;        \
}

//------------------------------------------------------------------------------
// GB_hash_table_size
//------------------------------------------------------------------------------

// flmax is the max flop count for computing A*B(:,j), for any vector j that
// this task computes.  If the mask M is present, flmax also includes the
// number of entries in M(:,j).  GB_hash_table_size determines the hash table
// size for this task, which is twice the smallest power of 2 larger than
// flmax.  If flmax is large enough, the hash_size is returned as cvlen, so
// that Gustavson's method will be used instead of the Hash method.

// By default, Gustavson vs Hash is selected automatically.  AxB_method can be
// selected via the descriptor or a global setting, as the non-default
// GxB_AxB_GUSTAVSON or GxB_AxB_HASH settings, to enforce the selection of
// either of those methods.  However, if Hash is selected but the hash table
// equals or exceeds cvlen, then Gustavson's method is used instead.

static inline int64_t GB_hash_table_size
(
    int64_t flmax,      // max flop count for any vector computed by this task
    int64_t cvlen,      // vector length of C
    const GrB_Desc_Value AxB_method     // Default, Gustavson, or Hash
)
{
    // hash_size = 2 * (smallest power of 2 >= flmax)
    double hlog = log2 ((double) flmax) ;
    int64_t hash_size = ((int64_t) 2) << ((int64_t) floor (hlog) + 1) ;
    bool use_Gustavson ;

    if (AxB_method == GxB_AxB_GUSTAVSON)
    { 
        // always use Gustavson's method
        use_Gustavson = true ;
    }
    else if (AxB_method == GxB_AxB_HASH)
    { 
        // always use Hash method, unless the hash_size >= cvlen
        use_Gustavson = (hash_size >= cvlen) ;
    }
    else
    { 
        // default: auto selection:
        // use Gustavson's method if hash_size is too big
        use_Gustavson = (hash_size >= cvlen/12) ;
    }

    if (use_Gustavson)
    { 
        hash_size = cvlen ;
    }
    return (hash_size) ;
}

//------------------------------------------------------------------------------
// GB_create_coarse_task: create a single coarse task
//------------------------------------------------------------------------------

// Compute the max flop count for any vector in a coarse task, determine the
// hash table size, and construct the coarse task.

static inline void GB_create_coarse_task
(
    int64_t kfirst,     // coarse task consists of vectors kfirst:klast
    int64_t klast,
    GB_saxpy3task_struct *TaskList,
    int taskid,         // taskid for this coarse task
    int64_t *Bflops,    // size bnvec; cum sum of flop counts for vectors of B
    int64_t cvlen,      // vector length of B and C
    double chunk,
    int nthreads_max,
    int64_t *Coarse_Work,   // workspace for parallel reduction for flop count
    const GrB_Desc_Value AxB_method     // Default, Gustavson, or Hash
)
{

    //--------------------------------------------------------------------------
    // find the max # of flops for any vector in this task
    //--------------------------------------------------------------------------

    int64_t nk = klast - kfirst + 1 ;
    int nth = GB_nthreads (nk, chunk, nthreads_max) ;

    // each thread finds the max flop count for a subset of the vectors
    int tid ;
    #pragma omp parallel for num_threads(nth) schedule(static)
    for (tid = 0 ; tid < nth ; tid++)
    {
        int64_t my_flmax = 1, istart, iend ;
        GB_PARTITION (istart, iend, nk, tid, nth) ;
        for (int64_t i = istart ; i < iend ; i++)
        { 
            int64_t kk = kfirst + i ;
            int64_t fl = Bflops [kk+1] - Bflops [kk] ;
            my_flmax = GB_IMAX (my_flmax, fl) ;
        }
        Coarse_Work [tid] = my_flmax ;
    }

    // combine results from each thread
    int64_t flmax = 1 ;
    for (tid = 0 ; tid < nth ; tid++)
    { 
        flmax = GB_IMAX (flmax, Coarse_Work [tid]) ;
    }

    // check the parallel computation
    #ifdef GB_DEBUG
    int64_t flmax2 = 1 ;
    for (int64_t kk = kfirst ; kk <= klast ; kk++)
    {
        int64_t fl = Bflops [kk+1] - Bflops [kk] ;
        flmax2 = GB_IMAX (flmax2, fl) ;
    }
    ASSERT (flmax == flmax2) ;
    #endif

    //--------------------------------------------------------------------------
    // define the coarse task
    //--------------------------------------------------------------------------

    TaskList [taskid].start   = kfirst ;
    TaskList [taskid].end     = klast ;
    TaskList [taskid].vector  = -1 ;
    TaskList [taskid].hsize   = GB_hash_table_size (flmax, cvlen, AxB_method) ;
    TaskList [taskid].Hi      = NULL ;      // assigned later
    TaskList [taskid].Hf      = NULL ;      // assigned later
    TaskList [taskid].Hx      = NULL ;      // assigned later
    TaskList [taskid].my_cjnz = 0 ;         // unused
    TaskList [taskid].leader  = taskid ;
    TaskList [taskid].team_size = 1 ;
}

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_slice_balanced: create balanced tasks for saxpy3
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy3_slice_balanced
(
    // inputs
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    GrB_Desc_Value AxB_method,      // Default, Gustavson, or Hash
    // outputs
    GB_saxpy3task_struct **TaskList_handle,
    bool *apply_mask,               // if true, apply M during sapxy3
    bool *M_dense_in_place,         // if true, use M in-place
    int *ntasks,                    // # of tasks created (coarse and fine)
    int *nfine,                     // # of fine tasks created
    int *nthreads,                  // # of threads to use
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    (*apply_mask) = false ;
    (*M_dense_in_place) = false ;
    (*ntasks) = 0 ;
    (*nfine) = 0 ;
    (*nthreads) = 0 ;

    ASSERT_MATRIX_OK_OR_NULL (M, "M for saxpy3_slice_balanced A*B", GB0) ;
    ASSERT (!GB_PENDING (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!GB_ZOMBIES (M)) ;

    ASSERT_MATRIX_OK (A, "A for saxpy3_slice_balanced A*B", GB0) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    ASSERT_MATRIX_OK (B, "B for saxpy3_slice_balanced A*B", GB0) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (GB_JUMBLED_OK (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;

    //--------------------------------------------------------------------------
    // determine the # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // define result and workspace
    //--------------------------------------------------------------------------

    GB_saxpy3task_struct *GB_RESTRICT TaskList = NULL ;

    int64_t *GB_RESTRICT Coarse_initial = NULL ;    // initial coarse tasks
    int64_t *GB_RESTRICT Coarse_Work = NULL ;       // workspace for flop counts
    int64_t *GB_RESTRICT Fine_slice = NULL ;
    int64_t *GB_RESTRICT Fine_fl = NULL ;

    //--------------------------------------------------------------------------
    // get A, and B
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t avlen = A->vlen ;
    const int64_t anvec = A->nvec ;
    const bool A_is_hyper = GB_IS_HYPERSPARSE (A) ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    const int8_t  *GB_RESTRICT Bb = B->b ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    const int64_t bvdim = B->vdim ;
    const int64_t bnz = GB_NNZ_HELD (B) ;
    const int64_t bnvec = B->nvec ;
    const int64_t bvlen = B->vlen ;
    const bool B_is_hyper = GB_IS_HYPERSPARSE (B) ;

    int64_t cvlen = avlen ;
    int64_t cvdim = bvdim ;

    //--------------------------------------------------------------------------
    // compute flop counts for each vector of B and C
    //--------------------------------------------------------------------------

    int64_t Mwork = 0 ;
    int64_t *GB_RESTRICT Bflops = C->p ;    // use C->p as workspace for Bflops
    GB_OK (GB_AxB_saxpy3_flopcount (&Mwork, Bflops, M, Mask_comp, A, B,
        Context)) ;
    int64_t total_flops = Bflops [bnvec] ;
    double axbflops = total_flops - Mwork ;
    GBURBLE ("axbwork %g ", axbflops) ;
    if (Mwork > 0) GBURBLE ("mwork %g ", (double) Mwork) ;

    //--------------------------------------------------------------------------
    // determine if the mask M should be applied, or done later
    //--------------------------------------------------------------------------

    if (M == NULL)
    {

        //----------------------------------------------------------------------
        // M is not present 
        //----------------------------------------------------------------------

        (*apply_mask) = false ;

    }
    else if (GB_is_packed (M))
    {

        //----------------------------------------------------------------------
        // M is present and full, bitmap, or sparse/hyper with all entries
        //----------------------------------------------------------------------

        // Choose all-hash or all-Gustavson tasks, and apply M during saxpy3.

        (*apply_mask) = true ;

        // The work for M has not yet been added Bflops.
        // Each vector M(:,j) has cvlen entries.
        Mwork = cvlen * cvdim ;

        if (!(AxB_method == GxB_AxB_HASH || AxB_method == GxB_AxB_GUSTAVSON))
        {
            if (axbflops < (double) Mwork * GB_MWORK_BETA)
            { 
                // The mask is too costly to scatter into the Hf workspace.
                // Leave it in place and use all-hash tasks.
                AxB_method = GxB_AxB_HASH ;
            }
            else
            { 
                // Scatter M into Hf and use all-Gustavson tasks.
                AxB_method = GxB_AxB_GUSTAVSON ;
            }
        }

        if (AxB_method == GxB_AxB_HASH)
        {
            // Use the hash method for all tasks (except for those tasks which
            // require a hash table size >= cvlen; those tasks use Gustavson).
            // Do not scatter the mask into the Hf hash workspace.  The work
            // for the mask is not accounted for in Bflops, so the hash tables
            // can be small.
            (*M_dense_in_place) = true ;
            GBURBLE ("(use dense mask in-place) ") ;
        }
        else
        {
            // Use the Gustavson method for all tasks, and scatter M into the
            // fine Gustavson workspace.  The work for M is not yet in the
            // Bflops cumulative sum.  Add it now.
            ASSERT (AxB_method == GxB_AxB_GUSTAVSON)
            int nth = GB_nthreads (bnvec, chunk, nthreads_max) ;
            int64_t kk ;
            #pragma omp parallel for num_threads(nth) schedule(static)
            for (kk = 0 ; kk <= bnvec ; kk++)
            { 
                Bflops [kk] += cvlen * (kk+1) ;
            }
            total_flops = Bflops [bnvec] ;
            GBURBLE ("(use dense mask) ") ;
        }

    }
    else if (axbflops < ((double) Mwork * GB_MWORK_ALPHA))
    {

        //----------------------------------------------------------------------
        // M is costly to use; apply it after C=A*B
        //----------------------------------------------------------------------

        // Do not use M during the computation of A*B.  Instead, compute C=A*B
        // and then apply the mask later.  Tell the caller that the mask should
        // not be applied, so that it will be applied later in GB_mxm.

        (*apply_mask) = false ;

        // redo the flop count analysis, without the mask
        GB_OK (GB_AxB_saxpy3_flopcount (&Mwork, Bflops, NULL, false, A, B,
            Context)) ;
        total_flops = Bflops [bnvec] ;
        GBURBLE ("(discard mask) ") ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // use M during saxpy3
        //----------------------------------------------------------------------

        (*apply_mask) = true ;
        GBURBLE ("(use mask) ") ;
    }

    //--------------------------------------------------------------------------
    // determine # of threads and # of initial coarse tasks
    //--------------------------------------------------------------------------

    (*nthreads) = GB_nthreads ((double) total_flops, chunk, nthreads_max) ;
    int ntasks_initial = ((*nthreads) == 1) ? 1 :
        (GB_NTASKS_PER_THREAD * (*nthreads)) ;

    //--------------------------------------------------------------------------
    // give preference to Gustavson when using few threads
    //--------------------------------------------------------------------------

    if ((*nthreads) <= 8 &&
        (!(AxB_method == GxB_AxB_HASH || AxB_method == GxB_AxB_GUSTAVSON)))
    {
        // Unless a specific method has been explicitly requested, see if
        // Gustavson should be used with a small number of threads.
        // Matrix-vector has a maximum intensity of 1, so this heuristic only
        // applies to GrB_mxm.
        double abnz = GB_NNZ (A) + GB_NNZ (B) + 1 ;
        double workspace = (double) ntasks_initial * (double) cvlen ;
        double intensity = total_flops / abnz ;
        GBURBLE ("(intensity: %0.3g workspace/(nnz(A)+nnz(B)): %0.3g",
            intensity, workspace / abnz) ;
        if (intensity >= 8 && workspace < abnz)
        {
            // work intensity is large, and Gustvason workspace is modest;
            // use Gustavson for all tasks
            AxB_method = GxB_AxB_GUSTAVSON ;
            GBURBLE (": select Gustvason) ") ;
        }
        else
        {
            // use default task creation: mix of Hash and Gustavson
            GBURBLE (") ") ;
        }
    }

    //--------------------------------------------------------------------------
    // determine target task size
    //--------------------------------------------------------------------------

    double target_task_size = ((double) total_flops) / ntasks_initial ;
    target_task_size = GB_IMAX (target_task_size, chunk) ;
    double target_fine_size = target_task_size / GB_FINE_WORK ;
    target_fine_size = GB_IMAX (target_fine_size, chunk) ;

    //--------------------------------------------------------------------------
    // determine # of parallel tasks
    //--------------------------------------------------------------------------

    int ncoarse = 0 ;       // # of coarse tasks
    int max_bjnz = 0 ;      // max (nnz (B (:,j))) of fine tasks

    // FUTURE: also use ultra-fine tasks that compute A(i1:i2,k)*B(k,j)

    if (ntasks_initial > 1)
    {

        //----------------------------------------------------------------------
        // construct initial coarse tasks
        //----------------------------------------------------------------------

        if (!GB_pslice (&Coarse_initial, Bflops, bnvec, ntasks_initial, true))
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // split the work into coarse and fine tasks
        //----------------------------------------------------------------------

        for (int taskid = 0 ; taskid < ntasks_initial ; taskid++)
        {
            // get the initial coarse task
            int64_t kfirst = Coarse_initial [taskid] ;
            int64_t klast  = Coarse_initial [taskid+1] ;
            int64_t task_ncols = klast - kfirst ;
            int64_t task_flops = Bflops [klast] - Bflops [kfirst] ;

            if (task_ncols == 0)
            { 
                // This coarse task is empty, having been squeezed out by
                // costly vectors in adjacent coarse tasks.
            }
            else if (task_flops > 2 * GB_COSTLY * target_task_size)
            {
                // This coarse task is too costly, because it contains one or
                // more costly vectors.  Split its vectors into a mixture of
                // coarse and fine tasks.

                int64_t kcoarse_start = kfirst ;

                for (int64_t kk = kfirst ; kk < klast ; kk++)
                {
                    // jflops = # of flops to compute a single vector A*B(:,j)
                    // where j == GBH (Bh, kk)
                    double jflops = Bflops [kk+1] - Bflops [kk] ;
                    // bjnz = nnz (B (:,j))
                    int64_t bjnz = (Bp == NULL) ? bvlen : (Bp [kk+1] - Bp [kk]);

                    if (jflops > GB_COSTLY * target_task_size && bjnz > 1)
                    {
                        // A*B(:,j) is costly; split it into 2 or more fine
                        // tasks.  First flush the prior coarse task, if any.
                        if (kcoarse_start < kk)
                        { 
                            // vectors kcoarse_start to kk-1 form a single
                            // coarse task
                            ncoarse++ ;
                        }

                        // next coarse task (if any) starts at kk+1
                        kcoarse_start = kk+1 ;

                        // vectors kk will be split into multiple fine tasks
                        max_bjnz = GB_IMAX (max_bjnz, bjnz) ;
                        int team_size = ceil (jflops / target_fine_size) ;
                        (*nfine) += team_size ;
                    }
                }

                // flush the last coarse task, if any
                if (kcoarse_start < klast)
                { 
                    // vectors kcoarse_start to klast-1 form a single
                    // coarse task
                    ncoarse++ ;
                }

            }
            else
            { 
                // This coarse task is OK as-is.
                ncoarse++ ;
            }
        }
    }
    else
    {

        //----------------------------------------------------------------------
        // entire computation in a single fine or coarse task
        //----------------------------------------------------------------------

        if (bnvec == 1)
        { 
            // If B is a single vector, and is computed by a single thread,
            // then a single fine task is used.
            (*nfine) = 1 ;
            ncoarse = 0 ;
        }
        else
        { 
            // One thread uses a single coarse task if B is not a vector.
            (*nfine) = 0 ;
            ncoarse = 1 ;
        }
    }

    (*ntasks) = ncoarse + (*nfine) ;

    //--------------------------------------------------------------------------
    // allocate the tasks, and workspace to construct fine tasks
    //--------------------------------------------------------------------------

    TaskList    = GB_CALLOC ((*ntasks), GB_saxpy3task_struct) ;
    Coarse_Work = GB_MALLOC (nthreads_max, int64_t) ;
    if (max_bjnz > 0)
    { 
        // also allocate workspace to construct fine tasks
        Fine_slice = GB_MALLOC ((*ntasks)+1  , int64_t) ;
        Fine_fl    = GB_MALLOC (max_bjnz+1, int64_t) ;
    }

    if (TaskList == NULL || Coarse_Work == NULL ||
        (max_bjnz > 0 && (Fine_slice == NULL || Fine_fl == NULL)))
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // create the tasks
    //--------------------------------------------------------------------------

    if (ntasks_initial > 1)
    {

        //----------------------------------------------------------------------
        // create the coarse and fine tasks
        //----------------------------------------------------------------------

        int nf = 0 ;            // fine tasks have task id 0:nfine-1
        int nc = (*nfine) ;     // coarse task ids are nfine:ntasks-1

        for (int taskid = 0 ; taskid < ntasks_initial ; taskid++)
        {
            // get the initial coarse task
            int64_t kfirst = Coarse_initial [taskid] ;
            int64_t klast  = Coarse_initial [taskid+1] ;
            int64_t task_ncols = klast - kfirst ;
            int64_t task_flops = Bflops [klast] - Bflops [kfirst] ;

            if (task_ncols == 0)
            { 
                // This coarse task is empty, having been squeezed out by
                // costly vectors in adjacent coarse tasks.
            }
            else if (task_flops > 2 * GB_COSTLY * target_task_size)
            {
                // This coarse task is too costly, because it contains one or
                // more costly vectors.  Split its vectors into a mixture of
                // coarse and fine tasks.

                int64_t kcoarse_start = kfirst ;

                for (int64_t kk = kfirst ; kk < klast ; kk++)
                {
                    // jflops = # of flops to compute a single vector A*B(:,j)
                    double jflops = Bflops [kk+1] - Bflops [kk] ;
                    // bjnz = nnz (B (:,j))
                    int64_t bjnz = (Bp == NULL) ? bvlen : (Bp [kk+1] - Bp [kk]);

                    if (jflops > GB_COSTLY * target_task_size && bjnz > 1)
                    {
                        // A*B(:,j) is costly; split it into 2 or more fine
                        // tasks.  First flush the prior coarse task, if any.
                        if (kcoarse_start < kk)
                        { 
                            // kcoarse_start:kk-1 form a single coarse task
                            GB_create_coarse_task (kcoarse_start, kk-1,
                                TaskList, nc++, Bflops, cvlen, chunk,
                                nthreads_max, Coarse_Work, AxB_method) ;
                        }

                        // next coarse task (if any) starts at kk+1
                        kcoarse_start = kk+1 ;

                        // count the work for each entry B(k,j).  Do not
                        // include the work to scan M(:,j), since that will
                        // be evenly divided between all tasks in this team.
                        int64_t pB_start = GBP (Bp, kk, bvlen) ;
                        int nth = GB_nthreads (bjnz, chunk, nthreads_max) ;
                        int64_t s ;
                        #pragma omp parallel for num_threads(nth) \
                            schedule(static)
                        for (s = 0 ; s < bjnz ; s++)
                        { 
                            // get B(k,j)
                            Fine_fl [s] = 1 ;
                            int64_t pB = pB_start + s ;
                            if (!GBB (Bb, pB)) continue ;
                            int64_t k = GBI (Bi, pB, bvlen) ;
                            // fl = flop count for just A(:,k)*B(k,j)
                            int64_t pA, pA_end ;
                            int64_t pleft = 0 ;
                            GB_lookup (A_is_hyper, Ah, Ap, avlen, &pleft,
                                anvec-1, k, &pA, &pA_end) ;
                            int64_t fl = pA_end - pA ;
                            Fine_fl [s] = fl ;
                            ASSERT (fl >= 0) ;
                        }

                        // cumulative sum of flops to compute A*B(:,j)
                        GB_cumsum (Fine_fl, bjnz, NULL, nth) ;

                        // slice B(:,j) into fine tasks
                        int team_size = ceil (jflops / target_fine_size) ;
                        ASSERT (Fine_slice != NULL) ;
                        GB_pslice (&Fine_slice, Fine_fl, bjnz, team_size,
                            false) ;

                        // shared hash table for all fine tasks for A*B(:,j)
                        int64_t hsize = 
                            GB_hash_table_size (jflops, cvlen, AxB_method) ;

                        // construct the fine tasks for C(:,j)=A*B(:,j)
                        int leader = nf ;
                        for (int fid = 0 ; fid < team_size ; fid++)
                        { 
                            int64_t pstart = Fine_slice [fid] ;
                            int64_t pend   = Fine_slice [fid+1] ;
                            int64_t fl = Fine_fl [pend] - Fine_fl [pstart] ;
                            TaskList [nf].start  = pB_start + pstart ;
                            TaskList [nf].end    = pB_start + pend - 1 ;
                            TaskList [nf].vector = kk ;
                            TaskList [nf].hsize  = hsize ;
                            TaskList [nf].Hi = NULL ;   // assigned later
                            TaskList [nf].Hf = NULL ;   // assigned later
                            TaskList [nf].Hx = NULL ;   // assigned later
                            TaskList [nf].my_cjnz = 0 ;
                            TaskList [nf].leader = leader ;
                            TaskList [nf].team_size = team_size ;
                            nf++ ;
                        }
                    }
                }

                // flush the last coarse task, if any
                if (kcoarse_start < klast)
                { 
                    // kcoarse_start:klast-1 form a single coarse task
                    GB_create_coarse_task (kcoarse_start, klast-1, TaskList,
                        nc++, Bflops, cvlen, chunk, nthreads_max,
                        Coarse_Work, AxB_method) ;
                }

            }
            else
            { 
                // This coarse task is OK as-is.
                GB_create_coarse_task (kfirst, klast-1, TaskList, nc++, Bflops,
                    cvlen, chunk, nthreads_max, Coarse_Work, AxB_method) ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // entire computation in a single fine or coarse task
        //----------------------------------------------------------------------

        // create a single coarse task: hash or Gustavson
        GB_create_coarse_task (0, bnvec-1, TaskList, 0, Bflops, cvlen, 1, 1,
            Coarse_Work, AxB_method) ;

        if (bnvec == 1)
        { 
            // convert the single coarse task into a single fine task
            TaskList [0].start  = 0 ;           // first entry in B(:,0)
            TaskList [0].end = bnz - 1 ;        // last entry in B(:,0)
            TaskList [0].vector = 0 ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    (*TaskList_handle) = TaskList ;
    return (GrB_SUCCESS) ;
}


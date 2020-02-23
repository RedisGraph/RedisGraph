//------------------------------------------------------------------------------
// GB_AxB_saxpy3_symbolic: symbolic analysis for GB_AxB_saxpy3
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Symbolic analysis for C=A*B, C<M>=A*B or C<!M>=A*B, via GB_AxB_saxpy3.
// Coarse tasks compute nnz (C (:,j)) for each of their vectors j.  Fine tasks
// just scatter the mask M into the hash table.  This phase does not depend on
// the semiring, nor does it depend on the type of C, A, or B.  It does access
// the values of M, if the mask matrix M is present and not structural.

#include "GB_AxB_saxpy3.h"
#include "GB_AxB_saxpy3_template.h"
#include "GB_atomics.h"
#include "GB_bracket.h"
// GB_GET_A_k and GB_GET_M_j declare aknz and mjnz, but these are unused here.
#include "GB_unused.h"

void GB_AxB_saxpy3_symbolic
(
    GrB_Matrix C,               // Cp [k] is computed for coarse tasks
    const GrB_Matrix M,         // mask matrix M
    bool Mask_comp,             // M complemented, or not
    bool Mask_struct,           // M structural, or not
    const GrB_Matrix A,         // A matrix; only the pattern is accessed
    const GrB_Matrix B,         // B matrix; only the pattern is accessed
    GB_saxpy3task_struct *TaskList,     // list of tasks, and workspace
    int ntasks,                 // total number of tasks
    int nfine,                  // number of fine tasks
    int nthreads                // number of threads
)
{

    //--------------------------------------------------------------------------
    // get M, A, B, and C
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Cp = C->p ;
    // const int64_t *GB_RESTRICT Ch = C->h ;
    const int64_t cvlen = C->vlen ;
    // const int64_t cnvec = C->nvec ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    // const GB_BTYPE *GB_RESTRICT Bx = B_is_pattern ? NULL : B->x ;
    // const int64_t bvlen = B->vlen ;
    // const int64_t bnvec = B->nvec ;
    // const bool B_is_hyper = B->is_hyper ;

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const int64_t anvec = A->nvec ;
    const bool A_is_hyper = GB_IS_HYPER (A) ;
    // const GB_ATYPE *GB_RESTRICT Ax = A_is_pattern ? NULL : A->x ;

    const int64_t *GB_RESTRICT Mp = NULL ;
    const int64_t *GB_RESTRICT Mh = NULL ;
    const int64_t *GB_RESTRICT Mi = NULL ;
    const GB_void *GB_RESTRICT Mx = NULL ;
    size_t msize = 0 ;
    int64_t mnvec = 0 ;
    bool M_is_hyper = false ;
    if (M != NULL)
    { 
        Mp = M->p ;
        Mh = M->h ;
        Mi = M->i ;
        Mx = (Mask_struct ? NULL : (M->x)) ;
        msize = M->type->size ;
        mnvec = M->nvec ;
        M_is_hyper = M->is_hyper ;
    }

    // 3 cases:
    //      M not present and Mask_comp false: compute C=A*B
    //      M present     and Mask_comp false: compute C<M>=A*B
    //      M present     and Mask_comp true : compute C<!M>=A*B
    // If M is NULL on input, then Mask_comp is also false on input.

    bool mask_is_M = (M != NULL && !Mask_comp) ;

    //==========================================================================
    // phase1: count nnz(C(:,j)) for coarse tasks, scatter M for fine tasks
    //==========================================================================

    // At this point, all of Hf [...] is zero, for all tasks.
    // Hi and Hx are not initialized.

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        int64_t hash_size = TaskList [taskid].hsize ;
        bool use_Gustavson = (hash_size == cvlen) ;

        if (taskid < nfine)
        {

            //------------------------------------------------------------------
            // no work for fine tasks in phase1 if M is not present
            //------------------------------------------------------------------

            if (M == NULL) continue ;

            //------------------------------------------------------------------
            // get the task descriptor
            //------------------------------------------------------------------
        
            int64_t kk = TaskList [taskid].vector ;
            // partition M(:,j)
            GB_GET_M_j ;        // get M(:,j)
            int team_size = TaskList [taskid].team_size ;
            int master    = TaskList [taskid].master ;
            int my_teamid = taskid - master ;
            int64_t mystart, myend ;
            GB_PARTITION (mystart, myend, mjnz, my_teamid, team_size) ;
            mystart += pM_start ;
            myend   += pM_start ;

            if (use_Gustavson)
            { 

                //--------------------------------------------------------------
                // phase1: fine Gustavson task, C<M>=A*B or C<!M>=A*B
                //--------------------------------------------------------------

                // Scatter the values of M(:,j) into Hf.  No atomics needed
                // since all indices i in M(;,j) are unique.

                uint8_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
                GB_SCATTER_M_j (mystart, myend, 1) ;

            }
            else
            {

                //--------------------------------------------------------------
                // phase1: fine hash task, C<M>=A*B or C<!M>=A*B
                //--------------------------------------------------------------

                // The least significant 2 bits of Hf [hash] is the flag f, and
                // the upper bits contain h, as (h,f).  After this phase1, if
                // M(i,j)=1 then the hash table contains ((i+1),1) in Hf [hash]
                // at some location.

                // Later, the flag values of f = 2 and 3 are also used.
                // Only f=1 is set in this phase.

                // h == 0,   f == 0: unoccupied and unlocked
                // h == i+1, f == 1: occupied with M(i,j)=1

                int64_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
                int64_t hash_bits = (hash_size-1) ;
                for (int64_t pM = mystart ; pM < myend ; pM++) // scan my M(:,j)
                {
                    GB_GET_M_ij ;                   // get M(i,j)
                    if (!mij) continue ;            // skip if M(i,j)=0
                    int64_t i = Mi [pM] ;
                    int64_t i_mine = ((i+1) << 2) + 1 ;  // ((i+1),1)
                    for (GB_HASH (i))
                    { 
                        int64_t hf ;
                        // swap my hash entry into the hash table
                        GB_ATOMIC_CAPTURE
                        {
                            hf = Hf [hash] ; Hf [hash] = i_mine ;
                        }
                        if (hf == 0) break ;        // success
                        // i_mine has been inserted, but a prior entry was
                        // already there.  It needs to be replaced, so take
                        // ownership of this displaced entry, and keep
                        // looking until a new empty slot is found for it.
                        i_mine = hf ;
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // coarse tasks: compute nnz in each vector of A*B(:,kfirst:klast)
            //------------------------------------------------------------------

            int64_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
            int64_t kfirst = TaskList [taskid].start ;
            int64_t klast  = TaskList [taskid].end ;
            int64_t mark = 0 ;
            // int64_t nk = klast - kfirst + 1 ;

            if (use_Gustavson)
            {

                //--------------------------------------------------------------
                // phase1: coarse Gustavson task
                //--------------------------------------------------------------

                if (M == NULL)
                {

                    //----------------------------------------------------------
                    // phase1: coarse Gustavson task, C=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all Hf.
                    // Hf [i] is set to mark when C(i,j) is found.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        GB_GET_B_j ;            // get B(:,j)
                        if (bjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        if (bjnz == 1)
                        { 
                            int64_t k = Bi [pB] ;   // get B(k,j)
                            GB_GET_A_k ;            // get A(:,k)
                            Cp [kk] = aknz ;        // nnz(C(:,j)) = nnz(A(:,k))
                            continue ;
                        }
                        mark++ ;
                        int64_t cjnz = 0 ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        {
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            if (aknz == cvlen)
                            { 
                                cjnz = cvlen ;  // A(:,k) is dense
                                break ;         // so nnz(C(:,j)) = cvlen
                            }
                            // scan A(:,k)
                            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                            {
                                int64_t i = Ai [pA] ;    // get A(i,k)
                                if (Hf [i] != mark)     // if true, i is new
                                { 
                                    Hf [i] = mark ; // mark C(i,j) as seen
                                    cjnz++ ;        // C(i,j) is a new entry
                                }
                            }
                        }
                        Cp [kk] = cjnz ;    // count the entries in C(:,j)
                    }

                }
                else if (mask_is_M)
                {

                    //----------------------------------------------------------
                    // phase1: coarse Gustavson task, C<M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.

                    // Hf [i] < mark    : M(i,j)=0, C(i,j) is ignored.
                    // Hf [i] == mark   : M(i,j)=1, and C(i,j) not yet seen.
                    // Hf [i] == mark+1 : M(i,j)=1, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        GB_GET_B_j ;            // get B(:,j)
                        if (bjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        GB_GET_M_j ;            // get M(:,j)
                        if (mjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        GB_GET_M_j_RANGE (64) ; // get first and last in M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        // scatter M(:,j)
                        GB_SCATTER_M_j (pM_start, pM_end, mark) ;
                        int64_t cjnz = 0 ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        { 
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                            #define GB_IKJ_VECTORIZE GB_PRAGMA_VECTORIZE
                            #define GB_IKJ_IVDEP     GB_PRAGMA_IVDEP
                            #define GB_IKJ                                     \
                            {                                                  \
                                if (Hf [i] == mark)   /* if true, M(i,j) is 1*/\
                                {                                              \
                                    Hf [i] = mark1 ;  /* mark C(i,j) as seen */\
                                    cjnz++ ;          /* C(i,j) is new */      \
                                }                                              \
                            }
                            GB_SCAN_M_j_OR_A_k ;
                            #undef GB_IKJ_VECTORIZE
                            #undef GB_IKJ_IVDEP
                            #undef GB_IKJ
                        }
                        Cp [kk] = cjnz ;    // count the entries in C(:,j)
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // phase1: coarse Gustavson task, C<!M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.

                    // Hf [i] < mark    : M(i,j)=0, C(i,j) is not yet seen.
                    // Hf [i] == mark   : M(i,j)=1, so C(i,j) is ignored.
                    // Hf [i] == mark+1 : M(i,j)=0, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        GB_GET_B_j ;                    // get B(:,j)
                        if (bjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        GB_GET_M_j ;            // get M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        // scatter M(:,j)
                        GB_SCATTER_M_j (pM_start, pM_end, mark) ;
                        int64_t cjnz = 0 ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        {
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            // scan A(:,k)
                            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                            {
                                int64_t i = Ai [pA] ;   // get A(i,k)
                                if (Hf [i] < mark)      // if true, M(i,j) is 0
                                { 
                                    Hf [i] = mark1 ;    // mark C(i,j) as seen
                                    cjnz++ ;            // C(i,j) is a new entry
                                }
                            }
                        }
                        Cp [kk] = cjnz ;    // count the entries in C(:,j)
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // phase1: coarse hash task
                //--------------------------------------------------------------

                int64_t *GB_RESTRICT Hi = TaskList [taskid].Hi ;
                int64_t hash_bits = (hash_size-1) ;

                if (M == NULL)
                {

                    //----------------------------------------------------------
                    // phase1: coarse hash task, C=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.
                    // Let f = Hf [hash] and h = Hi [hash]

                    // f < mark          : unoccupied.
                    // h == i, f == mark : occupied with C(i,j)

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        GB_GET_B_j ;            // get B(:,j)
                        if (bjnz == 0)
                        { 
                            Cp [kk] = 0 ; continue ;
                        }
                        if (bjnz == 1)
                        { 
                            int64_t k = Bi [pB] ;   // get B(k,j)
                            GB_GET_A_k ;            // get A(:,k)
                            Cp [kk] = aknz ;        // nnz(C(:,j)) = nnz(A(:,k))
                            continue ;
                        }
                        mark++ ;
                        int64_t cjnz = 0 ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        {
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            // scan A(:,k)
                            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                            {
                                int64_t i = Ai [pA] ;   // get A(i,k)
                                for (GB_HASH (i))       // find i in hash
                                {
                                    if (Hf [hash] < mark)
                                    { 
                                        Hf [hash] = mark ; // insert C(i,j)
                                        Hi [hash] = i ;
                                        cjnz++ ;  // C(i,j) is a new entry.
                                        break ;
                                    }
                                    if (Hi [hash] == i) break ;
                                }
                            }
                        }
                        Cp [kk] = cjnz ;    // count the entries in C(:,j)
                    }

                }
                else if (mask_is_M)
                {

                    //----------------------------------------------------------
                    // phase1: hash task, C<M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.
                    // Let h = Hi [hash] and f = Hf [hash].

                    // f < mark: unoccupied, M(i,j)=0, C(i,j) ignored if
                    //           this case occurs while scanning A(:,k)
                    // h == i, f == mark   : M(i,j)=1, and C(i,j) not yet seen.
                    // h == i, f == mark+1 : M(i,j)=1, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        GB_GET_B_j ;            // get B(:,j)
                        if (bjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        GB_GET_M_j ;            // get M(:,j)
                        if (mjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        GB_GET_M_j_RANGE (64) ; // get first and last in M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        GB_HASH_M_j ;           // hash M(:,j)
                        int64_t cjnz = 0 ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        { 
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                            #define GB_IKJ_VECTORIZE
                            #define GB_IKJ_IVDEP
                            #define GB_IKJ                                     \
                            {                                                  \
                                for (GB_HASH (i))       /* find i in hash */   \
                                {                                              \
                                    int64_t f = Hf [hash] ;                    \
                                    if (f < mark) break ; /* M(i,j)=0; ignore*/\
                                    if (Hi [hash] == i)   /* if true, i found*/\
                                    {                                          \
                                        if (f == mark)  /* if true, i is new */\
                                        {                                      \
                                            Hf [hash] = mark1 ; /* mark seen */\
                                            cjnz++ ;    /* C(i,j) is new */    \
                                        }                                      \
                                        break ;                                \
                                    }                                          \
                                }                                              \
                            }
                            GB_SCAN_M_j_OR_A_k ;
                            #undef GB_IKJ_VECTORIZE
                            #undef GB_IKJ_IVDEP
                            #undef GB_IKJ
                        }
                        Cp [kk] = cjnz ;    // count the entries in C(:,j)
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // phase1: coarse hash task, C<!M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.
                    // Let h = Hi [hash] and f = Hf [hash].

                    // f < mark: unoccupied, M(i,j)=0, and C(i,j) not yet seen.
                    // h == i, f == mark   : M(i,j)=1. C(i,j) ignored.
                    // h == i, f == mark+1 : M(i,j)=0, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        GB_GET_B_j ;            // get B(:,j)
                        if (bjnz == 0)
                        { 
                            Cp [kk] = 0 ;
                            continue ;
                        }
                        GB_GET_M_j ;            // get M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        GB_HASH_M_j ;           // hash M(:,j)
                        int64_t cjnz = 0 ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        {
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            // scan A(:,k)
                            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                            {
                                int64_t i = Ai [pA] ;   // get A(i,k)
                                for (GB_HASH (i))       // find i in hash
                                {
                                    if (Hf [hash] < mark)   // if true, i is new
                                    { 
                                        Hf [hash] = mark1 ; // mark C(i,j) seen
                                        Hi [hash] = i ;
                                        cjnz++ ;        // C(i,j) is a new entry
                                        break ;
                                    }
                                    if (Hi [hash] == i) break ;
                                }
                            }
                        }
                        Cp [kk] = cjnz ;    // count the entries in C(:,j)
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // check result for phase1 for fine tasks
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    if (M != NULL)
    {
        for (taskid = 0 ; taskid < nfine ; taskid++)
        {
            int64_t kk = TaskList [taskid].vector ;
            ASSERT (kk >= 0 && kk < B->nvec) ;
            int64_t hash_size = TaskList [taskid].hsize ;
            bool use_Gustavson = (hash_size == cvlen) ;
            int master = TaskList [taskid].master ;
            if (master != taskid) continue ;
            GB_GET_M_j ;        // get M(:,j)
            int64_t mjcount2 = 0 ;
            int64_t mjcount = 0 ;
            for (int64_t pM = pM_start ; pM < pM_end ; pM++)
            {
                GB_GET_M_ij ;           // get M(i,j)
                if (mij) mjcount++ ;
            }
            if (use_Gustavson)
            {
                // phase1: fine Gustavson task, C<M>=A*B or C<!M>=A*B
                uint8_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    GB_GET_M_ij ;                    // get M(i,j)
                    ASSERT (Hf [Mi [pM]] == mij) ;
                }
                for (int64_t i = 0 ; i < cvlen ; i++)
                {
                    ASSERT (Hf [i] == 0 || Hf [i] == 1) ;
                    if (Hf [i] == 1) mjcount2++ ;
                }
                ASSERT (mjcount == mjcount2) ;
            }
            else
            {
                // phase1: fine hash task, C<M>=A*B or C<!M>=A*B
                // h == 0,   f == 0: unoccupied and unlocked
                // h == i+1, f == 1: occupied with M(i,j)=1
                int64_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
                int64_t hash_bits = (hash_size-1) ;
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    GB_GET_M_ij ;                   // get M(i,j)
                    if (!mij) continue ;            // skip if M(i,j)=0
                    int64_t i = Mi [pM] ;
                    int64_t i_mine = ((i+1) << 2) + 1 ;  // ((i+1),1)
                    int64_t probe = 0 ;
                    for (GB_HASH (i))
                    {
                        int64_t hf = Hf [hash] ;
                        if (hf == i_mine) 
                        {
                            mjcount2++ ;
                            break ;
                        }
                        ASSERT (hf != 0) ;
                        probe++ ;
                        ASSERT (probe < cvlen) ;
                    }
                }
                ASSERT (mjcount == mjcount2) ;
                mjcount2 = 0 ;
                for (int64_t hash = 0 ; hash < hash_size ; hash++)
                {
                    int64_t hf = Hf [hash] ;
                    int64_t h = (hf >> 2) ;     // empty (0), or a 1-based 
                    int64_t f = (hf & 3) ;      // 0 if empty or 1 if occupied
                    if (f == 1) ASSERT (h >= 1 && h <= cvlen) ;
                    ASSERT (hf == 0 || f == 1) ;
                    if (f == 1) mjcount2++ ;
                }
                ASSERT (mjcount == mjcount2) ;
            }
        }
    }
    #endif
}


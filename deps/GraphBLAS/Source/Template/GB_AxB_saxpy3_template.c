//------------------------------------------------------------------------------
// GB_AxB_saxpy3_template: C=A*B, C<M>=A*B, or C<!M>=A*B via saxpy3 method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_AxB_saxpy3_template.c computes C=A*B for any semiring and matrix types.
// It is #include'd in GB_AxB_saxpy3 to construct the generic method (for
// arbitary user-defined operators and/or typecasting), and in the hard-coded
// GB_Asaxpy3B* workers in the Generated/ folder.

#include "GB_unused.h"

//------------------------------------------------------------------------------
// template code for C=A*B via the saxpy3 method
//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get the chunk size
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get M, A, B, and C
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Cp = C->p ;
    // const int64_t *GB_RESTRICT Ch = C->h ;
    const int64_t cvlen = C->vlen ;
    const int64_t cnvec = C->nvec ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    const GB_BTYPE *GB_RESTRICT Bx = B_is_pattern ? NULL : B->x ;
    // const int64_t bvlen = B->vlen ;
    // const int64_t bnvec = B->nvec ;
    // const bool B_is_hyper = B->is_hyper ;

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const int64_t anvec = A->nvec ;
    const bool A_is_hyper = GB_IS_HYPER (A) ;
    const GB_ATYPE *GB_RESTRICT Ax = A_is_pattern ? NULL : A->x ;

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
    // phase2: numeric work for fine tasks
    //==========================================================================

    // Coarse tasks: nothing to do in phase2.
    // Fine tasks: compute nnz (C(:,j)), and values in Hx via atomics.

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < nfine ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        int64_t kk = TaskList [taskid].vector ;
        int64_t hash_size = TaskList [taskid].hsize ;
        bool use_Gustavson = (hash_size == cvlen) ;
        int64_t pB     = TaskList [taskid].start ;
        int64_t pB_end = TaskList [taskid].end + 1 ;
        #if !GB_IS_ANY_PAIR_SEMIRING
        GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *) TaskList [taskid].Hx ;
        #endif
        int64_t pleft = 0, pright = anvec-1 ;

        if (use_Gustavson)
        {

            //------------------------------------------------------------------
            // phase2: fine Gustavson task
            //------------------------------------------------------------------

            // Hf [i] == 0: unlocked, i has not been seen in C(:,j).
            //      Hx [i] is not initialized.
            //      M(i,j) is 0, or M is not present.
            //      if M: Hf [i] stays equal to 0 (or 3 if locked)
            //      if !M, or no M: C(i,j) is a new entry seen for 1st time

            // Hf [i] == 1: unlocked, i has not been seen in C(:,j).
            //      Hx [i] is not initialized.  M is present.
            //      M(i,j) is 1. (either M or !M case)
            //      if M: C(i,j) is a new entry seen for the first time.
            //      if !M: Hf [i] stays equal to 1 (or 3 if locked)

            // Hf [i] == 2: unlocked, i has been seen in C(:,j).
            //      Hx [i] is initialized.  This case is independent of M.

            // Hf [i] == 3: locked.  Hx [i] cannot be accessed.

            uint8_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // phase2: fine Gustavson task, C=A*B
                //--------------------------------------------------------------

                // Hf [i] is initially 0.

                // 0 -> 3 : to lock, if i seen for first time
                // 2 -> 3 : to lock, if i seen already
                // 3 -> 2 : to unlock; now i has been seen

                for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                {
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    GB_GET_A_k ;                // get A(:,k)
                    if (aknz == 0) continue ;
                    GB_GET_B_kj ;               // bkj = B(k,j)
                    // scan A(:,k)
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;    // get A(i,k)
                        GB_MULT_A_ik_B_kj ;      // t = A(i,k) * B(k,j)
                        uint8_t f ;

                        #if GB_IS_ANY_MONOID

                        GB_ATOMIC_READ
                        f = Hf [i] ;            // grab the entry
                        if (f == 2) continue ;  // check if already updated
                        GB_ATOMIC_WRITE
                        Hf [i] = 2 ;                // flag the entry
                        GB_ATOMIC_WRITE_HX (i, t) ;    // Hx [i] = t

                        #else

                        #if GB_HAS_ATOMIC
                        GB_ATOMIC_READ
                        f = Hf [i] ;            // grab the entry
                        if (f == 2)             // if true, update C(i,j)
                        { 
                            GB_ATOMIC_UPDATE_HX (i, t) ;   // Hx [i] += t
                            continue ;          // C(i,j) has been updated
                        }
                        #endif
                        do  // lock the entry
                        {
                            GB_ATOMIC_CAPTURE
                            {
                                f = Hf [i] ; Hf [i] = 3 ;
                            }
                        } while (f == 3) ; // lock owner gets f=0 or 2
                        if (f == 0)
                        { 
                            // C(i,j) is a new entry
                            GB_ATOMIC_WRITE_HX (i, t) ;    // Hx [i] = t
                        }
                        else // f == 2
                        { 
                            // C(i,j) already appears in C(:,j)
                            GB_ATOMIC_UPDATE_HX (i, t) ;   // Hx [i] += t
                        }
                        GB_ATOMIC_WRITE
                        Hf [i] = 2 ;                // unlock the entry

                        #endif
                    }
                }

            }
            else if (mask_is_M)
            {

                //--------------------------------------------------------------
                // phase2: fine Gustavson task, C<M>=A*B
                //--------------------------------------------------------------

                // Hf [i] is 0 if M(i,j) not present or M(i,j)=0.
                // 0 -> 1 : has already been done in phase0 if M(i,j)=1

                // 0 -> 0 : to ignore, if M(i,j)=0
                // 1 -> 3 : to lock, if i seen for first time
                // 2 -> 3 : to lock, if i seen already
                // 3 -> 2 : to unlock; now i has been seen

                GB_GET_M_j ;                // get M(:,j)
                GB_GET_M_j_RANGE (16) ;     // get first and last in M(:,j)
                for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                { 
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    GB_GET_A_k ;                // get A(:,k)
                    GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                    GB_GET_B_kj ;               // bkj = B(k,j)

                    #if GB_IS_ANY_MONOID

                    #define GB_IKJ                                             \
                        uint8_t f ;                                            \
                        GB_ATOMIC_READ                                         \
                        f = Hf [i] ;            /* grab the entry */           \
                        if (f == 0 || f == 2) continue ;                       \
                        GB_ATOMIC_WRITE                                        \
                        Hf [i] = 2 ;            /* unlock the entry */         \
                        GB_MULT_A_ik_B_kj ;     /* t = A(i,k) * B(k,j) */      \
                        GB_ATOMIC_WRITE_HX (i, t) ;    /* Hx [i] = t */

                    #else

                    #define GB_IKJ                                             \
                    {                                                          \
                        GB_MULT_A_ik_B_kj ;     /* t = A(i,k) * B(k,j) */      \
                        uint8_t f ;                                            \
                        GB_ATOMIC_READ                                         \
                        f = Hf [i] ;            /* grab the entry */           \
                        if (GB_HAS_ATOMIC && (f == 2))                         \
                        {                                                      \
                            /* C(i,j) already seen; update it */               \
                            GB_ATOMIC_UPDATE_HX (i, t) ; /* Hx [i] += t */     \
                            continue ;       /* C(i,j) has been updated */     \
                        }                                                      \
                        if (f == 0) continue ; /* M(i,j)=0; ignore C(i,j)*/    \
                        do  /* lock the entry */                               \
                        {                                                      \
                            GB_ATOMIC_CAPTURE                                  \
                            {                                                  \
                                f = Hf [i] ; Hf [i] = 3 ;                      \
                            }                                                  \
                        } while (f == 3) ; /* lock owner gets f=1 or 2 */      \
                        if (f == 1)                                            \
                        {                                                      \
                            /* C(i,j) is a new entry */                        \
                            GB_ATOMIC_WRITE_HX (i, t) ; /* Hx [i] = t */       \
                        }                                                      \
                        else /* f == 2 */                                      \
                        {                                                      \
                            /* C(i,j) already appears in C(:,j) */             \
                            GB_ATOMIC_UPDATE_HX (i, t) ; /* Hx [i] += t */     \
                        }                                                      \
                        GB_ATOMIC_WRITE                                        \
                        Hf [i] = 2 ;                /* unlock the entry */     \
                    }
                    #endif

                    #define GB_IKJ_VECTORIZE
                    #define GB_IKJ_IVDEP
                    GB_SCAN_M_j_OR_A_k ;
                    #undef GB_IKJ_VECTORIZE
                    #undef GB_IKJ_IVDEP
                    #undef GB_IKJ
                }

            }
            else
            {

                //--------------------------------------------------------------
                // phase2: fine Gustavson task, C<!M>=A*B
                //--------------------------------------------------------------

                // Hf [i] is 0 if M(i,j) not present or M(i,j)=0.
                // 0 -> 1 : has already been done in phase0 if M(i,j)=1

                // 1 -> 1 : to ignore, if M(i,j)=1
                // 0 -> 3 : to lock, if i seen for first time
                // 2 -> 3 : to lock, if i seen already
                // 3 -> 2 : to unlock; now i has been seen

                for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                {
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    GB_GET_A_k ;                // get A(:,k)
                    if (aknz == 0) continue ;
                    GB_GET_B_kj ;               // bkj = B(k,j)
                    // scan A(:,k)
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;   // get A(i,k)
                        GB_MULT_A_ik_B_kj ;     // t = A(i,k) * B(k,j)
                        uint8_t f ;

                        #if GB_IS_ANY_MONOID

                        GB_ATOMIC_READ
                        f = Hf [i] ;            // grab the entry
                        if (f == 1 || f == 2) continue ;
                        GB_ATOMIC_WRITE
                        Hf [i] = 2 ;                // unlock the entry
                        GB_ATOMIC_WRITE_HX (i, t) ;    // Hx [i] = t

                        #else

                        GB_ATOMIC_READ
                        f = Hf [i] ;            // grab the entry
                        #if GB_HAS_ATOMIC
                        if (f == 2)             // if true, update C(i,j)
                        { 
                            GB_ATOMIC_UPDATE_HX (i, t) ;   // Hx [i] += t
                            continue ;          // C(i,j) has been updated
                        }
                        #endif
                        if (f == 1) continue ; // M(i,j)=1; ignore C(i,j)
                        do  // lock the entry
                        {
                            GB_ATOMIC_CAPTURE
                            {
                                f = Hf [i] ; Hf [i] = 3 ;
                            }
                        } while (f == 3) ; // lock owner of gets f=0 or 2
                        if (f == 0)
                        { 
                            // C(i,j) is a new entry
                            GB_ATOMIC_WRITE_HX (i, t) ;    // Hx [i] = t
                        }
                        else // f == 2
                        { 
                            // C(i,j) already seen
                            GB_ATOMIC_UPDATE_HX (i, t) ;   // Hx [i] += t
                        }
                        GB_ATOMIC_WRITE
                        Hf [i] = 2 ;                // unlock the entry
                        #endif
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // phase2: fine hash task
            //------------------------------------------------------------------

            // Each hash entry Hf [hash] splits into two parts, (h,f).  f
            // is in the 2 least significant bits.  h is 62 bits, and is
            // the 1-based index i of the C(i,j) entry stored at that
            // location in the hash table.

            // If M is present (M or !M), and M(i,j)=1, then (i+1,1)
            // has been inserted into the hash table, in phase0.

            // Given Hf [hash] split into (h,f)

            // h == 0, f == 0: unlocked and unoccupied.
            //                  note that if f=0, h must be zero too.

            // h == i+1, f == 1: unlocked, occupied by M(i,j)=1.
            //                  C(i,j) has not been seen, or is ignored.
            //                  Hx is not initialized.  M is present.
            //                  if !M: this entry will be ignored in C.

            // h == i+1, f == 2: unlocked, occupied by C(i,j).
            //                  Hx is initialized.  M is no longer
            //                  relevant.

            // h == (anything), f == 3: locked.

            int64_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
            int64_t hash_bits = (hash_size-1) ;

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // phase2: fine hash task, C=A*B
                //--------------------------------------------------------------

                // Given Hf [hash] split into (h,f)

                // h == 0  , f == 0 : unlocked and unoccupied.
                // h == i+1, f == 2 : unlocked, occupied by C(i,j).
                //                    Hx is initialized.
                // h == ..., f == 3 : locked.

                // 0 -> 3 : to lock, if i seen for first time
                // 2 -> 3 : to lock, if i seen already
                // 3 -> 2 : to unlock; now i has been seen

                for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                {
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    GB_GET_A_k ;                // get A(:,k)
                    if (aknz == 0) continue ;
                    GB_GET_B_kj ;               // bkj = B(k,j)
                    // scan A(:,k)
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;       // get A(i,k)
                        GB_MULT_A_ik_B_kj ;         // t = A(i,k) * B(k,j)
                        int64_t i1 = i + 1 ;        // i1 = one-based index
                        int64_t i_unlocked = (i1 << 2) + 2 ;    // (i+1,2)
                        for (GB_HASH (i))           // find i in hash table
                        {
                            int64_t hf ;
                            GB_ATOMIC_READ
                            hf = Hf [hash] ;        // grab the entry
                            #if GB_HAS_ATOMIC
                            if (hf == i_unlocked)  // if true, update C(i,j)
                            { 
                                GB_ATOMIC_UPDATE_HX (hash, t) ;// Hx [.]+=t
                                break ;         // C(i,j) has been updated
                            }
                            #endif
                            int64_t h = (hf >> 2) ;
                            if (h == 0 || h == i1)
                            {
                                // h=0: unoccupied, h=i1: occupied by i
                                do  // lock the entry
                                {
                                    GB_ATOMIC_CAPTURE
                                    {
                                        hf = Hf [hash] ; Hf [hash] |= 3 ;
                                    }
                                } while ((hf & 3) == 3) ; // owner: f=0 or 2
                                if (hf == 0) // f == 0
                                { 
                                    // C(i,j) is a new entry in C(:,j)
                                    // Hx [hash] = t
                                    GB_ATOMIC_WRITE_HX (hash, t) ;
                                    GB_ATOMIC_WRITE
                                    Hf [hash] = i_unlocked ; // unlock entry
                                    break ;
                                }
                                if (hf == i_unlocked) // f == 2
                                { 
                                    // C(i,j) already appears in C(:,j)
                                    // Hx [hash] += t
                                    GB_ATOMIC_UPDATE_HX (hash, t) ;
                                    GB_ATOMIC_WRITE
                                    Hf [hash] = i_unlocked ; // unlock entry
                                    break ;
                                }
                                // hash table occupied, but not with i
                                GB_ATOMIC_WRITE
                                Hf [hash] = hf ;  // unlock with prior value
                            }
                        }
                    }
                }

            }
            else if (mask_is_M)
            {

                //--------------------------------------------------------------
                // phase2: fine hash task, C<M>=A*B
                //--------------------------------------------------------------

                // Given Hf [hash] split into (h,f)

                // h == 0  , f == 0 : unlocked, unoccupied. C(i,j) ignored
                // h == i+1, f == 1 : unlocked, occupied by M(i,j)=1.
                //                    C(i,j) has not been seen.
                //                    Hx is not initialized.
                // h == i+1, f == 2 : unlocked, occupied by C(i,j), M(i,j)=1
                //                    Hx is initialized.
                // h == ..., f == 3 : locked.

                // 0 -> 0 : to ignore, if M(i,j)=0
                // 1 -> 3 : to lock, if i seen for first time
                // 2 -> 3 : to lock, if i seen already
                // 3 -> 2 : to unlock; now i has been seen

                GB_GET_M_j ;                // get M(:,j)
                GB_GET_M_j_RANGE (16) ;     // get first and last in M(:,j)
                for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                { 
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    GB_GET_A_k ;                // get A(:,k)
                    GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                    GB_GET_B_kj ;               // bkj = B(k,j)
                    #define GB_IKJ_VECTORIZE
                    #define GB_IKJ_IVDEP
                    #define GB_IKJ                                             \
                    {                                                          \
                        GB_MULT_A_ik_B_kj ;      /* t = A(i,k) * B(k,j) */     \
                        int64_t i1 = i + 1 ;     /* i1 = one-based index */    \
                        int64_t i_unlocked = (i1 << 2) + 2 ;  /* (i+1,2) */    \
                        for (GB_HASH (i))        /* find i in hash table */    \
                        {                                                      \
                            int64_t hf ;                                       \
                            GB_ATOMIC_READ                                     \
                            hf = Hf [hash] ;        /* grab the entry */       \
                            if (GB_HAS_ATOMIC && (hf == i_unlocked))           \
                            {                                                  \
                                /* Hx [hash] += t */                           \
                                GB_ATOMIC_UPDATE_HX (hash, t) ;                \
                                break ;     /* C(i,j) has been updated */      \
                            }                                                  \
                            if (hf == 0) break ; /* M(i,j)=0; ignore Cij */    \
                            if ((hf >> 2) == i1) /* if true, i found */        \
                            {                                                  \
                                do /* lock the entry */                        \
                                {                                              \
                                    GB_ATOMIC_CAPTURE                          \
                                    {                                          \
                                        hf = Hf [hash] ; Hf [hash] |= 3 ;      \
                                    }                                          \
                                } while ((hf & 3) == 3) ; /* own: f=1,2 */     \
                                if ((hf & 3) == 1) /* f == 1 */                \
                                {                                              \
                                    /* C(i,j) is a new entry in C(:,j) */      \
                                    /* Hx [hash] = t */                        \
                                    GB_ATOMIC_WRITE_HX (hash, t) ;             \
                                }                                              \
                                else /* f == 2 */                              \
                                {                                              \
                                    /* C(i,j) already appears in C(:,j) */     \
                                    /* Hx [hash] += t */                       \
                                    GB_ATOMIC_UPDATE_HX (hash, t) ;            \
                                }                                              \
                                GB_ATOMIC_WRITE                                \
                                Hf [hash] = i_unlocked ; /* unlock entry */    \
                                break ;                                        \
                            }                                                  \
                        }                                                      \
                    }
                    GB_SCAN_M_j_OR_A_k ;
                    #undef GB_IKJ_VECTORIZE
                    #undef GB_IKJ_IVDEP
                    #undef GB_IKJ
                }

            }
            else
            { 

                //--------------------------------------------------------------
                // phase2: fine hash task, C<!M>=A*B
                //--------------------------------------------------------------

                // Given Hf [hash] split into (h,f)

                // h == 0  , f == 0 : unlocked and unoccupied.
                // h == i+1, f == 1 : unlocked, occupied by M(i,j)=1.
                //                    C(i,j) is ignored.
                // h == i+1, f == 2 : unlocked, occupied by C(i,j).
                //                    Hx is initialized.

                // h == (anything), f == 3: locked.

                // 1 -> 1 : to ignore, if M(i,j)=1
                // 0 -> 3 : to lock, if i seen for first time
                // 2 -> 3 : to lock, if i seen already
                // 3 -> 2 : to unlock; now i has been seen

                for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                {
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    GB_GET_A_k ;                // get A(:,k)
                    if (aknz == 0) continue ;
                    GB_GET_B_kj ;               // bkj = B(k,j)
                    // scan A(:,k)
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;       // get A(i,k)
                        GB_MULT_A_ik_B_kj ;         // t = A(i,k) * B(k,j)
                        int64_t i1 = i + 1 ;        // i1 = one-based index
                        int64_t i_unlocked = (i1 << 2) + 2 ;    // (i+1,2)
                        int64_t i_masked   = (i1 << 2) + 1 ;    // (i+1,1)
                        for (GB_HASH (i))           // find i in hash table
                        {
                            int64_t hf ;
                            GB_ATOMIC_READ
                            hf = Hf [hash] ;        // grab the entry
                            #if GB_HAS_ATOMIC
                            if (hf == i_unlocked)  // if true, update C(i,j)
                            { 
                                GB_ATOMIC_UPDATE_HX (hash, t) ;// Hx [.]+=t
                                break ;         // C(i,j) has been updated
                            }
                            #endif
                            if (hf == i_masked) break ; // M(i,j)=1; ignore
                            int64_t h = (hf >> 2) ;
                            if (h == 0 || h == i1)
                            {
                                // h=0: unoccupied, h=i1: occupied by i
                                do // lock the entry
                                {
                                    GB_ATOMIC_CAPTURE
                                    {
                                        hf = Hf [hash] ; Hf [hash] |= 3 ;
                                    }
                                } while ((hf & 3) == 3) ; // owner: f=0,1,2
                                if (hf == 0)            // f == 0
                                { 
                                    // C(i,j) is a new entry in C(:,j)
                                    // Hx [hash] = t
                                    GB_ATOMIC_WRITE_HX (hash, t) ;
                                    GB_ATOMIC_WRITE
                                    Hf [hash] = i_unlocked ; // unlock entry
                                    break ;
                                }
                                if (hf == i_unlocked)   // f == 2
                                { 
                                    // C(i,j) already appears in C(:,j)
                                    // Hx [hash] += t
                                    GB_ATOMIC_UPDATE_HX (hash, t) ;
                                    GB_ATOMIC_WRITE
                                    Hf [hash] = i_unlocked ; // unlock entry
                                    break ;
                                }
                                // hash table occupied, but not with i,
                                // or with i but M(i,j)=1 so C(i,j) ignored
                                GB_ATOMIC_WRITE
                                Hf [hash] = hf ;  // unlock with prior value
                            }
                        }
                    }
                }
            }
        }
    }

    //==========================================================================
    // phase3/phase4: count nnz(C(:,j)) for fine tasks, cumsum of Cp
    //==========================================================================

    int64_t cjnz_max = GB_AxB_saxpy3_cumsum (C, TaskList,
        nfine, chunk, nthreads) ;

    //==========================================================================
    // phase5: numeric phase for coarse tasks, gather for fine tasks
    //==========================================================================

    // allocate Ci and Cx
    int64_t cnz = Cp [cnvec] ;
    GrB_Info info = GB_ix_alloc (C, cnz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    int64_t  *GB_RESTRICT Ci = C->i ;
    GB_CTYPE *GB_RESTRICT Cx = C->x ;

    #if GB_IS_ANY_PAIR_SEMIRING

        // ANY_PAIR semiring: result is purely symbolic
        int64_t pC ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (pC = 0 ; pC < cnz ; pC++)
        { 
            Cx [pC] = 1 ;
        }

        // Just a precaution; these variables are not used below.  Any attempt
        // to access them will lead to a compile error.
        #define Cx is not used
        #define Hx is not used

        // these have been renamed to ANY_PAIR:
        // EQ_PAIR
        // LAND_PAIR
        // LOR_PAIR
        // MAX_PAIR
        // MIN_PAIR
        // TIMES_PAIR

    #endif

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        #if !GB_IS_ANY_PAIR_SEMIRING
        GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *) TaskList [taskid].Hx ;
        #endif
        int64_t hash_size = TaskList [taskid].hsize ;
        bool use_Gustavson = (hash_size == cvlen) ;

        if (taskid < nfine)
        {

            //------------------------------------------------------------------
            // fine task: gather pattern and values
            //------------------------------------------------------------------

            int64_t kk = TaskList [taskid].vector ;
            int team_size = TaskList [taskid].team_size ;
            int master    = TaskList [taskid].master ;
            int my_teamid = taskid - master ;
            int64_t pC = Cp [kk] ;

            if (use_Gustavson)
            {

                //--------------------------------------------------------------
                // phase5: fine Gustavson task, C=A*B, C<M>=A*B, or C<!M>=A*B
                //--------------------------------------------------------------

                // Hf [i] == 2 if C(i,j) is an entry in C(:,j)
                uint8_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;

                int64_t cjnz = Cp [kk+1] - pC ;
                int64_t istart, iend ;
                GB_PARTITION (istart, iend, cvlen, my_teamid, team_size) ;
                if (cjnz == cvlen)
                {
                    // C(:,j) is dense
                    for (int64_t i = istart ; i < iend ; i++)
                    { 
                        Ci [pC + i] = i ;
                    }
                    #if !GB_IS_ANY_PAIR_SEMIRING
                    // copy Hx [istart:iend-1] into Cx [pC+istart:pC+iend-1]
                    GB_CIJ_MEMCPY (pC + istart, istart, iend - istart) ;
                    #endif
                }
                else
                {
                    // C(:,j) is sparse
                    pC += TaskList [taskid].my_cjnz ;
                    for (int64_t i = istart ; i < iend ; i++)
                    {
                        if (Hf [i] == 2)
                        { 
                            #if !GB_IS_ANY_PAIR_SEMIRING
                            GB_CIJ_GATHER (pC, i) ; // Cx [pC] = Hx [i]
                            #endif
                            Ci [pC++] = i ;
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // phase5: fine hash task, C=A*B, C<M>=A*B, C<!M>=A*B
                //--------------------------------------------------------------

                // (Hf [hash] & 3) == 2 if C(i,j) is an entry in C(:,j),
                // and the index i of the entry is (Hf [hash] >> 2) - 1.

                int64_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
                int64_t mystart, myend ;
                GB_PARTITION (mystart, myend, hash_size, my_teamid, team_size) ;
                pC += TaskList [taskid].my_cjnz ;
                for (int64_t hash = mystart ; hash < myend ; hash++)
                {
                    int64_t hf = Hf [hash] ;
                    if ((hf & 3) == 2)
                    { 
                        int64_t i = (hf >> 2) - 1 ; // found C(i,j) in hash
                        Ci [pC++] = i ;
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // numeric coarse task: compute C(:,kfirst:klast)
            //------------------------------------------------------------------

            int64_t *GB_RESTRICT Hf = TaskList [taskid].Hf ;
            int64_t kfirst = TaskList [taskid].start ;
            int64_t klast = TaskList [taskid].end ;
            int64_t nk = klast - kfirst + 1 ;
            int64_t mark = 2*nk + 1 ;

            if (use_Gustavson)
            {

                //--------------------------------------------------------------
                // phase5: coarse Gustavson task
                //--------------------------------------------------------------

                if (M == NULL)
                {

                    //----------------------------------------------------------
                    // phase5: coarse Gustavson task, C=A*B
                    //----------------------------------------------------------

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        int64_t pC = Cp [kk] ;
                        int64_t cjnz = Cp [kk+1] - pC ;
                        if (cjnz == 0) continue ;   // nothing to do
                        GB_GET_B_j ;                // get B(:,j)
                        mark++ ;
                        if (cjnz == cvlen)          // C(:,j) is dense
                        { 
                            GB_COMPUTE_DENSE_C_j ;  // C(:,j) = A*B(:,j)
                        }
                        else if (bjnz == 1)         // C(:,j) = A(:,k)*B(k,j)
                        { 
                            GB_COMPUTE_C_j_WHEN_NNZ_B_j_IS_ONE ;
                        }
                        else if (16 * cjnz > cvlen) // C(:,j) is not very sparse
                        {
                            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                            {
                                int64_t k = Bi [pB] ;       // get B(k,j)
                                GB_GET_A_k ;                // get A(:,k)
                                if (aknz == 0) continue ;
                                GB_GET_B_kj ;               // bkj = B(k,j)
                                // scan A(:,k)
                                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                                {
                                    int64_t i = Ai [pA] ;   // get A(i,k)
                                    GB_MULT_A_ik_B_kj ;     // t = A(i,k)*B(k,j)
                                    if (Hf [i] != mark)
                                    { 
                                        // C(i,j) = A(i,k) * B(k,j)
                                        Hf [i] = mark ;
                                        GB_HX_WRITE (i, t) ;    // Hx [i] = t
                                    }
                                    else
                                    { 
                                        // C(i,j) += A(i,k) * B(k,j)
                                        GB_HX_UPDATE (i, t) ;   // Hx [i] += t
                                    }
                                }
                            }
                            GB_GATHER_ALL_C_j(mark) ;   // gather into C(:,j) 
                        }
                        else    // C(:,j) is very sparse
                        {
                            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                            {
                                int64_t k = Bi [pB] ;       // get B(k,j)
                                GB_GET_A_k ;                // get A(:,k)
                                if (aknz == 0) continue ;
                                GB_GET_B_kj ;               // bkj = B(k,j)
                                // scan A(:,k)
                                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                                {
                                    int64_t i = Ai [pA] ;   // get A(i,k)
                                    GB_MULT_A_ik_B_kj ;     // t = A(i,k)*B(k,j)
                                    if (Hf [i] != mark)
                                    { 
                                        // C(i,j) = A(i,k) * B(k,j)
                                        Hf [i] = mark ;
                                        GB_HX_WRITE (i, t) ;    // Hx [i] = t
                                        Ci [pC++] = i ;
                                    }
                                    else
                                    { 
                                        // C(i,j) += A(i,k) * B(k,j)
                                        GB_HX_UPDATE (i, t) ;   // Hx [i] += t
                                    }
                                }
                            }
                            GB_SORT_AND_GATHER_C_j ;    // gather into C(:,j)
                        }
                    }

                }
                else if (mask_is_M)
                {

                    //----------------------------------------------------------
                    // phase5: coarse Gustavson task, C<M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.

                    // Hf [i] < mark    : M(i,j)=0, C(i,j) is ignored.
                    // Hf [i] == mark   : M(i,j)=1, and C(i,j) not yet seen.
                    // Hf [i] == mark+1 : M(i,j)=1, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        int64_t pC = Cp [kk] ;
                        int64_t cjnz = Cp [kk+1] - pC ;
                        if (cjnz == 0) continue ;   // nothing to do
                        GB_GET_B_j ;                // get B(:,j)
                        if (cjnz == cvlen)          // C(:,j) is dense
                        { 
                            GB_COMPUTE_DENSE_C_j ;  // C(:,j) = A*B(:,j)
                            continue ;              // no need to examine M(:,j)
                        }
                        GB_GET_M_j ;            // get M(:,j)
                        GB_GET_M_j_RANGE (64) ; // get first and last in M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        // scatter M(:,j)
                        GB_SCATTER_M_j (pM_start, pM_end, mark) ;
                        if (16 * cjnz > cvlen)  // C(:,j) is not very sparse
                        {
                            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                            { 
                                int64_t k = Bi [pB] ;       // get B(k,j)
                                GB_GET_A_k ;                // get A(:,k)
                                GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                                GB_GET_B_kj ;               // bkj = B(k,j)
                                #define GB_IKJ_VECTORIZE GB_PRAGMA_VECTORIZE
                                #define GB_IKJ_IVDEP     GB_PRAGMA_IVDEP
                                #define GB_IKJ                                 \
                                {                                              \
                                    int64_t hf = Hf [i] ;                      \
                                    if (hf == mark)                            \
                                    {                                          \
                                        /* C(i,j) = A(i,k) * B(k,j) */         \
                                        Hf [i] = mark1 ;     /* mark as seen */\
                                        GB_MULT_A_ik_B_kj ;  /* t = aik*bkj */ \
                                        GB_HX_WRITE (i, t) ; /* Hx [i] = t */  \
                                    }                                          \
                                    else if (hf == mark1)                      \
                                    {                                          \
                                        /* C(i,j) += A(i,k) * B(k,j) */        \
                                        GB_MULT_A_ik_B_kj ;  /* t = aik*bkj */ \
                                        GB_HX_UPDATE (i, t) ;/* Hx [i] += t */ \
                                    }                                          \
                                }
                                GB_SCAN_M_j_OR_A_k ;
                                #undef GB_IKJ_VECTORIZE
                                #undef GB_IKJ_IVDEP
                                #undef GB_IKJ
                            }
                            GB_GATHER_ALL_C_j(mark1) ;  // gather into C(:,j) 
                        }
                        else    // C(:,j) is very sparse
                        {
                            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                            { 
                                int64_t k = Bi [pB] ;       // get B(k,j)
                                GB_GET_A_k ;                // get A(:,k)
                                GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                                GB_GET_B_kj ;               // bkj = B(k,j)
                                #define GB_IKJ_VECTORIZE GB_PRAGMA_VECTORIZE
                                #define GB_IKJ_IVDEP     GB_PRAGMA_IVDEP
                                #define GB_IKJ                                 \
                                {                                              \
                                    int64_t hf = Hf [i] ;                      \
                                    if (hf == mark)                            \
                                    {                                          \
                                        /* C(i,j) = A(i,k) * B(k,j) */         \
                                        Hf [i] = mark1 ;     /* mark as seen */\
                                        GB_MULT_A_ik_B_kj ;  /* t = aik*bkj */ \
                                        GB_HX_WRITE (i, t) ; /* Hx [i] = t */  \
                                        Ci [pC++] = i ; /* C(:,j) pattern */   \
                                    }                                          \
                                    else if (hf == mark1)                      \
                                    {                                          \
                                        /* C(i,j) += A(i,k) * B(k,j) */        \
                                        GB_MULT_A_ik_B_kj ;  /* t = aik*bkj */ \
                                        GB_HX_UPDATE (i, t) ;/* Hx [i] += t */ \
                                    }                                          \
                                }
                                GB_SCAN_M_j_OR_A_k ;
                                #undef GB_IKJ_VECTORIZE
                                #undef GB_IKJ_IVDEP
                                #undef GB_IKJ
                            }
                            GB_SORT_AND_GATHER_C_j ;    // gather into C(:,j)
                        }
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // phase5: coarse Gustavson task, C<!M>=A*B
                    //----------------------------------------------------------

                    // if !M:
                    // Hf [i] < mark    : M(i,j)=0, C(i,j) is not yet seen.
                    // Hf [i] == mark   : M(i,j)=1, so C(i,j) is ignored.
                    // Hf [i] == mark+1 : M(i,j)=0, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        int64_t pC = Cp [kk] ;
                        int64_t cjnz = Cp [kk+1] - pC ;
                        if (cjnz == 0) continue ;   // nothing to do
                        GB_GET_B_j ;                // get B(:,j)
                        if (cjnz == cvlen)          // C(:,j) is dense
                        { 
                            GB_COMPUTE_DENSE_C_j ;  // C(:,j) = A*B(:,j)
                            continue ;              // no need to examine M(:,j)
                        }
                        GB_GET_M_j ;            // get M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        // scatter M(:,j)
                        GB_SCATTER_M_j (pM_start, pM_end, mark) ;
                        if (16 * cjnz > cvlen)  // C(:,j) is not very sparse
                        {
                            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                            {
                                int64_t k = Bi [pB] ;       // get B(k,j)
                                GB_GET_A_k ;                // get A(:,k)
                                if (aknz == 0) continue ;
                                GB_GET_B_kj ;               // bkj = B(k,j)
                                // scan A(:,k)
                                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                                {
                                    int64_t i = Ai [pA] ;   // get A(i,k)
                                    int64_t hf = Hf [i] ;
                                    if (hf < mark)
                                    { 
                                        // C(i,j) = A(i,k) * B(k,j)
                                        Hf [i] = mark1 ;     // mark as seen
                                        GB_MULT_A_ik_B_kj ;  // t =A(i,k)*B(k,j)
                                        GB_HX_WRITE (i, t) ; // Hx [i] = t
                                    }
                                    else if (hf == mark1)
                                    { 
                                        // C(i,j) += A(i,k) * B(k,j)
                                        GB_MULT_A_ik_B_kj ;  // t =A(i,k)*B(k,j)
                                        GB_HX_UPDATE (i, t) ;// Hx [i] += t
                                    }
                                }
                            }
                            GB_GATHER_ALL_C_j(mark1) ;  // gather into C(:,j) 
                        }
                        else    // C(:,j) is very sparse
                        {
                            for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                            {
                                int64_t k = Bi [pB] ;       // get B(k,j)
                                GB_GET_A_k ;                // get A(:,k)
                                if (aknz == 0) continue ;
                                GB_GET_B_kj ;               // bkj = B(k,j)
                                // scan A(:,k)
                                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                                {
                                    int64_t i = Ai [pA] ;   // get A(i,k)
                                    int64_t hf = Hf [i] ;
                                    if (hf < mark)
                                    { 
                                        // C(i,j) = A(i,k) * B(k,j)
                                        Hf [i] = mark1 ;        // mark as seen
                                        GB_MULT_A_ik_B_kj ;  // t =A(i,k)*B(k,j)
                                        GB_HX_WRITE (i, t) ;    // Hx [i] = t
                                        Ci [pC++] = i ; // create C(:,j) pattern
                                    }
                                    else if (hf == mark1)
                                    { 
                                        // C(i,j) += A(i,k) * B(k,j)
                                        GB_MULT_A_ik_B_kj ;  // t =A(i,k)*B(k,j)
                                        GB_HX_UPDATE (i, t) ;   // Hx [i] += t
                                    }
                                }
                            }
                            GB_SORT_AND_GATHER_C_j ;    // gather into C(:,j)
                        }
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // phase5: coarse hash task
                //--------------------------------------------------------------

                int64_t *GB_RESTRICT Hi = TaskList [taskid].Hi ;
                int64_t hash_bits = (hash_size-1) ;

                if (M == NULL)
                {

                    //----------------------------------------------------------
                    // phase5: coarse hash task, C=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.
                    // Let f = Hf [hash] and h = Hi [hash]

                    // f < mark          : unoccupied.
                    // h == i, f == mark : occupied with C(i,j)

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        int64_t pC = Cp [kk] ;
                        int64_t cjnz = Cp [kk+1] - pC ;
                        if (cjnz == 0) continue ;   // nothing to do
                        GB_GET_B_j ;                // get B(:,j)
                        if (bjnz == 1)              // C(:,j) = A(:,k)*B(k,j)
                        { 
                            GB_COMPUTE_C_j_WHEN_NNZ_B_j_IS_ONE ;
                            continue ;
                        }
                        mark++ ;
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        {
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            if (aknz == 0) continue ;
                            GB_GET_B_kj ;               // bkj = B(k,j)
                            // scan A(:,k)
                            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                            {
                                int64_t i = Ai [pA] ;   // get A(i,k)
                                GB_MULT_A_ik_B_kj ;     // t = A(i,k)*B(k,j)
                                for (GB_HASH (i))   // find i in hash table
                                {
                                    if (Hf [hash] == mark)
                                    {
                                        // hash entry is occupied
                                        if (Hi [hash] == i)
                                        { 
                                            // i already in the hash table
                                            // Hx [hash] += t ;
                                            GB_HX_UPDATE (hash, t) ;
                                            break ;
                                        }
                                    }
                                    else
                                    { 
                                        // hash entry is not occupied
                                        Hf [hash] = mark ;
                                        Hi [hash] = i ;
                                        GB_HX_WRITE (hash, t) ;// Hx[hash]=t
                                        Ci [pC++] = i ;
                                        break ;
                                    }
                                }
                            }
                        }
                        // found i if: Hf [hash] == mark and Hi [hash] == i
                        GB_SORT_AND_GATHER_HASHED_C_j (mark, Hi [hash] == i)
                    }

                }
                else if (mask_is_M)
                {

                    //----------------------------------------------------------
                    // phase5: coarse hash task, C<M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.
                    // Let h = Hi [hash] and f = Hf [hash].

                    // f < mark            : M(i,j)=0, C(i,j) is ignored.
                    // h == i, f == mark   : M(i,j)=1, and C(i,j) not yet seen.
                    // h == i, f == mark+1 : M(i,j)=1, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        int64_t pC = Cp [kk] ;
                        int64_t cjnz = Cp [kk+1] - pC ;
                        if (cjnz == 0) continue ;   // nothing to do
                        GB_GET_M_j ;                // get M(:,j)
                        GB_GET_M_j_RANGE (64) ;     // get 1st & last in M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        GB_HASH_M_j ;               // hash M(:,j)
                        GB_GET_B_j ;                // get B(:,j)
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        { 
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            GB_SKIP_IF_A_k_DISJOINT_WITH_M_j ;
                            GB_GET_B_kj ;               // bkj = B(k,j)
                            #define GB_IKJ_VECTORIZE
                            #define GB_IKJ_IVDEP
                            #define GB_IKJ                                     \
                            {                                                  \
                                for (GB_HASH (i))       /* find i in hash */   \
                                {                                              \
                                    int64_t f = Hf [hash] ;                    \
                                    if (f < mark) break ; /* M(i,j)=0, ignore*/\
                                    if (Hi [hash] == i)                        \
                                    {                                          \
                                        GB_MULT_A_ik_B_kj ; /* t = aik*bkj */  \
                                        if (f == mark) /* if true, i is new */ \
                                        {                                      \
                                            /* C(i,j) is new */                \
                                            Hf [hash] = mark1 ; /* mark seen */\
                                            GB_HX_WRITE (hash, t) ;/*Hx[.]=t */\
                                            Ci [pC++] = i ;                    \
                                        }                                      \
                                        else                                   \
                                        {                                      \
                                            /* C(i,j) has been seen; update */ \
                                            GB_HX_UPDATE (hash, t) ;           \
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
                        // found i if: Hf [hash] == mark1 and Hi [hash] == i
                        GB_SORT_AND_GATHER_HASHED_C_j (mark1, Hi [hash] == i) ;
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // phase5: coarse hash task, C<!M>=A*B
                    //----------------------------------------------------------

                    // Initially, Hf [...] < mark for all of Hf.
                    // Let h = Hi [hash] and f = Hf [hash].

                    // f < mark: unoccupied, M(i,j)=0, and C(i,j) not yet seen.
                    // h == i, f == mark   : M(i,j)=1. C(i,j) ignored.
                    // h == i, f == mark+1 : M(i,j)=0, and C(i,j) has been seen.

                    for (int64_t kk = kfirst ; kk <= klast ; kk++)
                    {
                        int64_t pC = Cp [kk] ;
                        int64_t cjnz = Cp [kk+1] - pC ;
                        if (cjnz == 0) continue ;   // nothing to do
                        GB_GET_M_j ;                // get M(:,j)
                        mark += 2 ;
                        int64_t mark1 = mark+1 ;
                        GB_HASH_M_j ;               // hash M(:,j)
                        GB_GET_B_j ;                // get B(:,j)
                        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
                        {
                            int64_t k = Bi [pB] ;       // get B(k,j)
                            GB_GET_A_k ;                // get A(:,k)
                            if (aknz == 0) continue ;
                            GB_GET_B_kj ;               // bkj = B(k,j)
                            // scan A(:,k)
                            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                            {
                                int64_t i = Ai [pA] ;   // get A(i,k)
                                for (GB_HASH (i))       // find i in hash
                                {
                                    int64_t f = Hf [hash] ;
                                    if (f < mark)   // if true, i is new
                                    { 
                                        // C(i,j) is new
                                        Hf [hash] = mark1 ; // mark C(i,j) seen
                                        Hi [hash] = i ;
                                        GB_MULT_A_ik_B_kj ; // t = A(i,k)*B(k,j)
                                        GB_HX_WRITE (hash, t) ; // Hx [hash] = t
                                        Ci [pC++] = i ;
                                        break ;
                                    }
                                    if (Hi [hash] == i)
                                    {
                                        if (f == mark1)
                                        { 
                                            // C(i,j) has been seen; update it.
                                            GB_MULT_A_ik_B_kj ;//t=A(i,k)*B(k,j)
                                            GB_HX_UPDATE (hash, t) ;//Hx[ ] += t
                                        }
                                        break ;
                                    }
                                }
                            }
                        }
                        // found i if: Hf [hash] == mark1 and Hi [hash] == i
                        GB_SORT_AND_GATHER_HASHED_C_j (mark1, Hi [hash] == i) ;
                    }
                }
            }
        }
    }

    //==========================================================================
    // phase6: final gather phase for fine hash tasks
    //==========================================================================

    if (cjnz_max > 0)
    {
        int64_t *GB_RESTRICT W = NULL ;
        bool parallel_sort = (cjnz_max > GB_BASECASE && nthreads > 1) ;
        if (parallel_sort)
        {
            // allocate workspace for parallel mergesort
            GB_MALLOC_MEMORY (W, cjnz_max, sizeof (int64_t)) ;
            if (W == NULL)
            { 
                // out of memory
                return (GrB_OUT_OF_MEMORY) ;
            }
        }

        for (taskid = 0 ; taskid < nfine ; taskid++)
        {
            int64_t hash_size = TaskList [taskid].hsize ;
            bool use_Gustavson = (hash_size == cvlen) ;
            if (!use_Gustavson && taskid == TaskList [taskid].master)
            {

                //--------------------------------------------------------------
                // phase6: fine hash task, C=A*B, C<M>=A*B, C<!M>=A*B
                //--------------------------------------------------------------

                // (Hf [hash] & 3) == 2 if C(i,j) is an entry in C(:,j),
                // and the index i of the entry is (Hf [hash] >> 2) - 1.

                int64_t kk = TaskList [taskid].vector ;
                int64_t hash_bits = (hash_size-1) ;
                int64_t  *GB_RESTRICT Hf = TaskList [taskid].Hf ;
                int64_t cjnz = Cp [kk+1] - Cp [kk] ;

                // sort the pattern of C(:,j)
                int nth = GB_nthreads (cjnz, chunk, nthreads) ;
                if (parallel_sort && nth > 1)
                { 
                    // parallel mergesort
                    GB_msort_1 (Ci + Cp [kk], W, cjnz, nth) ;
                }
                else
                { 
                    // sequential quicksort
                    GB_qsort_1a (Ci + Cp [kk], cjnz) ;
                }

                #if !GB_IS_ANY_PAIR_SEMIRING

                    GB_CTYPE *GB_RESTRICT Hx =
                        (GB_CTYPE *) TaskList [taskid].Hx ;
                    // gather the values of C(:,j)
                    int64_t pC ;
                    #pragma omp parallel for num_threads(nth) schedule(static)
                    for (pC = Cp [kk] ; pC < Cp [kk+1] ; pC++)
                    {
                        int64_t i = Ci [pC] ;   // get C(i,j)
                        int64_t i1 = i + 1 ;
                        for (GB_HASH (i))       // find i in hash table
                        {
                            int64_t hf = Hf [hash] ;
                            if ((hf & 3) == 2 && (hf >> 2) == i1)
                            { 
                                // found i in the hash table
                                GB_CIJ_GATHER (pC, hash) ; // Cx[pC] = Hx[hash]
                                break ;
                            }
                        }
                    }

                #endif
            }
        }

        // free workspace
        GB_FREE_MEMORY (W, cjnz_max, sizeof (int64_t)) ;
    }
}

#undef Cx
#undef Hx


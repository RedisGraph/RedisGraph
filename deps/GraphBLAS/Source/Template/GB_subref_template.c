//------------------------------------------------------------------------------
// GB_subref_template: C = A(I,J), C = (A(J,I))', or C = pattern (A(I,J))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This template creates two functions:

// GB_subref_numeric: numeric extraction

//      Sparse submatrix reference, C = A(I,J), extracting the values.  This is
//      an internal function called by GB_extract that does the work of the
//      user-callable GrB_*_extract methods.  It is also called by GB_assign to
//      extract the submask.  No pending tuples or zombies appear in A.

// GB_subref_symbolic: symbolic extraction

//      Sparse submatrix reference, C = A(I,J), extracting the pattern, not the
//      values.  This function is called only by GB_subassign_kernel.  Symbolic
//      extraction creates a matrix C with the same pattern (C->p and C->i) as
//      numeric extraction, but with different values, C->x.  For numeric
//      extracion if C(inew,jnew) = A(i,j), the value of A(i,j) is copied into
//      C(i,j).  For symbolic extraction, its *pointer* is copied into C(i,j).
//      Suppose an entry A(i,j) is held in Ai [pa] and Ax [pa], and it appears
//      in the output matrix C in Ci [pc] and Cx [pc].  Then the two methods
//      differ as follows:

//          // this is the same:
//          i = Ai [pa] ;           // index i of entry A(i,j)
//          aij = Ax [pa] ;         // value of the entry A(i,j)
//          Ci [pc] = inew ;        // index inew of C(inew,jnew)

//          // this is different:
//          Cx [pc] = aij ;         // for numeric extraction
//          Cx [pc] = pa ;          // for symbolic extraction

//      GB_subref_symolic is created if GB_SYMBOLIC is defined.  The function
//      is used by GB_subassign_kernel, which uses it to extract the pattern of
//      C(I,J), for the submatrix assignment C(I,J)=A.  GB_subref_symbolic
//      needs to deal with zombie entries.  The GB_subassign_kernel caller uses
//      this function on its C matrix, which is called A here because it is not
//      modified here.

//      Reading a zombie entry:  A zombie entry A(i,j) has been marked by
//      flipping its index.  The value of a zombie is not important, just its
//      presence in the pattern.  All zombies have been flipped (i < 0), and
//      all regular entries are not flipped (i >= 0).  Zombies are entries that
//      have been marked for deletion but have not been removed from the matrix
//      yet, since it's more efficient to delete zombies all at once rather
//      than one at a time.

//      GB_subref_pattern may encounter zombies in A.  It is zombie-agnostic,
//      doing nothing to them and treating them like regular entries.  Their
//      normal index must be used, not their flipped indices.  The output
//      matrix C contains all unflipped indices, and its references to zombies
//      and regular entries are identical.  Zombies in A are dealt with later.
//      They cannot be detected in the output C matrix, but they can be
//      detected in A.  Since pa = Cx [pc] holds the position of the entry in
//      A, the entry is a zombie if Ai [pa] has been flipped.

// Neither function is user-callable.

// The output matrix is passed as a handle, and created by this function, just
// like GrB_Matrix_new or GrB_Matrix_dup.

// This function is agnostic as to the CSR/CSC format, except for C_is_csc
// which is the requested format of the output matrix C (either CSR or CSC).
// It is assigned to C->is_csc but otherwise has no effect on this function.

#ifdef GB_SYMBOLIC
GrB_Info GB_subref_symbolic     // C = A (I,J), extract the pattern
#else
GrB_Info GB_subref_numeric      // C = A (I,J), extract the values
#endif
(
    GrB_Matrix *Chandle,        // output C
    const bool C_is_csc,        // requested format of C
    const GrB_Matrix A,         // input matrix
    const GrB_Index *I,         // list of indices, duplicates OK
    int64_t ni,                 // length of the I array, or special
    const GrB_Index *J,         // list of vector indices, duplicates OK
    int64_t nj,                 // length of the J array, or special
    #ifndef GB_SYMBOLIC
    const bool must_sort,       // if true, must return C sorted
    #endif
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // dealing with zombies
    //--------------------------------------------------------------------------

    #ifdef GB_SYMBOLIC
    // Ignore any zombies that may exist by unflipping all indices in A to
    // their normal non-negative values.
    #define GB_Ai(p) GB_UNFLIP (Ai [p])
    #else
    // there are no zombies
    #define GB_Ai(p) Ai [p]
    #endif

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A for C=A(I,J) subref template", GB0)) ;

    #ifdef GB_SYMBOLIC
        // GB_subref_symbolic can tolerate a matrix C with zombies and pending
        // tuples.
    #else
        // GB_subref_numeric extracts the values, so all pending tuples must be
        // assembled and there can be no zombies.
        ASSERT (!GB_PENDING (A)) ;
        ASSERT (!GB_ZOMBIES (A)) ;
    #endif

    ASSERT (I != NULL) ;
    ASSERT (J != NULL) ;

    (*Chandle) = NULL ;

    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // check the properties of I and J
    //--------------------------------------------------------------------------

    // C = A(I,J) so I is in range 0:avlen-1 and J is in range 0:avdim-1
    int64_t nI, nJ, Icolon [3], Jcolon [3] ;
    int Ikind, Jkind ;
    GB_ijlength (I, ni, avlen, &nI, &Ikind, Icolon) ;
    GB_ijlength (J, nj, avdim, &nJ, &Jkind, Jcolon) ;

    bool I_unsorted, I_contig, J_unsorted, J_contig ;
    int64_t imin, imax, jmin, jmax ;
    info = GB_ijproperties (I, ni, nI, avlen, Ikind, Icolon,
        &I_unsorted, &I_contig, &imin, &imax, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // I invalid
        return (info) ;
    }

    info = GB_ijproperties (J, nj, nJ, avdim, Jkind, Jcolon,
        &J_unsorted, &J_contig, &jmin, &jmax, false, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // J invalid
        return (info) ;
    }

    bool need_qsort = I_unsorted ;

    #ifndef GB_SYMBOLIC
    // GB_subref_symbolic must always return C sorted.  GB_subref_numeric may
    // return C with jumbled indices in each vector, if C will be transposed
    // later by GB_accum_mask.
    if (must_sort == false)
    { 
        // The caller does not need C to be returned with sorted vectors.
        need_qsort = false ;
    }
    #endif

    //--------------------------------------------------------------------------
    // I inverse buckets may be created when I is a large and not contiguous
    //--------------------------------------------------------------------------

    int64_t *Mark = NULL ;
    int64_t *Inext = NULL ;
    int64_t *Iwork1 = NULL ;
    int64_t nduplicates = 0 ;
    int64_t flag = 0 ;

    #ifdef GB_SYMBOLIC
    // the symbolic case does not need Iwork1 since it sorts Cx in place
    bool need_Iwork1 = false ;
    #else
    // the numeric case needs Iwork1 of size nI, but only if it needs to sort
    bool need_Iwork1 = need_qsort ;
    #endif

    // I = ":" or I = imin:imax are always contiguous
    ASSERT (GB_IMPLIES (Ikind == GB_ALL || Ikind == GB_RANGE, I_contig)) ;

    // FUTURE: give user control over I_inverse decision (workspace & time)
    bool I_inverse = (!I_contig && Ikind == GB_LIST && nI > avlen / 256) ;

    // the I inverse buckets are not yet created
    bool I_inverted = false ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t *restrict Ah = A->h ;
    int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const GB_void *restrict Ax = A->x ;
    const int64_t asize = A->type->size ;
    int64_t anvec = A->nvec ;

    // if A is hypersparse but all vectors are present, then
    // treat A as if it were non-hypersparse
    bool A_is_hyper = A->is_hyper && (anvec < A->vdim) ;

    //--------------------------------------------------------------------------
    // trim the hyperlist of A
    //--------------------------------------------------------------------------

    // Find kleft and kright so that Ah [kleft:kright] contains jmin:jmax.
    // Then create a trimmed view of the hyperlist by adjusting the Ah and Ap
    // pointers, and decreasing anvec.  A->h and A->p are not modified, just
    // this view of Ah and Ap.

    // if the list J is empty, the new Ah will be empty
    ASSERT ((nJ == 0) == (jmin > jmax)) ;

    // if the list is ":" then nothing would be trimmed, so skip GB_bracket
    ASSERT (GB_IMPLIES (Jkind == GB_ALL, jmin == 0 && jmax == avdim-1)) ;

    if (A_is_hyper && Jkind != GB_ALL)
    { 
        int64_t kleft, kright ;
        GB_bracket (jmin, jmax, Ah, 0, anvec-1, &kleft, &kright) ;
        Ah += kleft ;
        Ap += kleft ;
        anvec = kright - kleft + 1 ;
        ASSERT (GB_IMPLIES (nJ == 0, anvec == 0)) ;
    }

    //--------------------------------------------------------------------------
    // allocate initial space for C; this will increase in size as needed
    //--------------------------------------------------------------------------

    // Estimating nnz(C) is difficult.  If the index list I has lots of
    // duplicates, then a very sparse A can result in a dense C=A(I,J).  Or A
    // can be very dense and I very large, yet A(I,J) can be completely empty.
    // Estimating nnz(C) = nnz(A) could be severe overkill, particularly for
    // very large user-defined types.  This space is doubled if it is
    // exhausted, so start small and let it increase in size as needed.

    #ifdef GB_SYMBOLIC
    GrB_Type C_type = GrB_INT64 ;       // extract the pattern
    #else
    GrB_Type C_type = A->type ;         // extract the values
    #endif

    // determine C->plen: a strict upper bound or an estimate
    int64_t cplen = nJ ;
    if (A_is_hyper)
    {
        if (Jkind == GB_ALL)
        { 
            // cplen is a strict upper bound on the eventual C->nvec.  C->nvec
            // could be less than A->nvec, since there could be vectors that
            // appear in the hypersparse A but are actually empty, and since
            // C(I,j) can be empty where A(:,j) is not.
            cplen = anvec ;
        }
        else
        { 
            // Estimate the # of non-empty vectors of C, from A.  If this
            // estimate is too low, GB_jappend will safely grow C->p and C->h
            // as needed.
            double r = ((double) anvec) / ((double) GB_IMAX (avdim,1)) ;
            cplen = r * nJ ;
            cplen = GB_IMAX (cplen, 16) ;
            cplen = GB_IMIN (cplen, nJ) ;
        }

        // cplen is not a strict upper bound on the final C->plen, so
        // GB_jappend (C, ...) may need to reallocate C->p and C->h.  This
        // means it could fail if it runs out of memory.
    }

    // get the first index in the J = jbegin:jinc:jend list
    int64_t jbegin = GB_ijlist (J, 0, Jkind, Jcolon) ;

    // cnz_bound = nI*nJ, the max possible number of entries in C.  This may
    // result in integer overflow, in which case cnz_bounded will be false.
    GrB_Index cnz_bound ;
    bool cnz_bounded = GB_Index_multiply (&cnz_bound, nI, nJ) ;

    // if A is hypersparse, nI and nJ can far exceed nnz(C), by orders of
    // magnitude.  So it is safest to allow C to start small and grow as needed.
    int64_t cnz_init = 256 ;

    if (cnz_bounded)
    { 
        cnz_init = GB_IMIN (cnz_init, cnz_bound) ;
    }

    // C has the same hypersparsity as A.  Its CSR/CSC format
    // is determined by the caller, but is otherwise unused here.
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, C_type, nI, nJ, GB_Ap_malloc, C_is_csc,
        GB_SAME_HYPER_AS (A_is_hyper), A->hyper_ratio, cplen, cnz_init, true) ;
    if (info != GrB_SUCCESS)
    {
        // out of memory
        return (info) ;
    }

    // be careful, C is not fully initialized; C->p is merely malloc'd
    ASSERT (C->magic == GB_MAGIC2) ;

    //--------------------------------------------------------------------------
    // GB_C_REALLIC: ensures C has enough space for new entries
    //--------------------------------------------------------------------------

    #define GB_SUBREF_FREE_WORK                                         \
    {                                                                   \
        GB_FREE_MEMORY (Inext,  nI,    sizeof (int64_t)) ;              \
        GB_FREE_MEMORY (Iwork1, nI,    sizeof (int64_t)) ;              \
        GB_FREE_MEMORY (Mark,   avlen, sizeof (int64_t)) ;              \
    }

    #define GB_C_REALLOC(cnz)                                           \
    {                                                                   \
        if ((cnz) >= C->nzmax)                                          \
        {                                                               \
            /* double the space, but not beyond the bound */            \
            GrB_Index new_cnzmax = 2 * (cnz) ;                          \
            if (cnz_bounded)                                            \
            {                                                           \
                new_cnzmax = GB_IMIN (new_cnzmax, cnz_bound) ;          \
            }                                                           \
            info = GB_ix_realloc (C, new_cnzmax, true, Context) ;       \
            if (info != GrB_SUCCESS)                                    \
            {                                                           \
                /* out of memory */                                     \
                GB_MATRIX_FREE (&C) ;                                   \
                GB_SUBREF_FREE_WORK ;                                   \
                return (info) ;                                         \
            }                                                           \
            Cx = C->x ;                                                 \
            Ci = C->i ;                                                 \
        }                                                               \
    }

    //--------------------------------------------------------------------------
    // start the construction of C
    //--------------------------------------------------------------------------

    #ifdef GB_SYMBOLIC
    int64_t *Cx = C->x ;        // extract the pattern
    #else
    GB_void *Cx = C->x ;        // extract the values
    #endif
    int64_t *Ci = C->i ;

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    //--------------------------------------------------------------------------
    // determine the for-loop bounds
    //--------------------------------------------------------------------------

    // Jkind is:
    //  0: GB_ALL       J = 0:nJ-1            nJ = avdim
    //  1: GB_RANGE     J = jbegin:jend       nJ = jbegin-jend+1, or zero
    //  2: GB_STRIDE    J = jbegin:jinc:jend  nJ = see GB_ijlength
    //  3: GB_LIST      J [0:nJ-1]            nJ is anything, even nJ > avdim

    // the for loop will be: for (kk = kk_first ; kk < kk_last ; kk++)
    int64_t kk_first, kk_last ;

    if (nI == 0 || nJ == 0)
    { 

        //----------------------------------------------------------------------
        // C is empty; do nothing
        //----------------------------------------------------------------------

        kk_first = 0 ;
        kk_last  = -1 ;
        
    }
    else if (Jkind == GB_ALL)
    {

        //----------------------------------------------------------------------
        // GB_ALL:  iterate over all vectors of A, each is a vector of C
        //----------------------------------------------------------------------

        //  hyper:      for (k = 0 ; k < anvec ; k++)
        //              {
        //                  j = Ah [k] ;
        //                  jnew = j
        //                  C(:,jnew) = A(I,j)
        //              }

        //  non hyper   for (j = 0 ; j < avdim ; j++)
        //              {
        //                  jnew = j
        //                  C(:,jnew) = A(I,j)
        //              }

        if (A_is_hyper)
        { 
            kk_first = 0 ;
            kk_last  = anvec ;
        }
        else
        { 
            kk_first = 0 ;
            kk_last  = avdim ;
        }

    }
    else if (Jkind == GB_RANGE)
    {

        //----------------------------------------------------------------------
        // GB_RANGE: j = begin:end, iterate over a subset of vectors of A
        //----------------------------------------------------------------------

        // hyper:       Ah has been trimmed so this is fast

        //              for (k = 0 ; k < anvec ; k++)
        //              {
        //                  j = Ah [k] ;
        //                  jnew = j - jbegin
        //                  if (jnew < 0) continue ;
        //                  if (jnew >= nJ) break ;
        //                  C(:,jnew) = A(I,j)
        //              }

        // non hyper:   for (jnew = 0 ; j < nJ ; j++)
        //              {
        //                  j = jbegin + jnew
        //                  C(:,jnew) = A(I,j)
        //              }

        if (A_is_hyper)
        { 
            kk_first = 0 ;
            kk_last  = anvec ;
        }
        else
        { 
            kk_first = 0 ;
            kk_last =  nJ ;
        }

    }
    else // Jkind == GB_LIST or Jkind == GB_STRIDE
    { 

        //----------------------------------------------------------------------
        // GB_STRIDE: j = jbegin:jinc:jend, iterate over subset of vectors of A
        //----------------------------------------------------------------------

        // jinc can be negative, since GB_STRIDE is used for both GxB_STRIDE
        // and GxB_BACKWARDS.

        // hyper:       Ah has been trimmed so that it has just vectors in
        //              the range jmin to jmax.

        //              for (jnew = 0 ; jnew < nJ ; jnew++)
        //              {
        //                  j = jbegin + jnew * jinc ;
        //                  binary search for k so that Ah [k] = j
        //                  if found, C(:,jnew) = A(I,j)
        //              }

        // non hyper:   for (jnew = 0 ; jnew < nJ ; jnew++)
        //              {
        //                  j = jbegin + jnew * jinc ;
        //                  C(:,jnew) = A(I,j)
        //              }

        //----------------------------------------------------------------------
        // GB_LIST: iterate over the list J, of size nJ
        //----------------------------------------------------------------------

        // Iterate over each entry j in J and search for the vector in A.  J
        // has length nJ.  The hypersparse case requires a binary search but
        // there is little more that can be done to optimize this method.

        // hyper:       for (jnew = 0 ; jnew < nJ ; jnew++)
        //              {
        //                  j = J [jnew]
        //                  binary search for k so that Ah [k] = j
        //                  if found, C(:,jnew) = A(I,j)
        //              }

        // non hyper:   for (jnew = 0 ; jnew < nJ ; jnew++)
        //              {
        //                  j = J [jnew]
        //                  C(:,jnew) = A(I,j)
        //              }

        kk_first = 0 ;
        kk_last = nJ ;

    }

    //--------------------------------------------------------------------------
    // iterate over the for-loop bounds
    //--------------------------------------------------------------------------

    for (int64_t kk = kk_first ; kk < kk_last ; kk++)
    {

        //----------------------------------------------------------------------
        // find j, jnew, pA_start, pA_end for this iteration
        //----------------------------------------------------------------------

        ASSERT (nJ > 0) ;
        int64_t j, jnew, pA_start, pA_end ;

        if (Jkind == GB_ALL)
        {

            //------------------------------------------------------------------
            // GB_ALL: J = 0:anvec-1
            //------------------------------------------------------------------

            if (A_is_hyper)
            { 
                int64_t k = kk ;
                j = Ah [k] ;
                jnew = j ;
                pA_start = Ap [k] ;
                pA_end = Ap [k+1] ;
            }
            else
            { 
                int64_t k = kk ;
                j = kk ;
                jnew = j ;
                pA_start = Ap [k] ;
                pA_end = Ap [k+1] ;
            }

        }
        else if (Jkind == GB_RANGE)
        {

            //------------------------------------------------------------------
            // GB_RANGE: J = begin:end
            //------------------------------------------------------------------

            if (A_is_hyper)
            { 
                int64_t k = kk ;
                j = Ah [k] ;
                jnew = j - jbegin ;
                if (jnew <  0 ) continue ;
                if (jnew >= nJ) break ;
                pA_start = Ap [k] ;
                pA_end = Ap [k+1] ;
            }
            else
            { 
                jnew = kk ;
                j = jnew + jbegin ;
                pA_start = Ap [j] ;
                pA_end = Ap [j+1] ;
            }

        }
        else // Jkind == GB_LIST or GB_STRIDE
        {

            //------------------------------------------------------------------
            // GB_LIST or GB_STRIDE: J = a list, or jbegin:jinc:jend
            //------------------------------------------------------------------

            if (A_is_hyper)
            { 
                // if j is not found, pA_start:pA_end is empty
                jnew = kk ;
                // j = J [jnew] ; or get j from a colon expression
                j = GB_ijlist (J, jnew, Jkind, Jcolon) ;
                int64_t pleft = 0, pright = anvec-1 ;
                GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, j,
                    &pA_start, &pA_end) ;
            }
            else
            { 
                jnew = kk ;
                // j = J [jnew] ; or get j from a colon expression
                j = GB_ijlist (J, jnew, Jkind, Jcolon) ;
                pA_start = Ap [j] ;
                pA_end = Ap [j+1] ;
            }
        }

        //----------------------------------------------------------------------
        // construct vector C (:,jnew) = A (I,j)
        //----------------------------------------------------------------------

        // j == J [jnew] or from a colon expression
        ASSERT (j == GB_ijlist (J, jnew, Jkind, Jcolon)) ;

        ASSERT (pA_start >= -1 && pA_start <= pA_end && pA_end <= GB_NNZ (A)) ;

        int64_t ajnz = pA_end - pA_start ;
        int64_t ajnz2 = 0 ;
        int64_t pstart = pA_start ;
        int64_t pend = pA_end - 1 ;

        #ifndef NDEBUG
        // used just for assertions
        int64_t pstart_orig = pstart ;
        int64_t pend_orig   = pend ;
        #endif

        bool aj_dense = (ajnz == avlen) ;   // true if A(:,j) completely dense

        ASSERT (ajnz >= 0 && ajnz <= avlen) ;

        // skip if no entries in A (:,j)
        if (ajnz == 0) continue ;

        // skip if all indices in A (:,j) are outside [imin..imax]
        if (GB_Ai (pstart) > imax || GB_Ai (pend) < imin) continue ;

        //----------------------------------------------------------------------
        // binary search in A(:,j) for first index i >= imin
        //----------------------------------------------------------------------

        // This phase does nothing if I is ":" since imin = 0 in that case.
        // It assists in other cases by trimming the search space of A(:,j), by
        // advancing pstart.  When done, the entries in I that could be in
        // A(:,j) are in the list Ai [pstart...pend].

        // Time taken for this step is at most O(log(ajnz)), in general, but
        // only O(1) time if I is ":" (because in that case imin = 0).

        // if A(:,j) is dense, skip the binary search; cases 10 or 11 will be
        // used.  They are like Case 6, but no binary search.  Like Case 6, no
        // sort needed even if I is unsorted.

        if (!aj_dense && (GB_Ai (pstart) < imin))
        { 
            ASSERT (pstart_orig <= pstart && pstart <= pend) ;
            ASSERT (GB_Ai (pstart) <= imin) ;

            int64_t pleft = pstart ;
            int64_t pright = pend ;
            #ifdef GB_SYMBOLIC
            GB_BINARY_TRIM_ZOMBIE (imin, Ai, pleft, pright) ;
            #else
            GB_BINARY_TRIM_SEARCH (imin, Ai, pleft, pright) ;
            #endif

            pstart = pleft ;
            ASSERT (imin <= GB_Ai (pstart)) ;
        }

        // If pstart has been advanced, the entry Ai [pstart-1] before it
        // must be less than imin, and can thus be ignored in all
        // subsequent searches

        ASSERT (pstart_orig <= pstart) ;
        ASSERT (pstart <= pend) ;
        ASSERT (pend == pend_orig) ;
        ASSERT (GB_IMPLIES (pstart_orig < pstart, GB_Ai (pstart-1) < imin)) ;

        // ajnz2 is the # of entries in A(:,j) that still need searching.
        ajnz2 = pend - pstart + 1 ;

        //----------------------------------------------------------------------
        // ready to do the work
        //----------------------------------------------------------------------

        // Summary of the different cases, applied to each vector j.  Let
        // ajnz = nnz (A (:,j)), and let cjnz == nnz (C (:,jnew)), the number
        // of entries in the new vector of C.  Let nI == length (I).  Each case
        // is preceded by a binary search of A (:,j) for the index imin = min
        // (I).  Let ajnz2 = nnz (A (imin:end,j)) and let ajnz3 = nnz (A
        // (imin:imax,j)).  The binary search takes log (ajnz) time, unless all
        // entries in A (:,j) are less than imin.  In that case, the binary
        // search is skipped.  If I = ":" then imin = 0 and the first binary
        // search is skipped.

        #if 0
        if (aj_dense)
        {
            // A(:,j) is dense
            if I is ":"
                // Case 1: C(:,j) = A(:,j)
            else
                // Case 2: C(I,j) = A(I,j)
            // time: O(nI) which is optimal since nI == nnz (C (I,jnew))
        }
        else if (nI == 1)
        {
            // Case 3: one index
            // C (:,jnew) = A (i,j)
            // time: O(log(ajnz)), optimal given the data structure
        }
        else if (I == ":")
        {
            // Case 4: I is ":"
            // C (:,jnew) = A (:,j)
            // time: O(ajnz), optimal since cjnz == ajnz
        }
        else if (I_contig)
        {
            // Case 5: I_contig I = ibegin:iend
            // C (:,jnew) = A (ibegin:iend,j)
            // time: O(log(ajnz) + cjnz), optimal given the data structure
        }
        else if (
            (Ikind == GB_LIST && !I_inverse) || // must do Case 6
            (need_qsort  && nI < ajnz2) ||      // Case 6 faster than 9
            (!need_qsort && 32 * nI < ajnz2))   // Case 6 faster than 10, 11
        {
            // Case 6: nI not large
            // C (:,jnew) = A (I,j)
            // time: O(nI*log(ajnz3))
        }
        else if (Ikind == GB_STRIDE && Icolon [GxB_INC] >= 0)
        {
            // Case 7: I = ibegin:iinc:iend with iinc >= 0
            // iinc will not be = 0 since it implies nI==0 and thus this
            // whole phase is skipped.
            // time: O(ajnz3)
        }
        else if (Ikind == GB_STRIDE && Icolon [GxB_INC] < 0)
        {
            // Case 8: I = ibegin:iinc:iend with iinc < 0
            // time: O(ajnz3)
        }
        else // I inverse buckets will be used
        {
            // construct the I inverse buckets
            if (need_qsort)
            {
                // Case 9: nI large, need qsort
                // C (:,jnew) = A (I,j)
                // time: O(cjnz*log(cjnz) + ajnz3)
            }
            else if (nduplicates > 0)
            {
                // Case 10: nI large, no qsort, with duplicates
                // C (:,jnew) = A (I,j)
                // time: O(cjnz + ajnz3)
            }
            else
            {
                // Case 11: nI large, no qsort, no dupl
                // C (:,jnew) = A (I,j)
                // time: O(ajnz3)
            }
        }
        #endif

        //----------------------------------------------------------------------
        // extract C (:,jnew) = A (I,j): consider all cases
        //----------------------------------------------------------------------

        if (aj_dense)
        {

            //------------------------------------------------------------------
            // Case 1 and Case 2: A(:,j) is dense
            //------------------------------------------------------------------

            // A(:,j) is entirely dense, so C(I,jnew) = A (I,j) will have
            // exactly nI = |I| entries.  The vector of C is constructed in
            // sorted order, so no sort is needed.  No binary search is needed.
            // Time is O(nI), which is optimal since nI == C(:,jnew)

            GB_C_REALLOC (cnz + nI) ;

            if (Ikind == GB_ALL)
            {

                //--------------------------------------------------------------
                // Case 1:  C(:,jnew) = A(:,j) where A(:,j) is dense
                //--------------------------------------------------------------

                ASSERT (nI == avlen) ;

                // construct the pattern of C(:,jnew)
                for (int64_t i = 0 ; i < avlen ; i++)
                {
                    Ci [cnz + i] = i ;
                }

                #ifdef GB_SYMBOLIC
                { 
                    // construct the pointers
                    for (int64_t i = 0, p = pstart ; i < avlen ; i++)
                    {
                        Cx [cnz++] = p++ ;
                    }
                }
                #else
                { 
                    // copy the values
                    memcpy (Cx +(cnz*asize), Ax +(pstart*asize), asize*avlen) ;
                    cnz += avlen ;
                }
                #endif

            }
            else
            {

                //--------------------------------------------------------------
                // Case 2:  C(I,jnew) = A(I,j) where A(I,j) is dense
                //--------------------------------------------------------------

                // scan I and get the entry in A(:,j) via direct lookup
                for (int64_t inew = 0 ; inew < nI ; inew++)
                {
                    // A (i,j) will become C (inew,jnew), and it always exists.
                    // i = I [inew] ; or from a colon expression
                    int64_t i = GB_ijlist (I, inew, Ikind, Icolon) ;
                    int64_t p = pstart + i ;
                    ASSERT (i == GB_Ai (p)) ;

                    Ci [cnz] = inew ;
                    #ifdef GB_SYMBOLIC
                    { 
                        // extract the pattern, not the value
                        Cx [cnz] = p ;
                    }
                    #else
                    { 
                        // Cx [cnz] = Ax [p] ;
                        memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                    }
                    #endif
                    cnz++ ;
                }
                ASSERT (cnz <= C->nzmax) ;
            }

        }
        else if (nI == 1)
        {

            //------------------------------------------------------------------
            // Case 3: the list I has a single index, ibegin
            //------------------------------------------------------------------

            // binary search has already found imin. Time is O(1) for this
            // step, or O(log(ajnz)) total for this vector.

            GB_C_REALLOC (cnz) ;

            ASSERT (imin == GB_ijlist (I, 0, Ikind, Icolon)) ;

            if (GB_Ai (pstart) == imin)
            {
                Ci [cnz] = 0 ;
                #ifdef GB_SYMBOLIC
                { 
                    // extract the position, not the value
                    Cx [cnz] = pstart ;
                }
                #else
                { 
                    // Cx [cnz] = Ax [pstart] ;
                    memcpy (Cx +(cnz*asize), Ax +(pstart*asize), asize) ;
                }
                #endif
                cnz++ ;
                ASSERT (cnz <= C->nzmax) ;
            }

        }
        else if (Ikind == GB_ALL)
        {

            //------------------------------------------------------------------
            // Case 4: I = 0:avlen-1, thus C(:,jnew) = A (:,j)
            //------------------------------------------------------------------

            // Total time is O(ajnz), but that entire time is required since
            // it is also the same as nnz (C (:,jnew)).

            GB_C_REALLOC (cnz + ajnz) ;

            ASSERT (cnz + ajnz <= C->nzmax) ;
            #ifdef GB_SYMBOLIC
            {
                // extract the positions, not the values
                // Ci [cnz:cnz+ajnz-1] = GB_UNFLIP (Ai [pstart:pstart+ajnz-1]) ;
                // Cx [cnz:cnz+ajnz-1] = [pstart:pstart+ajnz-1] ;
                for (int64_t k = 0 ; k < ajnz ; k++)
                { 
                    Ci [cnz + k] = GB_Ai (pstart + k) ;
                    Cx [cnz + k] = pstart + k ;
                }
            }
            #else
            { 
                // Ci [cnz:cnz+ajnz-1] = Ai [pstart:pstart+ajnz-1] ;
                memcpy (&Ci [cnz], &Ai [pstart], sizeof (int64_t)*ajnz) ;
                // Cx [cnz:cnz+ajnz-1] = Ax [pstart:pstart+ajnz-1] ;
                memcpy (Cx +(cnz*asize), Ax +(pstart*asize), asize*ajnz) ;
            }
            #endif
            cnz += ajnz ;
            ASSERT (cnz <= C->nzmax) ;

        }
        else if (I_contig)
        {

            //------------------------------------------------------------------
            // Case 5: I is a contiguous list, imin:imax
            //------------------------------------------------------------------

            // Total time is O(C(:,jnew) + log(ajnz)), which is nearly optimal

            int64_t cnz1 = cnz ;
            for (int64_t p = pstart ; p <= pend ; p++)
            {
                // A(i,j) is nonzero; no need to look it up in the I buckets
                int64_t i = GB_Ai (p) ;
                if (i > imax)
                { 
                    // no more entries in A(:,j) in range of imin:imax
                    break ;
                }
                ASSERT (i-imin >= 0 && i-imin < nI) ;
                ASSERT (i == GB_ijlist (I, i-imin, Ikind, Icolon)) ;
                ASSERT (GB_IMPLIES (Ikind == GB_LIST, i == I [i-imin])) ;
                GB_C_REALLOC (cnz) ;
                Ci [cnz++] = i-imin ;
                ASSERT (cnz <= C->nzmax) ;
            }
            #ifdef GB_SYMBOLIC
            {
                // extract the positions, not the values
                // Cx [cnz1:cnz-1] = [pstart:pstart+(cnz-cnz1-1)] ;
                for (int64_t k = 0 ; k < (cnz - cnz1) ; k++)
                { 
                    Cx [cnz1 + k] = pstart + k ;
                }
            }
            #else
            { 
                // Cx [cnz1:cnz-1] = Ax [pstart:pstart+(cnz-cnz1-1)] ;
                memcpy (Cx +(cnz1*asize), Ax +(pstart*asize), asize*(cnz-cnz1));
            }
            #endif

        }
        else if (
            (Ikind == GB_LIST && !I_inverse) || // must do Case 6
            (need_qsort  && nI < ajnz2) ||      // Case 6 faster than 9
            (!need_qsort && 32 * nI < ajnz2))   // Case 6 faster than 10, 11
        {

            //------------------------------------------------------------------
            // Case 6: I is short compared with nnz (A (:,j)), use binary search
            //------------------------------------------------------------------

            // If I_inverse is false then the I bucket inverse has not been
            // created, so this method is the only option.  Alternatively, if
            // nI = length (I) is small relative to nnz (A (:,j)), then
            // scanning I and doing a binary search of A (:,j) is faster than
            // doing a linear-time search of A(:,j) and a lookup into the I
            // bucket inverse.

            // Time taken is O(nI*log(ajnz3)) for this step

            // Ai [pstart..pend] contains all entries in the range imin:imax,
            // but pend is still equal to is original value (pend_orig).  Only
            // pstart has been advanced.  Do another binary search for imax, to
            // prune the search space at the bottom of A(:,j).

            // The vector of C is constructed in sorted order, so no sort is
            // needed.

            ASSERT (Ikind == GB_LIST || Ikind == GB_STRIDE) ;

            int64_t pleft = pstart ;
            int64_t pright = pend ;
            #ifdef GB_SYMBOLIC
            GB_BINARY_TRIM_ZOMBIE (imax, Ai, pleft, pright) ;
            #else
            GB_BINARY_TRIM_SEARCH (imax, Ai, pleft, pright) ;
            #endif

            // x = Ai [p] where p == pleft == pright.  The item searched for
            // is imax, and either x=imax and it is found, or imax is not found
            // and x is the first entry in the list greater than imax.
            pend = pright ;
            ASSERT (pstart_orig <= pstart) ;
            ASSERT (pstart <= pend) ;
            ASSERT (pend <= pend_orig) ;
            ASSERT (GB_IMPLIES (pstart_orig < pstart, GB_Ai (pstart-1) < imin));
            ASSERT (GB_IMPLIES (pend < pend_orig, imax < GB_Ai (pend+1))) ;

            // scan I, in order, and search for the entry in A(:,j)
            for (int64_t inew = 0 ; inew < nI ; inew++)
            {
                // A (i,j) will become C (inew,jnew), if it exists
                // i = I [inew] ; or from a colon expression
                int64_t i = GB_ijlist (I, inew, Ikind, Icolon) ;

                bool found, is_zombie ;
                int64_t pleft = pstart ;
                int64_t pright = pend ;
                #ifdef GB_SYMBOLIC
                GB_BINARY_ZOMBIE (i, Ai, pleft, pright, found,
                    A->nzombies, is_zombie) ;
                #else
                GB_BINARY_SEARCH (i, Ai, pleft, pright, found) ;
                #endif

                if (found)
                {
                    GB_C_REALLOC (cnz) ;
                    Ci [cnz] = inew ;
                    #ifdef GB_SYMBOLIC
                    { 
                        // extract the pattern, not the value
                        Cx [cnz] = pleft ;
                    }
                    #else
                    { 
                        // Cx [cnz] = Ax [pleft] ;
                        memcpy (Cx +(cnz*asize), Ax +(pleft*asize), asize) ;
                    }
                    #endif
                    cnz++ ;
                    ASSERT (cnz <= C->nzmax) ;
                }
            }

        }
        else if (Ikind == GB_STRIDE && Icolon [GxB_INC] >= 0)
        {

            //------------------------------------------------------------------
            // Case 7: I is ibegin:iinc:iend with iinc >= 0
            //------------------------------------------------------------------

            int64_t ibegin = Icolon [GxB_BEGIN] ;
            int64_t iinc   = Icolon [GxB_INC] ;
            for (int64_t p = pstart ; p <= pend ; p++)
            {
                // A(i,j) is nonzero; see if it is in ibegin:iinc:iend
                int64_t i = GB_Ai (p) ;
                if (i > imax)
                { 
                    // no more entries in A(:,j) in range of imin:imax
                    break ;
                }
                if (!GB_ij_is_in_list (I, nI, i, GB_STRIDE, Icolon))
                { 
                    // i is not in the sequence ibegin:iinc:iend
                    continue ;
                }
                int64_t inew = (i - ibegin) / iinc ;
                ASSERT (i == GB_ijlist (I, inew, GB_STRIDE, Icolon)) ;
                GB_C_REALLOC (cnz) ;
                Ci [cnz] = inew ;
                #ifdef GB_SYMBOLIC
                { 
                    // extract the pattern, not the value
                    Cx [cnz] = p ;
                }
                #else
                { 
                    // Cx [cnz] = Ax [p] ;
                    memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                }
                #endif
                cnz++ ;
                ASSERT (cnz <= C->nzmax) ;
            }

        }
        else if (Ikind == GB_STRIDE && Icolon [GxB_INC] < 0)
        {

            //------------------------------------------------------------------
            // Case 8: I is ibegin:iinc:iend with iinc < 0
            //------------------------------------------------------------------

            // trim the end of A(:,j).  see Case 6.
            int64_t pleft = pstart ;
            int64_t pright = pend ;
            #ifdef GB_SYMBOLIC
            GB_BINARY_TRIM_ZOMBIE (imax, Ai, pleft, pright) ;
            #else
            GB_BINARY_TRIM_SEARCH (imax, Ai, pleft, pright) ;
            #endif
            pend = pright ;

            // x = Ai [p] where p == pleft == pright.  The item searched for
            // is imax, and either x=imax and it is found, or imax is not found
            // and x is the first entry in the list greater than imax.

            int64_t ibegin = Icolon [GxB_BEGIN] ;
            int64_t iinc   = Icolon [GxB_INC] ;
            iinc = -iinc ;
            // iinc is now positive
            ASSERT (iinc > 0) ;

            if (iinc > 1)
            {

                //--------------------------------------------------------------
                // I = ibegin:(-inc):iend, general case
                //--------------------------------------------------------------

                for (int64_t p = pend ; p >= pstart ; p--)
                {
                    // A(i,j) is nonzero; see if it is in ibegin:iinc:iend
                    int64_t i = GB_Ai (p) ;
                    if (!GB_ij_is_in_list (I, nI, i, GB_STRIDE, Icolon))
                    { 
                        // i is not in the sequence ibegin:iinc:iend
                        continue ;
                    }
                    int64_t inew = (ibegin - i) / iinc ;
                    ASSERT (i == GB_ijlist (I, inew, GB_STRIDE, Icolon)) ;
                    GB_C_REALLOC (cnz) ;
                    Ci [cnz] = inew ;
                    #ifdef GB_SYMBOLIC
                    { 
                        // extract the pattern, not the value
                        Cx [cnz] = p ;
                    }
                    #else
                    { 
                        // Cx [cnz] = Ax [p] ;
                        memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                    }
                    #endif
                    cnz++ ;
                    ASSERT (cnz <= C->nzmax) ;
                }

            }
            else
            {

                //--------------------------------------------------------------
                // I = ibegin:(-1):iend, special case
                //--------------------------------------------------------------

                for (int64_t p = pend ; p >= pstart ; p--)
                {
                    // A(i,j) is nonzero; see if it is in ibegin:iinc:iend
                    int64_t i = GB_Ai (p) ;
                    int64_t inew = (ibegin - i) ;
                    // ensure the trim is OK
                    if (inew < 0 || inew >= nI)
                    { 
                        // i is not in ibegin:iinc:iend
                        continue ;
                    }
                    ASSERT (i == GB_ijlist (I, inew, GB_STRIDE, Icolon)) ;
                    GB_C_REALLOC (cnz) ;
                    Ci [cnz] = inew ;
                    #ifdef GB_SYMBOLIC
                    { 
                        // extract the pattern, not the value
                        Cx [cnz] = p ;
                    }
                    #else
                    { 
                        // Cx [cnz] = Ax [p] ;
                        memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                    }
                    #endif
                    cnz++ ;
                    ASSERT (cnz <= C->nzmax) ;
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Case 9 to 11: using the I inverse buckets
            //------------------------------------------------------------------

            // None of the above cases use the I inverse buckets.  From here on
            // Case 9 and all the following cases below all require the I
            // inverse buckets.

            ASSERT (Ikind == GB_LIST) ;     // I is an explicit list
            ASSERT (I_inverse) ;            // I may be inverted

            if (!I_inverted)
            {
                info = GB_I_inverse (I, nI, avlen, need_Iwork1,
                    &Mark, &Inext, &Iwork1, &nduplicates, &flag, Context) ;
                if (info != GrB_SUCCESS)
                { 
                    // out of memory
                    GB_MATRIX_FREE (&C) ;
                    GB_SUBREF_FREE_WORK ;
                    return (info) ;
                }
                I_inverted = true ;
            }

            ASSERT (Mark != NULL) ;
            ASSERT (Inext != NULL) ;

            // to iterate across all entries in a bucket:
            #define GB_for_each_entry_in_bucket(inew,i) \
                for (int64_t inew = Mark[i]-flag ; inew >= 0 ; \
                    inew = Inext [inew])

            if (need_qsort)
            {

                //--------------------------------------------------------------
                // Case 9: I unsorted, and C needs qsort, duplicates OK
                //--------------------------------------------------------------

                // Case 9 works well when I has many entries and A(:,j) has few
                // entries.  Time taken is O(cjnz*log(cjnz)+ajnz3) in the worst
                // case, where cjnz <= nI.  cjnz and ajnz3 are not comparable
                // because of duplicates.

                ASSERT (Ikind == GB_LIST) ;

                // The indices Ci for C(:,j) are sorted in-place.  The pointers
                // are sorted in-place in Cx for the symbolic case, or in
                // Iwork1 for the numeric case.

                // place indices in Ci [cnz...cnz+cjnz-1] and pointers in
                // Cx (symbolic case) or Iwork1 (numeric case)
                int64_t cjnz = 0 ;
                for (int64_t p = pstart ; p <= pend ; p++)
                {
                    // A(i,j) is nonzero, look it up in the I inverse buckets
                    int64_t i = GB_Ai (p) ;
                    if (i > imax)
                    { 
                        // no more entries in A(:,j) in range of imin:imax
                        break ;
                    }
                    // traverse bucket i for all indices inew where
                    // i == I [inew] or where i is from a colon expression
                    GB_for_each_entry_in_bucket (inew, i)
                    { 
                        GB_C_REALLOC (cnz + cjnz) ;
                        ASSERT (inew >= 0 && inew < nI) ;
                        ASSERT (i == GB_ijlist (I, inew, Ikind, Icolon)) ;
                        Ci [cnz + cjnz] = inew ;
                        // record the position p in Ai, Ax [p] of the entry
                        #ifdef GB_SYMBOLIC
                        Cx [cnz + cjnz] = p ;
                        #else
                        ASSERT (Iwork1 != NULL) ;
                        Iwork1 [cjnz] = p ;
                        #endif
                        cjnz++ ;
                    }
                }

                ASSERT (cjnz <= nI) ;

                // sort the entries in C(:,j) by ascending row index
                #ifdef GB_SYMBOLIC
                { 
                    // sort the indices and pointers in-place in [Ci,Cx]
                    GB_qsort_2a (Ci + cnz, Cx + cnz, cjnz) ;
                    cnz += cjnz ;
                }
                #else
                {
                    // sort the indices in-place and the pointers in Iwork1
                    GB_qsort_2a (Ci + cnz, Iwork1, cjnz) ;
                    // copy into C (:,jnew) using Iwork1 pointers
                    for (int64_t k = 0 ; k < cjnz ; k++)
                    { 
                        int64_t p = Iwork1 [k] ;
                        // Cx [cnz] = Ax [p] ;
                        memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                        cnz++ ;
                        ASSERT (cnz <= C->nzmax) ;
                    }
                }
                #endif

            }
            else if (nduplicates > 0)
            {

                //--------------------------------------------------------------
                // Case 10: I not contiguous, with duplicates.  No qsort needed.
                //--------------------------------------------------------------

                // Case 10 works well when I has many entries and A(:,j) has few
                // entries.  Time taken is O(cjnz+ajnz3) in the worst case,
                // where cjnz = nnz (C (:,jnew)) and ajnz3 < nnz (A (:,j)).

                for (int64_t p = pstart ; p <= pend ; p++)
                {
                    // A(i,j) is nonzero, look it up in the I inverse buckets
                    int64_t i = GB_Ai (p) ;
                    if (i > imax)
                    { 
                        // no more entries in A(:,j) in range of imin:imax
                        break ;
                    }
                    // traverse bucket i for all indices inew where i == I[inew]
                    GB_for_each_entry_in_bucket (inew, i)
                    {
                        ASSERT (inew >= 0 && inew < nI) ;
                        ASSERT (i == GB_ijlist (I, inew, Ikind, Icolon)) ;
                        GB_C_REALLOC (cnz) ;
                        Ci [cnz] = inew ;
                        #ifdef GB_SYMBOLIC
                        { 
                            Cx [cnz] = p ;
                        }
                        #else
                        { 
                            // Cx [cnz] = Ax [p] ;
                            memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                        }
                        #endif
                        cnz++ ;
                        ASSERT (cnz <= C->nzmax) ;
                    }
                }

            }
            else
            {

                //--------------------------------------------------------------
                // Case 11: I not contiguous, no duplicates.  No qsort needed.
                //--------------------------------------------------------------

                // Identical to Case 10, except GB_for_each_entry_in_bucket
                // (...) just needs to iterate 0 or 1 times.  Works well when I
                // has many entries and A(:,j) has few entries.  Time taken is
                // O(ajnz3)

                for (int64_t p = pstart ; p <= pend ; p++)
                {
                    // A(i,j) is nonzero, look it up in I inverse buckets
                    int64_t i = GB_Ai (p) ;
                    if (i > imax)
                    { 
                        // no more entries in A(:,j) in range of imin:imax
                        break ;
                    }
                    // bucket i has at most one index inew such that i==I[inew]
                    int64_t inew = Mark [i] - flag ;
                    if (inew >= 0)
                    {
                        ASSERT (inew >= 0 && inew < nI) ;
                        ASSERT (i == GB_ijlist (I, inew, Ikind, Icolon)) ;
                        GB_C_REALLOC (cnz) ;
                        Ci [cnz] = inew ;
                        #ifdef GB_SYMBOLIC
                        { 
                            // extract the pattern, not the values
                            Cx [cnz] = p ;
                        }
                        #else
                        { 
                            // Cx [cnz] = Ax [p] ;
                            memcpy (Cx +(cnz*asize), Ax +(p*asize), asize) ;
                        }
                        #endif
                        cnz++ ;
                        ASSERT (cnz <= C->nzmax) ;
                        // at most one entry in bucket i
                        ASSERT (Inext [inew] < 0) ;
                    }
                }
            }
        }

        //----------------------------------------------------------------------
        // finalize the vector C(:,jnew)
        //----------------------------------------------------------------------

        // C->plen is not allocated up front at its upper bound, since no good
        // bound exists.  Thus, C->p and C->h may need to increase in size.
        info = GB_jappend (C, jnew, &jlast, cnz, &cnz_last, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (&C) ;
            GB_SUBREF_FREE_WORK ;
            return (info) ;
        }
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_SUBREF_FREE_WORK ;

    //--------------------------------------------------------------------------
    // finalize the construction of C
    //--------------------------------------------------------------------------

    GB_jwrapup (C, jlast, cnz) ;

    ASSERT_OK_OR_JUMBLED (GB_check (C, "subref C=A(I,J), almost done", GB0)) ;

    //--------------------------------------------------------------------------
    // resize the output C
    //--------------------------------------------------------------------------

    // resize C to actual number of entries
    ASSERT (cnz <= C->nzmax) ;
    if (cnz < C->nzmax)
    { 
        info = GB_ix_realloc (C, cnz, true, Context) ;
        ASSERT (info == GrB_SUCCESS) ;
    }

    // conform C to its desired hypersparsity; note that C can have jumbled
    // columns, which is OK.  The GB_to_hyper* and GB_to_nonhyper do not access
    // the row indices so they tolerate jumbled matrices.
    info = GB_to_hyper_conform (C, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_MATRIX_FREE (&C) ;
        return (info) ;
    }

    // C now has with sorted indices
    ASSERT_OK_OR_JUMBLED (GB_check (C, "C output for  C=A(I,J)", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}


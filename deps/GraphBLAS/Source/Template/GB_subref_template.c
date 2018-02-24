//------------------------------------------------------------------------------
// GB_subref_template: C = A(I,J), C = (A(J,I))', or C = pattern (A(I,J))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This template creates two functions:

// GB_subref_numeric: numeric extraction

//      Sparse submatrix reference, C = A(I,J).  A can be optionally transposed
//      on input first.  This is equivalent to swapping I and J and transposing
//      the result, which is how this method implements this option.  Internal
//      function that does the work of the user-callable GrB_*_extract methods.
//      No pending tuples or zombies appear in A.

// GB_subref_symbolic: symbolic extraction

//      Sparse submatrix reference, C = A(I,J), extracting the pattern, not the
//      values.  Symbolic extraction creates a matrix C with the same pattern
//      (C->p and C->i) as numeric extraction, but with different values, C->x.
//      For numeric extracion if C(inew,jnew) = A(i,j), the value of A(i,j) is
//      copied into C(i,j).  For symbolic extraction, its *pointer* is copied
//      into C(i,j).  Suppose an entry A(i,j) is held in Ai [pa] and Ax [pa],
//      and it appears in the output matrix C in Ci [pc] and Cx [pc].  Then the
//      two methods differ as follows:

//          // this is the same:
//          i = Ai [pa] ;           // row index i of entry A(i,j)
//          aij = Ax [pa] ;         // value of the entry A(i,j)
//          Ci [pc] = inew ;        // row index inew of C(inew,jnew)

//          // this is different:
//          Cx [pc] = aij ;         // for numeric extraction
//          Cx [pc] = pa ;          // for symbolic extraction

//      GB_subref_symolic is created if SYMBOLIC is #define'd.  The function is
//      used by GB_subassign_kernel, which uses it to extract the pattern of
//      C(I,J), for the submatrix assignment C(I,J)=A.  The matrix on the left
//      hand side is not transposed, so that option is not used.
//      GB_subref_symbolic needs to deal with zombie entries.  The
//      GB_subassign_kernel caller uses this function on its C matrix, which is
//      called A here because it is not modified here.

//      Reading a zombie entry:  A zombie entry A(i,j) has been marked by
//      flipping its row index.  The value of a zombie is not important, just
//      its presence in the pattern.  All zombies have been flipped (i < 0),
//      and all regular entries are not flipped (i >= 0).  Zombies are entries
//      that have been marked for deletion but have not been removed from the
//      matrix yet, since it's more efficient to delete zombies all at once
//      rather than one at a time.

//      GB_subref_pattern may encounter zombies in A.  It is zombie-agnostic,
//      doing nothing to them and treating them like regular entries.  Their
//      normal row index must be used, not their flipped row indices.  The
//      output matrix C contains all unflipped row indices, and its references
//      to zombies and regular entries are identical.  Zombies in A are dealt
//      with later.  They cannot be detected in the output C matrix, but they
//      can be detected in A.  Since pa = Cx [pc] holds the position of the
//      entry in A, the entry is a zombie if Ai [pa] has been flipped.

// Neither function is user-callable.

// The output matrix is passed as a handle, and created by this function, just
// like GrB_Matrix_new or GrB_Matrix_dup.

// FUTURE: consider a special value for ni and nj: GrB_RANGE.  If this value
// is given for ni, then I has length 2, and it implies I = I[0]:I[1].

#ifdef SYMBOLIC
GrB_Info GB_subref_symbolic     // C = A (I,J), extract the pattern, not values
#else
GrB_Info GB_subref_numeric      // C = A (I,J) or (A (J,I))', extract values
#endif
(
    GrB_Matrix *handle,         // output matrix, if NULL just check dimensions
    const GrB_Index cnrows,     // requested # of rows of output matrix
    const GrB_Index cncols,     // requested # of columns of output matrix
    const GrB_Matrix A,         // input matrix, optionally transposed
    const GrB_Index *I_in,      // list of row indices, duplicates OK
    const GrB_Index ni_in,      // number of row indices
    const GrB_Index *J_in,      // list of column indices, duplicates OK
    const GrB_Index nj_in       // number of column indices
    #ifdef SYMBOLIC
    #define A_transpose false
    #else
    , const bool A_transpose    // use A' instead of A
    #endif
)
{

    //--------------------------------------------------------------------------
    // dealing with zombies
    //--------------------------------------------------------------------------

    #ifdef SYMBOLIC
    // Ignore any zombies that may exist by unflipping all row indices in A to
    // their normal non-negative values.
    #define AI(p) UNFLIP (Ai [p])
    #else
    // there are no zombies
    #define AI(p) Ai [p]
    #endif

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // handle may be NULL; this dictates error checks only. See GB_extract.
    ASSERT_OK (GB_check (A, "A for C=A(I,J)", 0)) ;

    #ifdef SYMBOLIC
        // GB_subref_symbolic can tolerate a matrix C with zombies and pending
        // tuples.
    #else
        // GB_subref_numeric extracts the values, so all pending tuples must be
        // assembled and there can be no zombies.
        ASSERT (!PENDING (A)) ;
        ASSERT (!ZOMBIES (A)) ;
    #endif

    ASSERT (I_in != NULL) ;
    ASSERT (J_in != NULL) ;

    //--------------------------------------------------------------------------
    // check dimensions
    //--------------------------------------------------------------------------

    int64_t anrows = A->nrows ;
    int64_t ancols = A->ncols ;

    const GrB_Index *I, *J ;
    int64_t ni, nj ;

    if (A_transpose)
    {
        // A = A' is transposed first and then C=A(I,J) is found.  This is the
        // same as C=A(J,I)'.  So I_in and J_in are swapped to become I and J.
        I = J_in ;
        J = I_in ;
        ni = nj_in ;
        nj = ni_in ;
    }
    else
    {
        // C = A (I,J)
        I = I_in ;
        J = J_in ;
        ni = ni_in ;
        nj = nj_in ;
    }

    // C=A(I,J) is computed, or C=A(I,J)' if A_transpose is true.  In either
    // case, I contains a list of row indices of A (never A') in the range
    // 0 to anrows-1, and J contains a list of column indices of A (never A')
    // in the range 0 to ancols-1.

    if (I == GrB_ALL)
    {
        // if I is GrB_ALL, this denotes that I is ":"
        ni = anrows ;
    }
    if (J == GrB_ALL)
    {
        // if J is GrB_ALL, this denotes that J is ":"
        nj = ancols ;
    }

    // return without doing any work, if just the dimensions are being checked
    if (handle == NULL)
    {
        // check dimensions of C.  If handle is not NULL, cnrows and cncols
        // are ignored, and C is created with size ni-by-nj.
        if (cnrows != ((A_transpose) ? nj : ni) ||
            cncols != ((A_transpose) ? ni : nj))
        {
            return (ERROR (GrB_DIMENSION_MISMATCH, (LOG,
                "Dimensions not compatible:\n"
                "IxJ is "GBd"-by-"GBd"%s\n"
                "output is "GBd"-by-"GBd"\n",
                ((A_transpose) ? nj : ni), ((A_transpose) ? ni : nj),
                A_transpose ?  " (transposed)" : "", cnrows, cncols))) ;
        }
        return (REPORT_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // check the rest of the inputs
    //--------------------------------------------------------------------------

    // check I and J
    bool need_qsort, contig ;
    int64_t imin, imax ;
    GrB_Info info = GB_ijproperties (I, ni, J, nj, anrows, ancols,
        &need_qsort, &contig, &imin, &imax) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    if (A_transpose)
    {
        // C=A(I,J)' does not need qsort since final transpose will do the sort
        need_qsort = false ;
    }

    //--------------------------------------------------------------------------
    // create the I inverse buckets when I is a large and not contiguous
    //--------------------------------------------------------------------------

    int64_t *Mark = NULL ;
    int64_t *Inext = NULL ;
    int64_t *Icol1 = NULL ;
    int64_t *Icol [2] = { NULL, NULL } ;
    int64_t nduplicates = 0 ;
    int64_t flag = 0 ;
    double memory = 0 ;

    bool ni_large = (!contig && ni > anrows / 256) ;

    if (ni_large)
    {

        // I is a large list relative to the number of rows of A, and it is not
        // contiguous (imax:imax).  Scatter I into the I inverse buckets
        // (Mark and Inext) for quick lookup.

        //----------------------------------------------------------------------
        // ensure Work is large enough for the scattered form of I
        //----------------------------------------------------------------------

        int64_t iworksize = ni ;            // Inext [ni]

        #ifndef SYMBOLIC
        if (need_qsort)
        {
            iworksize += ni ;               // Icol1 [ni]
        }
        #endif

        // memory space for Mark, Inext, and Icol1
        memory = GBYTES (anrows + iworksize, sizeof (int64_t)) ;

        if (!GB_Work_alloc (iworksize, sizeof (int64_t)))
        {
            // out of memory for Work
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }

        Inext = (int64_t *) GB_thread_local.Work ;

        #ifndef SYMBOLIC
        if (need_qsort)
        {
            // Icol workspace is only needed if the row indices I are jumbled,
            // and only for GB_subref_numeric
            Icol1 = Inext + ni ;            // size ni
            Icol [0] = NULL ;               // this is assigned later
            Icol [1] = Icol1 ;
        }
        #endif

        //----------------------------------------------------------------------
        // ensure Mark is large enough for Mark, of size anrows
        //----------------------------------------------------------------------

        if (!GB_Mark_alloc (anrows))
        {
            // out of memory for Mark
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }

        // ensure flag + ni does not overflow
        Mark = GB_thread_local.Mark ;
        flag = GB_Mark_reset (1, ni) ;

        //----------------------------------------------------------------------
        // scatter the row indices into buckets
        //----------------------------------------------------------------------

        // at this point, Mark is clear, so Mark [i] < flag for all i in
        // the range 0 to anrows-1.

        ASSERT (I != NULL) ;
        for (int64_t inew = ni-1 ; inew >= 0 ; inew--)
        {
            int64_t i = I [inew] ;
            int64_t ihead = (Mark [i] - flag) ;
            if (ihead < 0)
            {
                // first time row i has been seen in the list I
                ihead = -1 ;
            }
            else
            {
                // row i has already been seen in the list I
                nduplicates++ ;
            }
            Mark [i] = inew + flag ;       // (Mark [i] - flag) = inew
            Inext [inew] = ihead ;
        }

        // Row indices in I are now in buckets.  A row index i might appear
        // more than once in the list I.  inew = (Mark [i] - flag) is the
        // first position of row i in I (i will be I [inew]), (Mark [i] -
        // flag) is the head of a link list of all places where row i appears
        // in I.  inew = Inext [inew] traverses this list, until inew is -1.

        // If Mark [i] < flag, then the ith bucket is empty and i is not in I.
        // Otherise, the first row index in bucket i is (Mark [i] - flag).

        #ifndef NDEBUG
        // no part of this code takes O(anrows) time, except this debug test
        for (int64_t i = 0 ; i < anrows ; i++)
        {
            for (int64_t inew = Mark [i] - flag ; inew >= 0 ;
                 inew = Inext [inew])
            {
                ASSERT (inew >= 0 && inew < ni) ;
                ASSERT (i == I [inew]) ;
            }
        }
        #endif
    }

    //--------------------------------------------------------------------------
    // allocate initial space for C; this will grow as needed
    //--------------------------------------------------------------------------

    const int64_t *Ap = A->p ;
    const int64_t *Ai = A->i ;
    const void *Ax = A->x ;
    const int64_t asize = A->type->size ;

    // Estimating nnz(C) is difficult.  If the row index list I has lots of
    // duplicates, then a very sparse A can result in a dense A(I,J).  Or A can
    // be very dense and I very large, yet A(I,J) can be completely empty.
    // Estimating nnz(C) = nnz(A) could be severe overkill, particularly for
    // very large user-defined types.  This space is doubled if it is
    // exhausted, so start small and let it grow.

    // If A_transpose is false, C = A (I,J) is constructed and returned as the
    // output matrix, and its row indices must be sorted as it is created, by
    // qusort if needed.  If A_transpose is true, then I and J have been
    // swapped.  C = A(I,J) is extracted first, and then C is transposed into
    // the output matrix.  Its row indices are temporarily left in a jumbled
    // state, but these are sorted when the matrix is transposed at the end.
    // In both cases, the C matrix constructed here is ni-by-nj, and its row
    // indices are always sorted on output.

    // Since the SYMBOLIC method, GB_subref_symbolic, never transposes the
    // matrix A, it must rely on qsort if needed.  In that case, A_transpose
    // has a compile-time value of false.

    // [ be careful;  C->p is malloc'ed and the content is not initialized
    double Cp_memory = GBYTES (nj+1, sizeof (int64_t)) ;
    GrB_Matrix C ;

    #ifdef SYMBOLIC
    GrB_Type C_type = GrB_INT64 ;       // extract the pattern
    #else
    GrB_Type C_type = A->type ;         // extract the values
    #endif

    GB_NEW (&C, C_type, ni, nj, false, true) ;

    if (info != GrB_SUCCESS)
    {
        // out of memory for C
        GB_Mark_free ( ) ;
        GB_Work_free ( ) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory + Cp_memory))) ;
    }

    // be careful, C is not fully initialized; C->p is merely malloc'd
    ASSERT (C->p != NULL && C->magic == MAGIC2) ;

    int64_t cnz ;
    bool check_realloc ;

    if (ni == 0 || nj == 0)
    {
        // C will be empty
        cnz = 0 ;
        check_realloc = false ;
    }
    if (ni == 1)
    {
        // C = A (i,:) ; this assumes the row is dense.  Never need to realloc
        cnz = nj ;
        check_realloc = false ;
    }
    else if (nj == 1)
    {
        // C = A (..., j) for a single colum j
        int64_t j = (J == GrB_ALL) ? 0 : J [0] ;
        if (I == GrB_ALL)
        {
            // C = A (:,j) for a single column j.  This is exact.
            int64_t ajnz = Ap [j+1] - Ap [j] ;
            cnz = ajnz ;
        }
        else if (nduplicates == 0)
        {
            // C = A (I,j) for a single column j, where I has no duplicates.
            // This is exact.
            int64_t ajnz = Ap [j+1] - Ap [j] ;
            cnz = IMIN (ni, ajnz) ;
        }
        else
        {
            // C = A (I,j) for a single column j, where I has duplicates,
            // and so C could be dense even if A(:,j) is sparse
            cnz = ni ;
        }
        // for a single column, cnz is a strict upper bound on nnz(C), so
        // no need to check the realloc condition
        check_realloc = false ;
    }
    else
    {
        // C = A (I,J) ; account for some duplicates, also ni extra elbow room
        // in case J = [ ], so the last realloc doesn't hit the cnz+ni > nzmax
        // threshold.
        cnz = ni + 3*nduplicates ;
        for (int64_t jnew = 0 ; jnew < nj ; jnew++)
        {
            int64_t j = (J == GrB_ALL) ? jnew : J [jnew] ;
            int64_t ajnz = Ap [j+1] - Ap [j] ;
            cnz += IMIN (ni, ajnz) ;
        }
        check_realloc = true ;
    }

    // make sure C has space for at least a single entry
    cnz = IMAX (cnz, 1) ;

    // allocate space for the result C.  Note that C->p is allocated but
    // not yet initialized, so GB_Matrix_alloc must tolerate that case.
    double Cix_memory = 0 ;
    if (!GB_Matrix_alloc (C, cnz, true, &Cix_memory))
    {
        // That was too much.  Try again with something very small.
        check_realloc = true ;
        Cix_memory = 0 ;
        if (!GB_Matrix_alloc (C, 16, true, &Cix_memory))
        {
            // This is a trivial amount of memory so don't bother reporting
            // memory requirements.  If it fails it means either malloc
            // debugging is enabled (most likely), or memory is so totally
            // packed not even 16 entries can be allocated (which would be very
            // unusual unless there's an infinite-loop with a memory leak) .
            GB_MATRIX_FREE (&C) ;
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG, "out of memory"))) ;
        }
    }

    //--------------------------------------------------------------------------
    // C = A (I,J)
    //--------------------------------------------------------------------------

    #ifdef SYMBOLIC
    int64_t *Cx = C->x ;        // extract the pattern
    #else
    void    *Cx = C->x ;        // extract the values
    #endif
    int64_t *Ci = C->i ;
    int64_t *Cp = C->p ;
    cnz = 0 ;

    for (int64_t jnew = 0 ; jnew < nj ; jnew++)
    {

        //----------------------------------------------------------------------
        // construct column C (:,jnew) = A (I,j)
        //----------------------------------------------------------------------

        int64_t j = (J == GrB_ALL) ? jnew : J [jnew] ;

        // log the start of C (:, jnew)
        Cp [jnew] = cnz ;

        // if I is empty, all rows in A(:,j) will be outside [imin..imax]
        ASSERT ((ni == 0) == (imin == anrows && imax == -1))

        int64_t pstart = Ap [j] ;
        int64_t pend = Ap [j+1] - 1 ;
        int64_t ajnz = pend - pstart + 1 ;
        if (ajnz == 0)
        {
            continue ;      // no entries in A (:,j)
        }
        if (AI (pstart) > imax || AI (pend) < imin)
        {
            continue ;      // all rows in A (:,j) are outside [imin..imax]
        }

        ASSERT (ni > 0) ;

        // make sure C has enough space for the new entries
        // cnz + ni is safe against integer overflow
        if (check_realloc && cnz + ni > C->nzmax)
        {
            // double the space, and add an extra ni as well.
            // Note that C->p is not yet fully computed so GB_Matrix_realloc
            // must tolerate that condition
            GrB_Index nzmax ;
            bool ok = GB_Index_multiply (&nzmax, 2, C->nzmax + ni) ;
            if (!ok || !GB_Matrix_realloc (C, nzmax, true, &Cix_memory))
            {
                // out of memory
                GB_MATRIX_FREE (&C) ;
                GB_Mark_free ( ) ;
                GB_Work_free ( ) ;
                return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                    "out of memory, %g GBytes required",
                    memory + Cp_memory + Cix_memory))) ;
            }
            Cx = C->x ;
            Ci = C->i ;
        }

        //----------------------------------------------------------------------
        // binary search in A(:,j) for first row index i >= imin
        //----------------------------------------------------------------------

        // This phase does nothing if I is NULL since imin = 0 in that case.
        // It assists in other cases by trimming the search space of A(:,j),
        // by advancing pstart.  When done, the entries in I that could be
        // in A(:,j) are in the list Ai [pstart...pend].

        // Time taken for this step is at most O(log(ajnz)), in general, but
        // only O(1) time if I is NULL (because in that case imin = 0).

        if (AI (pstart) < imin)
        {
            ASSERT (Ap [j] <= pstart && pstart <= pend) ;
            ASSERT (AI (pstart) <= imin) ;

            int64_t pleft = pstart ;
            int64_t pright = pend ;
            #ifdef SYMBOLIC
            GB_BINARY_TRIM_ZOMBIE (imin, Ai, pleft, pright) ;
            #else
            GB_BINARY_TRIM_SEARCH (imin, Ai, pleft, pright) ;
            #endif

            pstart = pleft ;
        }

        // If pstart has been advanced, the entry Ai [pstart-1] before it must
        // be less than imin, and can thus be ignored in all subsequent searches
        ASSERT (Ap [j] <= pstart && pstart <= pend && pend == Ap [j+1]-1) ;
        ASSERT (imin <= AI (pstart)) ;
        ASSERT (IMPLIES (Ap [j] < pstart, AI (pstart-1) < imin)) ;

        // ajnz2 is the number of entries in A(:,j) that still need searching.
        int64_t ajnz2 = pend - pstart + 1 ;

        //----------------------------------------------------------------------
        // ready to do the work
        //----------------------------------------------------------------------

        // Summary of the seven different cases, applied to each column j.  Let
        // ajnz = nnz (A (:,j)), and let cjnz == nnz (C (:,jnew)), the number
        // of entries in the new column of C.  Let ni == length (I).  Each case
        // is preceded by a binary search of A (:,j) for the row index imin =
        // min (I).  Let ajnz2 = nnz (A (imin:end,j)).  The binary search takes
        // log (ajnz) time, unless all entries in A (:,j) are less than imin.
        // In that case, the binary search is skipped.  If I = ":" (GrB_ALL),
        // then imin = 0 and the first binary search is skipped.

        #if 0
        if (ni == 1)
        {
            // case 1: one row
            // C (:,jnew) = A (i,j)
            // time: O(log(ajnz)), optimal given the data structure
        }
        else if (I == GrB_ALL)
        {
            // case 2: all rows, I is null
            // C (:,jnew) = A (:,j)
            // time: O(ajnz), optimal
        }
        else if (contig)
        {
            // case 3: contig I = imin:imax
            // C (:,jnew) = A (imin:imax,j)
            // time: O(log(ajnz) + cjnz), optimal given the data structure
        }
        else if (!ni_large ||                   // must do case 4
            (need_qsort  && ni < ajnz2) ||      // case 4 faster than case 5
            (!need_qsort && 32 * ni < ajnz2))   // case 4 faster than case 6, 7
        {
            // case 4: ni not large
            // C (:,jnew) = A (I,j)
            // time: O(ni*log(ajnz2))
        }
        else if (need_qsort)
        {
            // case 5: ni large, need qsort
            // C (:,jnew) = A (I,j)
            // time: O(cjnz*log(cjnz) + ajnz2)
        }
        else if (nduplicates > 0)
        {
            // case 6: ni large, no qsort, with duplicates
            // C (:,jnew) = A (I,j)
            // time: O(cjnz + ajnz2)
        }
        else
        {
            // case 7: ni large, no qsort, no dupl
            // C (:,jnew) = A (I,j)
            // time: O(ajnz2)
        }
        #endif

        //----------------------------------------------------------------------
        // extract A (I,j): consider 7 cases
        //----------------------------------------------------------------------

        if (ni == 1)
        {

            //------------------------------------------------------------------
            // CASE 1: the list I has a single row index, imin
            //------------------------------------------------------------------

            // binary search has already found imin. Time is O(1) for this
            // step, or O(log(ajnz)) total for this column.

            if (AI (pstart) == imin)
            {
                Ci [cnz] = 0 ;
                #ifdef SYMBOLIC
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
        else if (I == GrB_ALL)
        {

            //------------------------------------------------------------------
            // CASE 2: I = 0:anrows-1, thus C(:,jnew) = A (:,j)
            //------------------------------------------------------------------

            // Total time is O(ajnz), but that entire time is required since
            // it is also the same as nnz (C (:,jnew)).

            ASSERT (cnz + ajnz <= C->nzmax) ;
            #ifdef SYMBOLIC
            {
                // extract the positions, not the values
                // Ci [cnz:cnz+ajnz-1] = UNFLIP (Ai [pstart:pstart+ajnz-1]) ;
                // Cx [cnz:cnz+ajnz-1] = [pstart:pstart+ajnz-1] ;
                for (int64_t k = 0 ; k < ajnz ; k++)
                {
                    Ci [cnz + k] = AI (pstart + k) ;
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
        else if (contig)
        {

            //------------------------------------------------------------------
            // CASE 3: I is a contiguous list, imin:imax
            //------------------------------------------------------------------

            // Total time is O(C(:,jnew) + log(ajnz)), which is nearly optimal

            int64_t cnz1 = cnz ;
            for (int64_t p = pstart ; p <= pend ; p++)
            {
                // A(i,j) is nonzero; no need to look it up in the I buckets
                int64_t i = AI (p) ;
                if (i > imax)
                {
                    // no more entries in A(:,j) in range of imin:imax
                    break ;
                }
                ASSERT (i == I [i-imin]) ;
                Ci [cnz] = i-imin ;
                cnz++ ;
                ASSERT (cnz <= C->nzmax) ;
            }
            #ifdef SYMBOLIC
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
        else if (!ni_large ||                   // must do case 4
            (need_qsort  && ni < ajnz2) ||      // case 4 faster than case 5
            (!need_qsort && 32 * ni < ajnz2))   // case 4 faster than case 6, 7
        {

            //------------------------------------------------------------------
            // CASE 4: I is short compared with nnz (A (:,j)), use binary search
            //------------------------------------------------------------------

            // If ni_large is false then the I bucket inverse has not been
            // created, so this method is the only option.  Alternatively, if
            // ni = length (I) is small relative to nnz (A (:,j)), then
            // scanning I and doing a binary search of A (:,j) is faster than
            // doing a linear-time search of A(:,j) and a lookup into the I
            // bucket inverse.

            // Time taken is O(ni*log(ajnz2)) for this step

            // Ai [pstart..pend] contains all entries in the range imin:imax,
            // but pend is just Ap[j+1]-1.  Only pstart has been advanced.  Do
            // another binary search for imax, to prune the search space at the
            // bottom of A(:,j).

            int64_t pleft = pstart ;
            int64_t pright = pend ;
            #ifdef SYMBOLIC
            GB_BINARY_TRIM_ZOMBIE (imax, Ai, pleft, pright) ;
            #else
            GB_BINARY_TRIM_SEARCH (imax, Ai, pleft, pright) ;
            #endif

            // x = Ai [p] where p == pleft == pright.  The item searched for
            // is imax, and either x=imax and it is found, or imax is not found
            // and or x is the first entry in the list greater than imax.

            pend = pright ;

            ASSERT (Ap [j] <= pstart && pstart <= pend && pend <= Ap [j+1]-1) ;
            ASSERT (IMPLIES (Ap [j] < pstart, AI (pstart-1) < imin)) ;
            ASSERT (IMPLIES (pend < Ap [j+1]-1, imax < AI (pend+1))) ;

            // scan I, in order, and search for the entry in A(:,j)
            for (int64_t inew = 0 ; inew < ni ; inew++)
            {
                // A (i,j) will become C (inew,jnew), if it exists
                int64_t i = I [inew] ;

                bool found, is_zombie ;
                int64_t pleft = pstart ;
                int64_t pright = pend ;
                #ifdef SYMBOLIC
                GB_BINARY_ZOMBIE (i, Ai, pleft, pright, found,
                    A->nzombies, is_zombie) ;
                #else
                GB_BINARY_SEARCH (i, Ai, pleft, pright, found) ;
                #endif

                if (found)
                {
                    Ci [cnz] = inew ;
                    #ifdef SYMBOLIC
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
        else if (need_qsort)
        {

            //------------------------------------------------------------------
            // CASE 5: I unsorted, and C needs qsort, with or without duplicates
            //------------------------------------------------------------------

            // works well when I has many entries and A(:,j) has few entries.
            // Time taken is O(cjnz*log(cjnz)+ajnz2) in the worst case, where
            // cjnz <= ni.  cjnz and ajnz2 are not comparable because of
            // duplicates.

            // The row indices Ci for C(:,j) are sorted in-place
            int64_t *Icol0 = &Ci [cnz] ;
            Icol [0] = Icol0 ;

            #ifdef SYMBOLIC
            // The pointers Cx are also sorted in-place
            int64_t *Icol1 = &Cx [cnz] ;
            Icol [1] = Icol1 ;
            #endif

            // place row indices in Icol [0..cjnz-1][0] and positions in [..][1]
            int64_t cjnz = 0 ;
            for (int64_t p = pstart ; p <= pend ; p++)
            {
                // A(i,j) is nonzero, look it up in the I inverse buckets
                int64_t i = AI (p) ;
                if (i > imax)
                {
                    // no more entries in A(:,j) in range of imin:imax
                    break ;
                }
                // traverse bucket i for all rows inew such that i == I [inew]
                ASSERT (Mark != NULL) ;
                ASSERT (Icol0 != NULL) ;
                ASSERT (Icol1 != NULL) ;
                ASSERT (Inext != NULL) ;
                for (int64_t inew = Mark [i] - flag ; inew >= 0 ;
                     inew = Inext [inew])
                {
                    ASSERT (&Icol0 [cjnz] == &Ci [cnz + cjnz]) ;
                    ASSERT (inew >= 0 && inew < ni) ;
                    ASSERT (i == I [inew]) ;
                    Icol0 [cjnz] = inew ;           // same as Ci [cnz+cjnz]
                    Icol1 [cjnz] = p ;              // position p in Ai, Ax [p]
                    cjnz++ ;
                }
            }

            ASSERT (cjnz <= ni) ;
            // sort Icol [0..1][0..cjnz-1] using a single key, Icol [0][:]
            GB_qsort_2a (Icol [0], Icol [1], cjnz) ;

            // Icol [0] is now a pointer to the sorted row indices
            // Icol [1] is now a pointer to the corresponding positions in Ax

            #ifdef SYMBOLIC
            {
                // the symbolic work is done; the row indices and pointers have
                // been sorted in-place in [Ci,Cx]
                cnz += cjnz ;
            }
            #else
            {
                // now copy Icol into C (:,jnew)
                for (int64_t k = 0 ; k < cjnz ; k++)
                {
                    ASSERT (Ci [cnz] == Icol0 [k]) ;    // already done
                    int64_t p = Icol1 [k] ;
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

            //------------------------------------------------------------------
            // CASE 6: I not contiguous, with duplicates.  No qsort needed.
            //------------------------------------------------------------------

            // works well when I has many entries and A(:,j) has few entries.
            // Time taken is O(cjnz+ajnz2) in the worst case, where cjnz =
            // nnz (C (:,jnew)) and ajnz2 < nnz (A (:,j)).

            for (int64_t p = pstart ; p <= pend ; p++)
            {
                // A(i,j) is nonzero, look it up in the I inverse buckets
                int64_t i = AI (p) ;
                if (i > imax)
                {
                    // no more entries in A(:,j) in range of imin:imax
                    break ;
                }
                // traverse bucket i for all rows inew such that i == I [inew]
                ASSERT (Mark != NULL) ;
                for (int64_t inew = Mark [i] - flag ; inew >= 0 ;
                     inew = Inext [inew])
                {
                    ASSERT (inew >= 0 && inew < ni) ;
                    ASSERT (i == I [inew]) ;
                    Ci [cnz] = inew ;
                    #ifdef SYMBOLIC
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

            //------------------------------------------------------------------
            // CASE 7: I not contiguous, no duplicates.  No qsort needed.
            //------------------------------------------------------------------

            // works well when I has many entries and A(:,j) has few entries.
            // Time taken is O(ajnz2)

            for (int64_t p = pstart ; p <= pend ; p++)
            {
                // A(i,j) is nonzero, look it up in I inverse buckets
                int64_t i = AI (p) ;
                if (i > imax)
                {
                    // no more entries in A(:,j) in range of imin:imax
                    break ;
                }
                // bucket i has at most one row inew such that i == I [inew]
                ASSERT (Mark != NULL) ;
                int64_t inew = Mark [i] - flag ;
                if (inew >= 0)
                {
                    ASSERT (inew >= 0 && inew < ni) ;
                    ASSERT (i == I [inew]) ;
                    Ci [cnz] = inew ;
                    #ifdef SYMBOLIC
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
                }
            }
        }
    }

    // log the end of the last column of C
    Cp [nj] = cnz ;

    // C was created with GB_new, which set C->magic to zero.  Now that C->p
    // has been properly initialized, C->magic = MAGIC, flagging the matrix as
    // initialized.  If A_transpose is true, C may have jumbled row indices.
    C->magic = MAGIC ;  // C now initialized ]

    // The right bracket above is used during code development.  Place your
    // cursor on the bracket and ask your editor to find the matching bracket.
    // You will be taken to the place where GB_new created the matrix with
    // C->magic = MAGIC2, and the "be careful..." statement.

    ASSERT_OK_OR_JUMBLED (GB_check (C, "subref C=A(I,J), almost done", 0)) ;

    //--------------------------------------------------------------------------
    // clear the Mark array
    //--------------------------------------------------------------------------

    GB_Mark_reset (ni + 1, 0) ;

    //--------------------------------------------------------------------------
    // create output C; transpose C if needed
    //--------------------------------------------------------------------------

    if (A_transpose)
    {
        // (*handle) = C', and free C.  This also does a bucket sort
        // [ (*handle)->p malloc'ed, not initialized
        GB_NEW (handle, A->type, nj, ni, false, true) ;
        if (info != GrB_SUCCESS)
        {
            // out of memory
            GB_MATRIX_FREE (&C) ;
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (info) ;
        }
        // No typecasting since C->type == A->type.  No operator.
        // The input matrix C may be jumbled.
        ASSERT (C->type == A->type) ;
        info = GB_Matrix_transpose (*handle, C, NULL, true) ;
        // (*handle)->p now initialized ]
        GB_MATRIX_FREE (&C) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (handle) ;
            ASSERT (*handle == NULL) ;
            GB_Mark_free ( ) ;
            GB_Work_free ( ) ;
            return (info) ;
        }
    }
    else
    {
        // (*handle) = C, resized to actual number of entries. This cannot fail.
        ASSERT (cnz <= C->nzmax) ;
        if (cnz < C->nzmax)
        {
            bool ok = GB_Matrix_realloc (C, cnz, true, NULL) ;
            ASSERT (ok) ;
        }
        *handle = C ;
    }

    // C is now OK, with sorted row indices
    ASSERT_OK (GB_check (*handle, "C output for  C=A(I,J)", 0)) ;
    return (REPORT_SUCCESS) ;
}

#undef SYMBOLIC
#undef AI
#ifdef A_transpose
#undef A_transpose
#endif


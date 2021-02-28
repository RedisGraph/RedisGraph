//------------------------------------------------------------------------------
// tri_template: count triangles in a graph, outer-product method
//------------------------------------------------------------------------------

// Compute the # of triangles in a graph, C<A>=A*A in GraphBLAS notation, then
// ntri=sum(C).  Or, in MATLAB notation, ntri = sum (sum ((A*A).*A)).  C=A*A is
// computed using an outer-product matrix multiplication.  C is not computed
// explicitly, but its entries are summed up in the scalar ntri.

// A is a binary matrix stored in compressed sparse column form.  Its values
// are not stored.  If A(i,j) is in the pattern, its value is assumed to be 1.
// The pattern of column j is in Ai [Ap [j]..Ap[j+1]].  Row indices in the
// matrix A must be sorted.  Ap[0]=0, and Ap [n] = total number of entries in
// the matrix.  Ap is of size n+1.

// When this function is called, A is a triangular matrix (with no diagonal
// entries, or a symmetric permutation of such a triangular matrix.  However,
// this function works on any matrix.  It just computes sum(sum((A*A).*A) in
// MATLAB notation, or C<A>=A*A where A is binary, followed by reduce(C), to
// scalar.

// So it can be used with C<L>=L*L or C<U>=U*U, and ntri is the number of
// triangles.  It can also be used as C<A>=A*A where A is symmetric, in
// which case the # of triangles is ntri/6 (Burkhardt's method).

// This file creates eight methods via compile-time definitions:
//
// BIT:       if defined, Mark is a bit vector of size n.  Otherwise it is a
//            bool array of size n.  This can help cut workspace if many
//            threads are used since each thread needs its own Mark array.
// PARALLEL:  if defined, then OpenMP is used
// LOGSEARCH: if binary search is used to reduce the work

// Compare this code with tri_simple.c.  That code is a simple version of this
// algorithm, with the bare essential features.

#ifdef BIT

#define MARK_TYPE     uint8_t
#define MARK_SIZE     (1 + n/8)
#define SET_MARK(i)   { Index t=(i) ; Mark [t/8] |= (1 << (t%8)) ; }
#define CLEAR_MARK(i) { Mark [(i)/8] = 0 ; }
#define COUNT_MARK(i) { Index t=(i) ; if (Mark [t/8] & (1 << t%8)) ntri++ ; }

#else

#define MARK_TYPE     bool
#define MARK_SIZE     n
#define SET_MARK(i)   { Mark [i] = 1 ; }
#define CLEAR_MARK(i) { Mark [i] = 0 ; }
#define COUNT_MARK(i) { ntri += Mark [i] ; }

#endif

#ifdef LOGSEARCH
    #ifdef PARALLEL
        #ifdef BIT
            #define TRI_FUNCTION  tri_logbit_parallel
        #else
            #define TRI_FUNCTION  tri_logmark_parallel
        #endif
    #else
        #ifdef BIT
            #define TRI_FUNCTION  tri_logbit
        #else
            #define TRI_FUNCTION  tri_logmark
        #endif
    #endif
#else
    #ifdef PARALLEL
        #ifdef BIT
            #define TRI_FUNCTION  tri_bit_parallel
        #else
            #define TRI_FUNCTION  tri_mark_parallel
        #endif
    #else
        #ifdef BIT
            #define TRI_FUNCTION  tri_bit
        #else
            #define TRI_FUNCTION  tri_mark
        #endif
    #endif
#endif

//------------------------------------------------------------------------------
// tri_* function: count the triangles in a graph
//------------------------------------------------------------------------------

int64_t TRI_FUNCTION                // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices, size nz = Ap [n]
    const Index n                   // A is n-by-n
    #ifdef PARALLEL
    , const int threads             // # of threads
    , const Index chunk             // scheduler chunk size
    #endif
)
{

    int64_t ntri = 0 ;      // # of triangles
    bool ok = true ;        // false if any thread ran out of memory

    //--------------------------------------------------------------------------
    // check if sequential version of same algorithm should be used
    //--------------------------------------------------------------------------

    #ifdef PARALLEL
    if (n < chunk || threads < 2)
    {
        #ifdef LOGSEARCH
            #ifdef BIT
            return (tri_logbit (Ap, Ai, n)) ;
            #else
            return (tri_logmark (Ap, Ai, n)) ;
            #endif
        #else
            #ifdef BIT
            return (tri_bit (Ap, Ai, n)) ;
            #else
            return (tri_mark (Ap, Ai, n)) ;
            #endif
        #endif
    }
    #endif

    //--------------------------------------------------------------------------
    // parallel and sequential triangle counting, outer-product method
    //--------------------------------------------------------------------------

    #ifdef PARALLEL
    #pragma omp parallel num_threads(threads) reduction(+:ntri) reduction(&&:ok)
    #endif
    {

        //----------------------------------------------------------------------
        // get workspace
        //----------------------------------------------------------------------

        // each thread needs its own private workspace, Mark [0..n-1] = 0
        MARK_TYPE *restrict Mark = calloc (MARK_SIZE, sizeof (MARK_TYPE)) ;
        if (Mark == NULL)
        {
            ok = false ;
        }
        else
        {

            //------------------------------------------------------------------
            // count triangles in each column C(:,j)
            //------------------------------------------------------------------

            #ifdef PARALLEL
            #pragma omp for schedule(dynamic,chunk)
            #endif
            for (Index j = 0 ; j < n ; j++)
            {

                //--------------------------------------------------------------
                // get column j of A
                //--------------------------------------------------------------

                // A(:,j) has row indices in range jlo..jhi
                Index jlo, jhi ;
                if (!tri_lohi (Ap, Ai, j, &jlo, &jhi)) continue ;

                bool marked = false ;

                #ifdef LOGSEARCH
                Index ljnz = jhi - jlo + 1 ;
                #endif

                //--------------------------------------------------------------
                // compute sum(C(:,j)) where C=(A*A(:,j))*.(A(:,j))
                //--------------------------------------------------------------

                for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                {

                    //----------------------------------------------------------
                    // A(k,j) is present, compute C(:,j) += A(:,j)*A(k,j)
                    //----------------------------------------------------------

                    const Index k = Ai [p] ;

                    // A(:,k) has row indices in range klo..khi
                    Index klo, khi ;
                    if (!tri_lohi (Ap, Ai, k, &klo, &khi)) continue ;

                    // skip if A(:,j) and A(:,k) do not overlap
                    if (khi < jlo || klo > jhi) continue ;

                    //----------------------------------------------------------
                    // binary search if A(:,k) has many nonzeros
                    //----------------------------------------------------------

                    #ifdef LOGSEARCH

                    // find the intersection between the mask, A(:,j),
                    // and the column A(:,k)

                    Index lknz = khi - klo + 1 ;

                    if (512 * ljnz < lknz) // (4 * ljnz * log2 (lknz) < lknz)
                    {

                        //------------------------------------------------------
                        // A (:,j) is very sparse compared with A (:,k) ;
                        //------------------------------------------------------

                        // Do not use the Mark array at all, but use binary
                        // search instead.  time is O(ljnz * log (lknz))
                        int64_t pleft  = Ap [k] ;
                        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                        {
                            // find i in A (:,k)
                            Index i = Ai [p] ;
                            // binary search of Ai [pleft ... pright] for i
                            int64_t pright = Ap [k+1] - 1 ;
                            while (pleft < pright)
                            {
                                int64_t pmiddle = (pleft + pright) / 2 ;
                                if (i > Ai [pmiddle])
                                {
                                    // if in the list, it appears in
                                    // [pmiddle+1..pright]
                                    pleft = pmiddle + 1 ;
                                }
                                else
                                {
                                    // if in the list, it appears in
                                    // [pleft..pmiddle]
                                    pright = pmiddle ;
                                }
                            }
                            if (pleft == pright && Ai [pleft] == i)
                            {
                                // found it:  A(i,k) and A (k,j) both nonzero
                                // C(i,j) += A (i,k) * A (k,j)
                                ntri++ ;
                            }
                        }
                        continue ;
                    }
                    #endif

                    //----------------------------------------------------------
                    // linear search
                    //----------------------------------------------------------

                    if (!marked)
                    {
                        // scatter A(:,j) into Mark
                        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                        {
                            // Mark [Ai [p]] = 1 ;
                            SET_MARK (Ai [p]) ;
                        }
                        marked = true ;
                    }

                    for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                    {
                        // C(i,j) += A (i,k) * A (k,j)
                        COUNT_MARK (Ai [pa]) ;
                    }
                }

                //--------------------------------------------------------------
                // clear the Mark array
                //--------------------------------------------------------------

                if (marked)
                {
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        // Mark [Ai [p]] = 0 ;
                        CLEAR_MARK (Ai [p]) ;
                    }
                }
            }

            //------------------------------------------------------------------
            // free workspace
            //------------------------------------------------------------------

            free (Mark) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (ok ? ntri : -1) ;
}

#undef BIT
#undef PARALLEL
#undef MARK_TYPE
#undef MARK_SIZE
#undef SET_MARK
#undef CLEAR_MARK
#undef COUNT_MARK
#undef TRI_FUNCTION
#undef LOGSEARCH


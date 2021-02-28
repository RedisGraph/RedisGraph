//------------------------------------------------------------------------------
// tri_dot: compute the number of triangles in a graph, using dot products 
//------------------------------------------------------------------------------

// Compute the # of triangles in a graph, C<L>=U'*L in GraphBLAS notation, then
// ntri=sum(C).  Or, in MATLAB notation, ntri = sum (sum ((U'*L).*L)).

// L is a binary matrix stored in compressed sparse column form.  Its values
// are not stored.  If L(i,j) is in the pattern, its value is assumed to be 1.
// C is not computed.  The pattern of column j is in Li [Lp [j]..Lp[j+1]].  Row
// indices in the matrix L MUST be sorted.  Lp [0]=0, and Lp [n] = total
// number of entries in the matrix.  Lp is of size n+1.

// U is the transpose of L.  It also must have sorted row indices.  It can
// also be viewed as the compressed-row form of L.

// L can also be a symmetric permutation of a lower triangular matrix.  U must
// always be the transpose of L (or equivalently, the compressed-sparse row
// form of L).

#ifdef PARALLEL
#define TRI_DOT tri_dot_parallel
#else
#define TRI_DOT tri_dot
#endif

int64_t TRI_DOT                     // # of triangles
(
    const int64_t *restrict Lp,     // column pointers of L, size n+1
    const Index   *restrict Li,     // row indices of L
    const int64_t *restrict Up,     // column pointers of U, size n+1
    const Index   *restrict Ui,     // row indices of U
    const Index n                   // L and U are n-by-n
    #ifdef PARALLEL
    , const int threads             // # of threads
    , const Index chunk             // scheduler chunk size
    #endif
)
{

    int64_t ntri = 0 ;

    #ifdef PARALLEL
    if (n < chunk || threads < 2)
    {
        // punt to sequential version of the same algorithm
        return (tri_dot (Lp, Li, Up, Ui, n)) ;
    }

    #pragma omp parallel for num_threads(threads) reduction(+:ntri) schedule(dynamic,chunk)
    #endif
    for (Index j = 0 ; j < n ; j++)
    {

        //----------------------------------------------------------------------
        // compute sum(C(:,j)), where C(:,j) = (U' * L(:,j)) .* L(:,j)
        //----------------------------------------------------------------------

        // get the pattern of the mask, L(:,j)
        int64_t pl_start = Lp [j] ;
        int64_t pl_end   = Lp [j+1] ;
        if (pl_start == pl_end) continue ;

        // first and last row indices in L(:,j)
        Index ifirst = Li [pl_start] ;
        Index ilast  = Li [pl_end-1] ;

        for (int64_t p = pl_start ; p < pl_end ; p++)
        {

            //------------------------------------------------------------------
            // compute C(i,j) = U(:,i)' * L(:,j), via merge
            //------------------------------------------------------------------

            // C(i,j) is in the pattern of the mask, L

            // get the head of the L(:,j) list
            Index i = Li [p] ;
            int64_t pl = pl_start ;

            // get the head of the U(:,i) list
            int64_t pu = Up [i] ;
            int64_t pu_end = Up [i+1] ;

            // skip if U(:,i) is empty
            if (pu == pu_end) continue ;

            // skip if all entries in U(:,i) are outside [ifirst..ilast]
            if (Ui [pu_end-1] < ifirst || ilast < Ui [pu]) continue ;

            while (pl < pl_end && pu < pu_end)
            {

                //--------------------------------------------------------------
                // get the next values at the head of the Ui and Li lists
                //--------------------------------------------------------------

                Index iu = Ui [pu] ;
                Index il = Li [pl] ;

                if (iu < il)
                {

                    //----------------------------------------------------------
                    // U(iu,i) appears before L(il,j)
                    //----------------------------------------------------------

                    // consume entries from U(:,i) until reaching L(il,j)

                    // binary search of Ui [pleft ... pright] for integer il
                    int64_t pleft  = pu + 1 ;
                    int64_t pright = pu_end ;
                    while (pleft < pright)
                    {
                        int64_t pmiddle = (pleft + pright) / 2 ;
                        if (il > Ui [pmiddle])
                        {
                            // if in the list, it appears in [pmiddle+1..pright]
                            pleft = pmiddle + 1 ;
                        }
                        else
                        {
                            // if in the list, it appears in [pleft..pmiddle]
                            pright = pmiddle ;
                        }
                    }
                    pu = pleft ;

                }
                else if (il < iu)
                {

                    //----------------------------------------------------------
                    // L(il,j) appears before U(iu,i)
                    //----------------------------------------------------------

                    // consume entries from L(:,j) until reaching U(iu,i)

                    // binary search of Li [pleft ... pright] for integer iu
                    int64_t pleft  = pl + 1 ;
                    int64_t pright = pl_end ;
                    while (pleft < pright)
                    {
                        int64_t pmiddle = (pleft + pright) / 2 ;
                        if (iu > Li [pmiddle])
                        {
                            // if in the list, it appears in [pmiddle+1..pright]
                            pleft = pmiddle + 1 ;
                        }
                        else
                        {
                            // if in the list, it appears in [pleft..pmiddle]
                            pright = pmiddle ;
                        }
                    }
                    pl = pleft ;

                }
                else // iu == il == k
                {

                    //----------------------------------------------------------
                    // U(k,i) and L(k,j) are both present
                    //----------------------------------------------------------

                    // C(i,j) += U(k,i) * L(k,j)
                    ntri++ ;

                    // advance to the next rows in the U(:,i) and L(:,j) lists
                    pl++ ;
                    pu++ ;
                }
            }
        }
    }
    return (ntri) ;
}

#undef TRI_DOT
#undef PARALLEL

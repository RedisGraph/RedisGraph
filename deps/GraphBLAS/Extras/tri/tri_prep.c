//------------------------------------------------------------------------------
// tri_prep: remove edges from a graph, making it acyclic
//------------------------------------------------------------------------------

// Given a symmetric binary graph A with no self-edges, prune the edges to make
// it acyclic.  The resulting graph is a symmetric permutation of a lower
// triangular matrix.

// methods: where [~,p] = sort (sum (A)) ;

// 0: S = tril (A) ;
// 1: S = triu (A) ;
// 2: S (p,p) = tril (A (p,p)) ;
// 3: S (p,p) = triu (A (p,p)) ;

#include "tri_def.h"

//------------------------------------------------------------------------------
// dsort: sort the rows/cols of A by degree
//------------------------------------------------------------------------------

// returns a permutation vector perm that sorts the rows/columns of A by
// increasing degree.  perm[k]=j if column j is the kth column in the permuted
// matrix.  Ties are sorted by original column index.

static Index *dsort                 // return perm of size n
(
    const int64_t *restrict Ap,     // column pointers of A, size n+1
    const Index n                   // A is n-by-n
)
{

    // allocate perm and workspace
    Index *perm = malloc ((n+1) * sizeof (Index)) ;
    Index *head = malloc ((n+1) * sizeof (Index)) ;
    Index *next = malloc ((n+1) * sizeof (Index)) ;
    if (perm == NULL || head == NULL || next == NULL)
    {
        if (perm != NULL) free (perm) ;
        if (head != NULL) free (head) ;
        if (next != NULL) free (next) ;
        return (NULL) ;
    }

    // empty the degree buckets
    for (Index d = 0 ; d < n ; d++)
    {
        head [d] = -1 ;
    }

    // place column j in bucket of its degree d
    for (Index j = n-1 ; j >= 0 ; j--)
    {
        Index d = (Index) (Ap [j+1] - Ap [j]) ;
        next [j] = head [d] ;
        head [d] = j ;
    }

    // scan the buckets in increasing degree
    Index k = 0 ;
    for (Index d = 0 ; d < n ; d++)
    {
        // scan bucket d and append its contents to perm
        for (Index j = head [d] ; j != -1 ; j = next [j])
        {
            perm [k++] = j ;
        }
        if (k == n) break ;
    }

    // free workspace
    free (head) ;
    free (next) ;

    // return the permutation
    return (perm) ;
}

//------------------------------------------------------------------------------
// tri_prep: prune an undirected graph to make it acyclic
//------------------------------------------------------------------------------

// construct the pruned matrix S from the symmetric graph A

bool tri_prep                   // true if successful, false otherwise
(
    int64_t *restrict Sp,       // column pointers, size n+1
    Index *restrict Si,         // row indices
    const int64_t *restrict Ap, // column pointers, size n+1
    const Index *restrict Ai,   // row indices
    const Index n,              // A is n-by-n
    int method,                 // 0 to 3, see above
    int nthreads                // # of threads to use
)
{

    int64_t snz = 0 ;
    Index *perm ;

    int64_t anz = Ap [n] ;
    nthreads = MIN (nthreads, CEIL (anz, 100000)) ;

    if (nthreads <= 2)
    {

        //----------------------------------------------------------------------
        // sequential case
        //----------------------------------------------------------------------

        // If only 2 threads are used, then just use the sequential version.
        // The parallel version must pass over the data twice.

        switch (method)
        {

            //------------------------------------------------------------------
            case 0: // S = tril (A)
            //------------------------------------------------------------------

                for (Index j = 0 ; j < n ; j++)
                {
                    Sp [j] = snz ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (i > j)
                        {
                            Si [snz++] = i ;
                        }
                    }
                }
                Sp [n] = snz ;
                return (true) ;

            //------------------------------------------------------------------
            case 1: // S = triu (A)
            //------------------------------------------------------------------

                for (Index j = 0 ; j < n ; j++)
                {
                    Sp [j] = snz ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (i < j)
                        {
                            Si [snz++] = i ;
                        }
                    }
                }
                Sp [n] = snz ;
                return (true) ;

            //------------------------------------------------------------------
            case 2: // sort by increasing degree:  S (p,p) = tril (A (p,p))
            //------------------------------------------------------------------

                perm = dsort (Ap, n) ;
                if (perm == NULL) return (false) ;
                for (Index j = 0 ; j < n ; j++)
                {
                    Sp [j] = snz ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (perm [i] > perm [j])
                        {
                            Si [snz++] = i ;
                        }
                    }
                }
                Sp [n] = snz ;
                free (perm) ;
                return (true) ;

            //------------------------------------------------------------------
            case 3: // sort by decreasing degree:  S (p,p) = triu (A (p,p))
            //------------------------------------------------------------------

                perm = dsort (Ap, n) ;
                if (perm == NULL) return (false) ;
                for (Index j = 0 ; j < n ; j++)
                {
                    Sp [j] = snz ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (perm [i] < perm [j])
                        {
                            Si [snz++] = i ;
                        }
                    }
                }
                Sp [n] = snz ;
                free (perm) ;
                return (true) ;

            default: return (false) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // parallel case
        //----------------------------------------------------------------------

        switch (method)
        {

            //------------------------------------------------------------------
            case 0: // S = tril (A)
            //------------------------------------------------------------------

                // count the entries in each column of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = 0 ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (i > j)
                        {
                            jnz++ ;
                        }
                    }
                    Sp [j] = jnz ;
                }

                // construct the column pointers of S
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = Sp [j] ;
                    Sp [j] = snz ;
                    snz += jnz ;
                }
                Sp [n] = snz ;

                // construct the row indices of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t s = Sp [j] ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (i > j)
                        {
                            Si [s++] = i ;
                        }
                    }
                }

                return (true) ;

            //------------------------------------------------------------------
            case 1: // S = triu (A)
            //------------------------------------------------------------------

                // count the entries in each column of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = 0 ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (i < j)
                        {
                            jnz++ ;
                        }
                    }
                    Sp [j] = jnz ;
                }

                // construct the column pointers of S
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = Sp [j] ;
                    Sp [j] = snz ;
                    snz += jnz ;
                }
                Sp [n] = snz ;

                // construct the row indices of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t s = Sp [j] ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (i < j)
                        {
                            Si [s++] = i ;
                        }
                    }
                }

                return (true) ;

            //------------------------------------------------------------------
            case 2: // sort by increasing degree:  S (p,p) = tril (A (p,p))
            //------------------------------------------------------------------

                // sort the columns
                perm = dsort (Ap, n) ;
                if (perm == NULL) return (false) ;

                // count the entries in each column of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = 0 ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (perm [i] > perm [j])
                        {
                            jnz++ ;
                        }
                    }
                    Sp [j] = jnz ;
                }

                // construct the column pointers of S
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = Sp [j] ;
                    Sp [j] = snz ;
                    snz += jnz ;
                }
                Sp [n] = snz ;

                // construct the row indices of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t s = Sp [j] ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (perm [i] > perm [j])
                        {
                            Si [s++] = i ;
                        }
                    }
                }

                free (perm) ;
                return (true) ;

            //------------------------------------------------------------------
            case 3: // sort by decreasing degree:  S (p,p) = triu (A (p,p))
            //------------------------------------------------------------------

                // sort the columns
                perm = dsort (Ap, n) ;
                if (perm == NULL) return (false) ;

                // count the entries in each column of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = 0 ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (perm [i] < perm [j])
                        {
                            jnz++ ;
                        }
                    }
                    Sp [j] = jnz ;
                }

                // construct the column pointers of S
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t jnz = Sp [j] ;
                    Sp [j] = snz ;
                    snz += jnz ;
                }
                Sp [n] = snz ;

                // construct the row indices of S
                #pragma omp parallel for num_threads(nthreads)
                for (Index j = 0 ; j < n ; j++)
                {
                    int64_t s = Sp [j] ;
                    for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
                    {
                        Index i = Ai [p] ;
                        if (perm [i] < perm [j])
                        {
                            Si [s++] = i ;
                        }
                    }
                }

                free (perm) ;
                return (true) ;

            default: return (false) ;
        }
    }
}


//------------------------------------------------------------------------------
// tri_simple: compute the number of triangles in a graph (simplest method)
//------------------------------------------------------------------------------

// A bare-bones version of the many variants in tri_template.c, with no
// parallelism, no log-time binary search, no use of tri_lohi to cut the work.
// This function is most similar to tri_mark (sequential version), defined
// in tri_template.c.

// Computes the sum(sum((A*A).*A)), in MATLAB notation, where A is binary
// (only the pattern is present).  Or, in GraphBLAS notation,
// C<A> = A*A followed by reduce(C) to scalar.

#include "tri_def.h"

int64_t tri_simple          // # of triangles, or -1 if out of memory
(
    const int64_t *restrict Ap,     // column pointers, size n+1
    const Index   *restrict Ai,     // row indices
    const Index   n                 // A is n-by-n
)
{

    bool *restrict Mark = (bool *) calloc (n, sizeof (bool)) ;
    if (Mark == NULL) return (-1) ;
    int64_t ntri = 0 ;

    for (Index j = 0 ; j < n ; j++)
    {

        // scatter A(:,j) into Mark
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            Mark [Ai [p]] = 1 ;
        }
        // compute sum(C(:,j)) where C(:,j) = (A * A(:,j)) .* Mark
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            const Index k = Ai [p] ;
            // C(:,j) += (A(:,k) * A(k,j)) .* Mark
            for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
            {
                // C(i,j) += (A(i,k) * A(k,j)) .* Mark
                ntri += Mark [Ai [pa]] ;
            }
        }
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            Mark [Ai [p]] = 0 ;
        }
    }

    free (Mark) ;
    return (ntri) ;
}


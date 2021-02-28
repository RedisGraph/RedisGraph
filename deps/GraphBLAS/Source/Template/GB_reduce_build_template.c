//------------------------------------------------------------------------------
// GB_build_template: T=build(S), and assemble any duplicate tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This template is used in GB_builder and the Generated/GB_red_build__*
// workers.  This is the same for both vectors and matrices, since this step is
// agnostic about which vectors the entries appear.

{

    // k unused for some uses of this template
    #include "GB_unused.h"

    if (ndupl == 0)
    {

        //----------------------------------------------------------------------
        // no duplicates, just permute S into Tx
        //----------------------------------------------------------------------

        // If no duplicates are present, then GB_builder has already
        // transplanted I_work into T->i, so this step does not need to
        // construct T->i.  The tuple values, in S, are copied or permuted into
        // T->x.

        if (K_work == NULL)
        {

            int tid ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (tid = 0 ; tid < nthreads ; tid++)
            {
                int64_t tstart = tstart_slice [tid] ;
                int64_t tend   = tstart_slice [tid+1] ;
                for (int64_t t = tstart ; t < tend ; t++)
                { 
                    // Tx [t] = (ttype) S [t] ; with typecast
                    GB_CAST_ARRAY_TO_ARRAY (Tx, t, S, t) ;
                }
            }

        }
        else
        {

            int tid ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (tid = 0 ; tid < nthreads ; tid++)
            {
                int64_t tstart = tstart_slice [tid] ;
                int64_t tend   = tstart_slice [tid+1] ;
                for (int64_t t = tstart ; t < tend ; t++)
                { 
                    // Tx [t] = (ttype) S [K_work [t]] ; with typecast
                    GB_CAST_ARRAY_TO_ARRAY (Tx, t, S, K_work [t]) ;
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // assemble duplicates
        //----------------------------------------------------------------------

        // Entries in S must be copied into T->x, with any duplicates summed
        // via the operator.  T->i must also be constructed.

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (tid = 0 ; tid < nthreads ; tid++)
        {
            int64_t my_tnz = tnz_slice [tid] ;
            int64_t tstart = tstart_slice [tid] ;
            int64_t tend   = tstart_slice [tid+1] ;

            // find the first unique tuple owned by this slice
            int64_t t ;
            for (t = tstart ; t < tend ; t++)
            { 
                // get the tuple and break if it is not a duplicate
                if (I_work [t] >= 0) break ;
            }

            // scan all tuples and assemble any duplicates
            for ( ; t < tend ; t++)
            {
                // get the t-th tuple, a unique tuple
                int64_t i = I_work [t] ;
                int64_t k = (K_work == NULL) ? t : K_work [t] ;
                ASSERT (i >= 0) ;
                // Tx [my_tnz] = S [k] ; with typecast
                GB_CAST_ARRAY_TO_ARRAY (Tx, my_tnz, S, k) ;
                Ti [my_tnz] = i ;

                // assemble all duplicates that follow it.  This may assemble
                // the first duplicates in the next slice(s) (up to but not
                // including the first unique tuple in the subsequent slice(s)).
                for ( ; t+1 < nvals && I_work [t+1] < 0 ; t++)
                { 
                    // assemble the duplicate tuple
                    int64_t k = (K_work == NULL) ? (t+1) : K_work [t+1] ;
                    // Tx [my_tnz] += S [k] with typecast
                    GB_ADD_CAST_ARRAY_TO_ARRAY (Tx, my_tnz, S, k) ;
                }
                my_tnz++ ;
            }
        }
    }
}


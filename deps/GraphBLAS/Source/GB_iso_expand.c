//------------------------------------------------------------------------------
// GB_iso_expand: expand a scalar into an entire array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

void GB_iso_expand          // expand an iso scalar into an entire array
(
    void *restrict X,       // output array to expand into
    int64_t n,              // # of entries in X
    void *restrict scalar,  // scalar to expand into X
    size_t size,            // size of the scalar and each entry of X
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // determine how many threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (n, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // copy the value into X
    //--------------------------------------------------------------------------

    int64_t p ;
    switch (size)
    {

        case GB_1BYTE : // bool, uint8, int8, and UDT of size 1
        {
            uint8_t a0 = (*((uint8_t *) scalar)) ;
            uint8_t *restrict Z = (uint8_t *) X ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < n ; p++)
            {
                Z [p] = a0 ;
            }
        }
        break ;

        case GB_2BYTE : // uint16, int16, and UDT of size 2
        {
            uint16_t a0 = (*((uint16_t *) scalar)) ;
            uint16_t *restrict Z = (uint16_t *) X ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < n ; p++)
            {
                Z [p] = a0 ;
            }
        }
        break ;

        case GB_4BYTE : // uint32, int32, float, and UDT of size 4
        {
            uint32_t a0 = (*((uint32_t *) scalar)) ;
            uint32_t *restrict Z = (uint32_t *) X ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < n ; p++)
            {
                Z [p] = a0 ;
            }
        }
        break ;

        case GB_8BYTE : // uint64, int64, double, float complex, UDT size 8
        {
            uint64_t a0 = (*((uint64_t *) scalar)) ;
            uint64_t *restrict Z = (uint64_t *) X ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < n ; p++)
            {
                Z [p] = a0 ;
            }
        }
        break ;

        case GB_16BYTE : // double complex, and UDT size 16
        {

            GB_blob16 a0 = (*((GB_blob16 *) scalar)) ;
            GB_blob16 *restrict Z = (GB_blob16 *) X ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < n ; p++)
            {
                Z [p] = a0 ;
            }

//          uint64_t *restrict a = (uint64_t *) scalar ;
//          uint64_t *restrict Z = (uint64_t *) X ;
//          #pragma omp parallel for num_threads(nthreads) schedule(static)
//          for (p = 0 ; p < n ; p++)
//          {
//              Z [2*p  ] = a [0] ;
//              Z [2*p+1] = a [1] ;
//          }

        }
        break ;

        default : // user-defined types of arbitrary size
        {
            GB_void *restrict Z = (GB_void *) X ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < n ; p++)
            {
                memcpy (Z + p*size, scalar, size) ;
            }
        }
        break ;
    }
}


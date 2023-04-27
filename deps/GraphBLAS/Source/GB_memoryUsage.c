//------------------------------------------------------------------------------
// GB_memoryUsage: # of bytes used for a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_memoryUsage     // count # allocated blocks and their sizes
(
    int64_t *nallocs,       // # of allocated memory blocks
    size_t *mem_deep,       // # of bytes in blocks owned by this matrix
    size_t *mem_shallow,    // # of bytes in blocks owned by another matrix
    const GrB_Matrix A      // matrix to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (nallocs != NULL) ;
    ASSERT (mem_deep != NULL) ;
    ASSERT (mem_shallow != NULL) ;

    //--------------------------------------------------------------------------
    // count the allocated blocks and their sizes
    //--------------------------------------------------------------------------

    // a matrix contains 0 to 10 dynamically malloc'd blocks
    (*nallocs) = 0 ;
    (*mem_deep) = 0 ;
    (*mem_shallow) = 0 ;

    if (A == NULL)
    { 
        #pragma omp flush
        return (GrB_SUCCESS) ;
    }

    GB_Pending Pending = A->Pending ;

    if (!A->static_header)
    { 
        (*nallocs)++ ;
        (*mem_deep) += A->header_size ;
    }

    if (A->p != NULL)
    { 
        if (A->p_shallow)
        {
            (*mem_shallow) += A->p_size ;
        }
        else
        {
            (*nallocs)++ ;
            (*mem_deep) += A->p_size ;
        }
    }

    if (A->h != NULL)
    { 
        if (A->h_shallow)
        {
            (*mem_shallow) += A->h_size ;
        }
        else
        {
            (*nallocs)++ ;
            (*mem_deep) += A->h_size ;
        }
    }

    if (A->b != NULL)
    { 
        if (A->b_shallow)
        {
            (*mem_shallow) += A->b_size ;
        }
        else
        {
            (*nallocs)++ ;
            (*mem_deep) += A->b_size ;
        }
    }

    if (A->i != NULL)
    { 
        if (A->i_shallow)
        {
            (*mem_shallow) += A->i_size ;
        }
        else
        {
            (*nallocs)++ ;
            (*mem_deep) += A->i_size ;
        }
    }

    if (A->x != NULL)
    { 
        if (A->x_shallow)
        {
            (*mem_shallow) += A->x_size ;
        }
        else
        {
            (*nallocs)++ ;
            (*mem_deep) += A->x_size ;
        }
    }

    if (Pending != NULL)
    { 
        (*nallocs)++ ;
        (*mem_deep) += Pending->header_size ;
    }

    if (Pending != NULL && Pending->i != NULL)
    { 
        (*nallocs)++ ;
        (*mem_deep) += Pending->i_size ;
    }

    if (Pending != NULL && Pending->j != NULL)
    { 
        (*nallocs)++ ;
        (*mem_deep) += Pending->j_size ;
    }

    if (Pending != NULL && Pending->x != NULL)
    { 
        (*nallocs)++ ;
        (*mem_deep) += Pending->x_size ;
    }

    #pragma omp flush
    return (GrB_SUCCESS) ;
}


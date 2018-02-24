//------------------------------------------------------------------------------
// GB_AxB_methods.h: definitions for GB_AxB_builtin.c and GB_Matrix_AdotB.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

#ifndef GB_AXB_METHODS_H
#define GB_AXB_METHODS_H

//------------------------------------------------------------------------------
// empty: for masked matrix multiply, C<M>=A*B
//------------------------------------------------------------------------------

// empty: return true if column j of matrix A is empty.  If not empty,
// return the first and last row index in the column, and Ap [j] and Ap [j+1]

static inline bool empty
(
    const int64_t *restrict Ap,
    const int64_t *restrict Ai,
    int64_t j,
    int64_t *ilo,
    int64_t *ihi
)
{
    int64_t pstart = Ap [j] ;
    int64_t pend   = Ap [j+1] ;
    if (pstart < pend)
    {
        // column j has at least one entry; return false and find ilo and ihi
        (*ilo) = Ai [pstart] ;
        (*ihi) = Ai [pend-1] ;
        return (false) ;
    }
    else
    {
        // column j is empty
        return (true) ;
    }
}

//------------------------------------------------------------------------------
// scatter_mask:  for masked matrix multiply, C<M>=A*B
//------------------------------------------------------------------------------

// scatter Mask(:,j) into Flag if it hasn't already been done

static inline void scatter_mask
(
    const int64_t j,                    // column to scatter
    const int64_t *restrict Maskp,      // column pointers of Mask
    const int64_t *restrict Maski,      // row indices of Mask
    const void    *restrict Maskx,      // values of Mask
    const size_t msize,                 // size of Mask entries
    GB_cast_function cast_Mask_to_bool, // cast function for Maskx
    int8_t *restrict Flag,              // array of size Mask->nrows
    bool *marked                        // true if Mask already scattered
)
{
    if (!(*marked))
    {
        for (int64_t p = Maskp [j] ; p < Maskp [j+1] ; p++)
        {
            // Mij = (bool) Mask (i,j)
            bool Mij ;
            cast_Mask_to_bool (&Mij, Maskx +(p*msize), 0) ;
            if (Mij)
            {
                // M(i,j) is true
                Flag [Maski [p]] = 1 ;
            }
        }
        (*marked) = true ;
    }
}

//------------------------------------------------------------------------------
// jinit: initializations for computing C(:,j) in AdotB
//------------------------------------------------------------------------------

static inline bool jinit        // true if there any work to do for C(:,j)
(
    // inputs, not modified:
    int64_t *restrict Cp,           // column pointers of C
    const int64_t j,                // column j to compute
    const int64_t cnz,              // number of entries in C, so far
    const int64_t *restrict Bp,     // column pointers of B
    const int64_t *restrict Bi,     // row indices of B
    const int64_t *restrict Maskp,  // column pointers of Mask
    const int64_t m,                // number of rows of C and A

    // outputs, not defined on input:
    int64_t *restrict pb_start,     // start of B(:,j)
    int64_t *restrict pb_end,       // end of B(:,j)
    int64_t *restrict bjnz,         // number of entries in B(:,j)
    int64_t *restrict ib_first,     // first row index in B(:,j)
    int64_t *restrict ib_last,      // last row index in B(:,j)
    int64_t *restrict kk1,          // first iteration counter for C(:,j)
    int64_t *restrict kk2           // last iteration counter for C(:,j)
)
{

    // log the start of column j of C
    Cp [j] = cnz ;

    // get the start and end of column B(:,j)
    (*pb_start) = Bp [j] ;
    (*pb_end) = Bp [j+1] ;
    (*bjnz) = (*pb_end) - (*pb_start) ;

    if ((*bjnz) == 0)
    {
        // B(:,j) has no entries, no work to do
        return (false) ;
    }

    // row indices of first and last entry in B(:,j)
    (*ib_first) = Bi [(*pb_start)] ;
    (*ib_last)  = Bi [(*pb_end)-1] ;

    // iterate for each possible entry in C(:,j)
    if (Maskp == NULL)
    {
        // compute all of C(:,j)
        (*kk1) = 0 ;
        (*kk2) = m ;
    }
    else
    {
        // C(i,j) can appear only if Mask(i,j)=1, so iterate over Mask(:,j)
        (*kk1) = Maskp [j] ;
        (*kk2) = Maskp [j+1] ;
    }
    
    // B(:,j) has entries; there is work to do
    return (true) ;
}

//------------------------------------------------------------------------------
// cij_init: initializations for computing C(i,j), for AdotB
//------------------------------------------------------------------------------

static inline bool cij_init     // true if work to do, false if zombie
(
    // inputs, not modified:
    const int64_t kk,                   // iteration counter
    const int64_t *restrict Maski,      // Mask row indices
    const void *restrict Maskx,         // Mask values
    const GB_cast_function cast_Mask,   // typecasting function for Mask to bool
    const size_t msize,                 // size of Mask entries

    const int64_t *restrict Ap, // column pointers of A
    const int64_t *restrict Ai, // row indices of A
    const int64_t ib_first,     // first row index in B(:,j)
    const int64_t ib_last,      // last row index in B(:,j)
    const int64_t pb_start,     // start of B(:,j)

    // outputs, not defined on input:
    int64_t *restrict i,        // row index i for computing C(i,j)
    int64_t *restrict pa,       // start of A(:,i)
    int64_t *restrict pa_end,   // end of A(:,i)
    int64_t *restrict pb,       // start of B(:,j)
    int64_t *restrict ainz      // number of entries in A(:,i)
)
{

    // get the row index of C(i,j) and the value of Mask(i,j)
    if (Maski == NULL)
    {
        (*i) = kk ;
    }
    else
    {
        bool Mij ;
        (*i) = Maski [kk] ;
        cast_Mask (&Mij, Maskx + (kk*msize), 0) ;
        if (!Mij)
        {
            // Mask(i,j) = 0, so no need to compute C(i,j)
            return (false) ;
        }
    }

    // get the start and end of column A(:,i)
    (*pa) = Ap [(*i)] ;
    (*pa_end) = Ap [(*i)+1] ;
    (*ainz) = (*pa_end) - (*pa) ;

    // quick checks that imply C(i,j) is symbolically zero
    if ((*ainz) == 0 || Ai [(*pa_end)-1] < ib_first || ib_last < Ai [(*pa)])
    {
        // no work to do
        return (false) ;
    }

    // get the start of column B(:,j)
    (*pb) = pb_start ;

    return (true) ;
}

//------------------------------------------------------------------------------
// built-in semirings
//------------------------------------------------------------------------------

#include "GB_AxB__semirings.h"

#endif

//------------------------------------------------------------------------------
// GB_mx_xsame: check if two arrays are equal (ignoring zombies)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_xsame    // true if arrays X and Y are the same (ignoring zombies)
(
    char *X,
    char *Y,
    int64_t len,    // length of X and Y
    size_t s,       // size of each entry of X and Y
    int64_t *I      // row indices (for zombies), same length as X and Y
)
{
    if (X == Y) return (true) ;
    if (X == NULL) return (false) ;
    if (Y == NULL) return (false) ;
    if (I == NULL) return (false) ;
    for (int64_t i = 0 ; i < len ; i++)
    {
        // check X [i] and Y [i], but ignore zombies
        if (I [i] >= 0 && !GB_mx_same (X+i*s, Y+i*s, s)) return (false) ;
    }
    return (true) ;
}


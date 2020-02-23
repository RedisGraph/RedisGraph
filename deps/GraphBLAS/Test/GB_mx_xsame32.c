//------------------------------------------------------------------------------
// GB_mx_xsame32: check if two FP32 arrays are equal (ignoring zombies)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_xsame32  // true if arrays X and Y are the same (ignoring zombies)
(
    float *X,
    float *Y,
    int64_t len,    // length of X and Y
    int64_t *I,     // row indices (for zombies), same length as X and Y
    float eps       // error tolerance allowed (eps > 0)
)
{
    if (X == Y) return (true) ;
    if (X == NULL) return (false) ;
    if (Y == NULL) return (false) ;
    if (I == NULL) return (false) ;
    for (int64_t i = 0 ; i < len ; i++)
    {
        // check X [i] and Y [i], but ignore zombies
        if (I [i] >= 0)
        {
            int xclass = fpclassify (X [i]) ;
            if (xclass != fpclassify (Y [i])) return (false) ;
            if (xclass == FP_ZERO)
            {
                // both are zero, which is OK
            }
            else if (xclass == FP_INFINITE)
            {
                // + or -infinity
                if (X [i] != Y [i]) return (false) ;
            }
            else if (xclass != FP_NAN)
            {
                // both are normal or subnormal, and nonzero
                float err = fabsf (X [i] - Y [i]) / fabsf (X [i]) ;
                if (err > eps) return (false) ;
            }
        }
    }
    return (true) ;
}


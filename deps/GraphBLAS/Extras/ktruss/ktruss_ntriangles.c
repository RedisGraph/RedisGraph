//------------------------------------------------------------------------------
// ktruss_ntriangles.c: count the number of triangles in a k-truss
//------------------------------------------------------------------------------

#include "ktruss_def.h"

int64_t ktruss_ntriangles (const int64_t anz, const Index *Ax)
{
    int64_t nt = 0 ;
    for (int64_t p = 0 ; p < anz ; p++)
    {
        nt += Ax [p] ;
    }
    return (nt/6) ;
}


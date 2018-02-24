//------------------------------------------------------------------------------
// gbcover_start.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is prepended to the very start of the created file cover_gb.c.  It
// defines and declars the static global array, gbcov.  The array could be
// dynamically allocated, but it's simpler just to declare it with a large
// fixed size.  If the GraphBLAS code size increases, just edit this file
// to make sure it is large enough.  Only the first part of the gbcov
// array is used, from 0 to gbcover_max-1.  If the size is exceeded the
// compiler will provide a helpful warning.

#include "GB.h"

#define GBCOVER_MAX 80000
int64_t gbcov [GBCOVER_MAX] ;

extern int gbcover_max ;


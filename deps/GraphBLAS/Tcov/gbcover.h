//------------------------------------------------------------------------------
// gbcover.h: include file for statement coverage testing
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbcover_max is the largest __COUNTER__ value computed by the C preprocessor
extern int gbcover_max ;

// gbcov contains the statment counters; see gbcover_start.c
extern int64_t gbcov [ ] ;

// gbcover_get copies GraphBLAS_gbcov from the MATLAB global workspace into
// the internal gbcov array.  The MALTAB array is created if it doesn't exist.
// Thus, to clear the counts simple clear GraphBLAS_gbcov from the MATLAB
// global workpace.
void gbcover_get ( ) ;

// gbcover_put copies the internal gbcov array back into the MATLAB
// GraphBLAS_gbcov array, for analysis and for subsequent statement counting.
// This way, multiple tests in MATLAB can be accumulated into a single array
// of counters.
void gbcover_put ( ) ;


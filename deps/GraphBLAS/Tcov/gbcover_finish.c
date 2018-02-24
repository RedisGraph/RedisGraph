//------------------------------------------------------------------------------
// gbcover_finish.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is appended to the very end of the created file cover_gb.c.
// Its sole purpose is to log the final value of the C preprocessor variable,
// __COUNTER__, which determines the size of the GraphBLAS_gbcov array.

int gbcover_max = __COUNTER__ ;


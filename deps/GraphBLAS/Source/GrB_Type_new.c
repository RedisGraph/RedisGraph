//------------------------------------------------------------------------------
// GrB_Type_new: create a new user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_Type_new is implemented both as a macro and a function.  Both are
// user-callable.  The default is to use the macro, since this allows the name
// of the type to be saved as a string, for subsequent error reporting by
// GrB_error.  It is also provided as a function so that applications that
// require a function instead of macro can access it.  User code can simply do
// #undef GrB_Type_new before using the function.  This approach also places
// the function GrB_Type_new in the linkable SuiteSparse:GraphBLAS library so
// that it is visible for linking with applications in languages other than
// ANSI C.  The function version does not allow the name of the ctype to be
// saved in the new GraphBLAS type, however.  It is given the generic name.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GrB_Type_new

GrB_Info GrB_Type_new           // create a new GraphBLAS type
(
    GrB_Type *type,             // handle of user type to create
    size_t sizeof_ctype         // size = sizeof (ctype) of the C type
)
{ 
    return (GB_Type_new (type, sizeof_ctype, NULL)) ;
}


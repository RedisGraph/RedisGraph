//------------------------------------------------------------------------------
// GB_mx_alias:  return an aliased argument
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

GrB_Matrix GB_mx_alias      // output matrix (NULL if no match found)
(
    char *arg_name,         // name of the output matrix
    const mxArray *arg,     // string to select the alias
    char *arg1_name,        // name of first possible alias
    GrB_Matrix arg1,        // first possible alias
    char *arg2_name,        // name of 2nd possible alias
    GrB_Matrix arg2         // second possible alias
)
{

    // get the string from the MATLAB field
    #define LEN 256
    char s [LEN] ;
    mxGetString (arg, s, LEN) ;
    if (MATCH (s, arg1_name))
    {
        // printf ("alias %s == %s\n", arg_name, arg1_name) ;
        return (arg1) ;
    }
    else if (MATCH (s, arg2_name))
    {
        // printf ("alias %s == %s\n", arg_name, arg2_name) ;
        return (arg2) ;
    }

    // no alias found
    return (NULL) ;
}


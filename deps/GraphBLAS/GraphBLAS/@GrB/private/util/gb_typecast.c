//------------------------------------------------------------------------------
// gb_typecast: typecast a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Matrix gb_typecast      // A = (type) S, where A is deep
(
    GrB_Type type,              // if NULL, copy but do not typecast
    GxB_Format_Value fmt,       // also convert to the requested format
    GrB_Matrix S                // may be shallow
)
{

    GrB_Matrix A ;

    if (type == NULL)
    { 

        //----------------------------------------------------------------------
        // make a deep copy of the input
        //----------------------------------------------------------------------

        OK (GrB_Matrix_dup (&A, S)) ;
        OK (GxB_Matrix_Option_set (A, GxB_FORMAT, fmt)) ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // typecast the input to the requested type and format
        //----------------------------------------------------------------------

        GrB_Index nrows, ncols ;
        OK (GrB_Matrix_nrows (&nrows, S)) ;
        OK (GrB_Matrix_ncols (&ncols, S)) ;
        OK (GrB_Matrix_new (&A, type, nrows, ncols)) ;
        OK (GxB_Matrix_Option_set (A, GxB_FORMAT, fmt)) ;

        // create a descriptor with d.trans = transpose
        GrB_Descriptor d ;
        OK (GrB_Descriptor_new (&d)) ;
        OK (GrB_Descriptor_set (d, GrB_INP0, GrB_TRAN)) ;

        // A = (type) S
        OK (GrB_transpose (A, NULL, NULL, S, d)) ;
        OK (GrB_Descriptor_free (&d)) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (A) ;
}


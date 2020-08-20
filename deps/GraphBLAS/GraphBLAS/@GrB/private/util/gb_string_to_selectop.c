//------------------------------------------------------------------------------
// gb_string_to_selectop: get a GraphBLAS select operator from a string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

//  select operators, with their equivalent aliases

GxB_SelectOp gb_string_to_selectop      // return select operator from a string
(
    char *opstring                      // string defining the operator
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (opstring == NULL || opstring [0] == '\0', "invalid selectop") ;

    //--------------------------------------------------------------------------
    // convert the string to a GraphBLAS select operator
    //--------------------------------------------------------------------------

    if (MATCH (opstring, "tril"))
    { 
        return (GxB_TRIL) ;
    }
    else if (MATCH (opstring, "triu"))
    { 
        return (GxB_TRIU) ;
    }
    else if (MATCH (opstring, "diag"))
    { 
        return (GxB_DIAG) ;
    }
    else if (MATCH (opstring, "offdiag"))
    { 
        return (GxB_OFFDIAG) ;
    }
    else if (MATCH (opstring, "nonzero") || MATCH (opstring, "~=0"))
    { 
        return (GxB_NONZERO) ;
    }
    else if (MATCH (opstring, "zero") || MATCH (opstring, "==0"))
    { 
        return (GxB_EQ_ZERO) ;
    }
    else if (MATCH (opstring, "positive") || MATCH (opstring, ">0"))
    { 
        return (GxB_GT_ZERO) ;
    }
    else if (MATCH (opstring, "nonnegative") || MATCH (opstring, ">=0"))
    { 
        return (GxB_GE_ZERO) ;
    }
    else if (MATCH (opstring, "negative") || MATCH (opstring, "<0"))
    { 
        return (GxB_LT_ZERO) ;
    }
    else if (MATCH (opstring, "nonpositive") || MATCH (opstring, "<=0"))
    { 
        return (GxB_LE_ZERO) ;
    }
    else if (MATCH (opstring, "~="))
    { 
        return (GxB_NE_THUNK) ;
    }
    else if (MATCH (opstring, "=="))
    { 
        return (GxB_EQ_THUNK) ;
    }
    else if (MATCH (opstring, ">"))
    { 
        return (GxB_GT_THUNK) ;
    }
    else if (MATCH (opstring, ">="))
    { 
        return (GxB_GE_THUNK) ;
    }
    else if (MATCH (opstring, "<"))
    { 
        return (GxB_LT_THUNK) ;
    }
    else if (MATCH (opstring, "<="))
    { 
        return (GxB_LE_THUNK) ;
    }

    ERROR2 ("selectop unknown: %s\n", opstring) ;
    return (NULL) ;
}


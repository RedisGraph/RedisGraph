//------------------------------------------------------------------------------
// GB_transplant_conform: transplant T into C, then conform C
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = (type) T, then conform C to its desired hypersparsity.  T is freed

#include "GB.h"

GrB_Info GB_transplant_conform      // transplant and conform hypersparsity
(
    GrB_Matrix C,                   // destination matrix to transplant into
    GrB_Type ctype,                 // type to cast into
    GrB_Matrix *Thandle,            // source matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;
    ASSERT (Thandle != NULL) ;
    ASSERT_OK (GB_check (*Thandle, "T to transplant into C", GB0)) ;
    ASSERT_OK (GB_check (ctype, "ctype for transplant into C", GB0)) ;

    //--------------------------------------------------------------------------
    // transplant and typecast T into C, and free T
    //--------------------------------------------------------------------------

    GrB_Info info = GB_transplant (C, ctype, Thandle, Context) ;

    // T is always freed, even if the transplant runs out of memory
    ASSERT (*Thandle == NULL) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    ASSERT_OK (GB_check (C, "C transplanted", GB0)) ;

    //--------------------------------------------------------------------------
    // conform C to its desired hypersparsity
    //--------------------------------------------------------------------------

    return (GB_to_hyper_conform (C, Context)) ;
}


//------------------------------------------------------------------------------
// gb_mxcell_to_index: convert cell array to index list I or colon expression
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Get a list of indices from a MATLAB cell array.

// I is a cell array.  I contains 0, 1, 2, or 3 items:
//
//      0:   { }    This is the MATLAB ':', like C(:,J), refering to all m rows,
//                  if C is m-by-n.
//      1:   { list }  A 1D list of row indices, like C(I,J) in MATLAB.
//      2:  { start,fini }  start and fini are scalars (either double, int64,
//                  or uint64).  This defines I = start:fini in MATLAB colon
//                  notation.
//      3:  { start,inc,fini } start, inc, and fini are scalars (double, int64,
//                  or uint64).  This defines I = start:inc:fini in MATLAB
//                  notation.

#include "gb_matlab.h"

GrB_Index *gb_mxcell_to_index   // return index list I
(
    const mxArray *I_cell,      // MATLAB cell array
    base_enum_t base,           // I is one-based or zero-based
    const GrB_Index n,          // dimension of matrix being indexed
    bool *I_allocated,          // true if output array I is allocated
    GrB_Index *ni               // length (I)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (I_cell == NULL || !mxIsCell (I_cell), "internal error 6") ;

    //--------------------------------------------------------------------------
    // get the contents of I_cell
    //--------------------------------------------------------------------------

    int len = mxGetNumberOfElements (I_cell) ;
    CHECK_ERROR (len > 3, "index must be a cell array of length 0 to 3") ;

    bool Item_allocated [3] = { false, false, false } ;
    int64_t Item_len [3] = { 0, 0, 0 } ;
    int64_t Item_max [3] = { -1, -1, -1 } ;
    GrB_Index *Item [3] = { NULL, NULL, NULL } ;

    for (int k = 0 ; k < len ; k++)
    { 
        // convert I_cell {k} content to an integer list
        Item [k] = gb_mxarray_to_list (mxGetCell (I_cell, k), base,
            &Item_allocated [k], &Item_len [k], &Item_max [k]) ;
    }

    //--------------------------------------------------------------------------
    // parse the lists in the cell array
    //--------------------------------------------------------------------------

    GrB_Index *I ;

    if (len == 0)
    { 

        //----------------------------------------------------------------------
        // I = { }
        //----------------------------------------------------------------------

        (*ni) = n ;
        (*I_allocated) = false ;
        I = (GrB_Index *) GrB_ALL ;

    }
    else if (len == 1)
    { 

        //----------------------------------------------------------------------
        // I = { list }
        //----------------------------------------------------------------------

        (*ni) = Item_len [0] ;
        (*I_allocated) = Item_allocated [0] ;
        I = (GrB_Index *) (Item [0]) ;

    }
    else if (len == 2)
    { 

        //----------------------------------------------------------------------
        // I = { start, fini }, defining start:fini
        //----------------------------------------------------------------------

        CHECK_ERROR (Item_len [0] != 1 || Item_len [1] != 1,
            "start and fini must be scalars for start:fini") ;

        I = mxCalloc (3, sizeof (GrB_Index)) ;
        (*I_allocated) = true ;

        I [GxB_BEGIN] = Item [0][0] ;
        I [GxB_END  ] = Item [1][0] ;

        if (Item_allocated [0]) gb_mxfree (& (Item [0])) ;
        if (Item_allocated [1]) gb_mxfree (& (Item [1])) ;

        (*ni) = GxB_RANGE ;

    }
    else // if (len == 3)
    {

        //----------------------------------------------------------------------
        // I = { start, inc, fini }, defining start:inc:fini
        //----------------------------------------------------------------------

        CHECK_ERROR (Item_len [0] != 1 || Item_len [1] != 1 ||
            Item_len [2] != 1,
            "start, inc, and fini must be scalars for start:inc:fini") ;

        I = mxCalloc (3, sizeof (GrB_Index)) ;
        (*I_allocated) = true ;

        I [GxB_BEGIN] = Item [0][0] ;
        I [GxB_END  ] = Item [2][0] ;
        int64_t inc = Item [1][0] ;

        if (Item_allocated [1])
        { 
            // the 2nd item in the list is inc, and if it was passed in as
            // 1-based, it has been decremented.  So increment it to get back
            // to the correct value.
            inc++ ;
        }

        if (Item_allocated [0]) gb_mxfree (& (Item [0])) ;
        if (Item_allocated [1]) gb_mxfree (& (Item [1])) ;
        if (Item_allocated [2]) gb_mxfree (& (Item [2])) ;

        if (inc < 0)
        { 
            I [GxB_INC] = (GrB_Index) (-inc) ;
            (*ni) = GxB_BACKWARDS ;
        }
        else
        { 
            I [GxB_INC] = (GrB_Index) (inc) ;
            (*ni) = GxB_STRIDE ;
        }
    }

    return (I) ;
}


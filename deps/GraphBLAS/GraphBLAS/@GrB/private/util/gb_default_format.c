//------------------------------------------------------------------------------
// gb_default_format: determine the default format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

#include "gb_interface.h"

GxB_Format_Value gb_default_format      // GxB_BY_ROW or GxB_BY_COL
(
    GrB_Index nrows,        // row vectors are stored by row
    GrB_Index ncols         // column vectors are stored by column
)
{

    GxB_Format_Value fmt ;
    if (ncols == 1)
    { 
        // column vectors are stored by column, by default
        fmt = GxB_BY_COL ;
    }
    else if (nrows == 1)
    { 
        // row vectors are stored by row, by default
        fmt = GxB_BY_ROW ;
    }
    else
    { 
        // get the default format
        OK (GxB_Global_Option_get (GxB_FORMAT, &fmt)) ;
    }
    return (fmt) ;
}


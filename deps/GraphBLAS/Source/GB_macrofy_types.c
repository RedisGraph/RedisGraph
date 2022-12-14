//------------------------------------------------------------------------------
// GB_macrofy_types: construct typedefs for up to 6 types
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_types
(
    FILE *fp,
    // input:
    const char *ctype_defn,
    const char *atype_defn,
    const char *btype_defn,
    const char *xtype_defn,
    const char *ytype_defn,
    const char *ztype_defn
)
{

    // need to check if any typedefs are repeated
    const char *defn [6] ;
    defn [0] = ctype_defn ;
    defn [1] = atype_defn ;
    defn [2] = btype_defn ;
    defn [3] = xtype_defn ;
    defn [4] = ytype_defn ;
    defn [5] = ztype_defn ;

    for (int k = 0 ; k <= 5 ; k++)
    {
        if (defn [k] != NULL)
        {
            // only print this typedef it is unique
            bool is_unique = true ;
            for (int j = 0 ; j < k && is_unique ; j++)
            {
                if (defn [j] != NULL && strcmp (defn [j], defn [k]) == 0)
                {
                    is_unique = false ;
                }
            }
            if (is_unique)
            {
                // the typedef is unique: include it in the .h file
                fprintf (fp, "%s\n\n", defn [k]) ;
            }
        }
    }
}


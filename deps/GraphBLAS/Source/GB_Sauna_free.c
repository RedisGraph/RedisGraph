//------------------------------------------------------------------------------
// GB_Sauna_free: free a Sauna
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_Sauna_free                  // free a Sauna
(
    GB_Sauna *Sauna_Handle          // handle of Sauna to free
)
{
    if (Sauna_Handle != NULL)
    {
        GB_Sauna Sauna = *Sauna_Handle ;
        if (Sauna != NULL)
        { 
            // free all content of the Sauna
            size_t n = Sauna->Sauna_n ;
            GB_FREE_MEMORY (Sauna->Sauna_Mark, n+1, sizeof (int64_t)) ;
            GB_FREE_MEMORY (Sauna->Sauna_Work, n+1, Sauna->Sauna_size) ;
            // free the header of the Sauna itself
            GB_FREE_MEMORY (*Sauna_Handle, 1, sizeof (struct GB_Sauna_opaque)) ;
        }
        (*Sauna_Handle) = NULL ;
    }
}


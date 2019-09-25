//------------------------------------------------------------------------------
// GB_Sauna_free: free a Sauna
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_Sauna.h"

void GB_Sauna_free                  // free a Sauna
(
    int Sauna_id                    // id of Sauna to free
)
{

    GB_Sauna Sauna = GB_Global_Saunas_get (Sauna_id) ;
    if (Sauna != NULL)
    { 
        // free all content of the Sauna
        size_t n = Sauna->Sauna_n ;
        GB_FREE_MEMORY (Sauna->Sauna_Mark, n+1, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Sauna->Sauna_Work, n+1, Sauna->Sauna_size) ;
        // free the header of the Sauna itself
        GB_FREE_MEMORY (Sauna, 1, sizeof (struct GB_Sauna_struct)) ;
        GB_Global_Saunas_set (Sauna_id, NULL) ;
    }
}


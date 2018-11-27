//------------------------------------------------------------------------------
// GB_Sauna_alloc: create a new Sauna
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Sauna_alloc             // create a Sauna
(
    GB_Sauna *Sauna_Handle,         // handle of Sauna to create
    int64_t Sauna_n,                // size of the Sauna
    size_t Sauna_size,              // size of each entry in the Sauna
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Sauna_Handle != NULL) ;
    (*Sauna_Handle) = NULL ;

    //--------------------------------------------------------------------------
    // allocate the Sauna header
    //--------------------------------------------------------------------------

    GB_CALLOC_MEMORY (*Sauna_Handle, 1, sizeof (struct GB_Sauna_opaque)) ;
    if (*Sauna_Handle == NULL)
    { 
        return (GB_NO_MEMORY) ;
    }

    GB_Sauna Sauna = *Sauna_Handle ;

    //--------------------------------------------------------------------------
    // allocate the contents of the Sauna
    //--------------------------------------------------------------------------

    Sauna_n = GB_IMAX (Sauna_n, 1) ;    // must have at least one entry
    Sauna->Sauna_hiwater = 1 ;          // Sauna_Mark [0..n-1] < hiwater
    Sauna->Sauna_n = Sauna_n ;
    Sauna->Sauna_size = Sauna_size ;

    double memory = GBYTES (Sauna_n, sizeof (int64_t)) ;
    GB_CALLOC_MEMORY (Sauna->Sauna_Mark, Sauna_n+1, sizeof (int64_t)) ;
    bool ok = (Sauna->Sauna_Mark != NULL) ;

    if (ok && Sauna_size > 0)
    { 
        // Sauna_Work is not allocated if Sauna_size is zero
        memory += GBYTES (Sauna_n, sizeof (int64_t)) ;
        GB_MALLOC_MEMORY (Sauna->Sauna_Work, Sauna_n+1, Sauna_size) ;
        ok = ok && (Sauna->Sauna_Work != NULL) ;
    }

    if (!ok)
    {
        // out of memory
        GB_Sauna_free (Sauna_Handle) ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GB_Sauna_alloc: create a new Sauna
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Does not use error reporting; returns GrB_SUCCESS or GrB_OUT_OF_MEMORY.

#include "GB_Sauna.h"

GrB_Info GB_Sauna_alloc             // create a Sauna
(
    int Sauna_id,                   // id of Sauna to create
    int64_t Sauna_n,                // size of the Sauna
    size_t Sauna_size               // size of each entry in the Sauna
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Sauna_id >= 0 && Sauna_id < GxB_NTHREADS_MAX) ;

    //--------------------------------------------------------------------------
    // allocate the Sauna header
    //--------------------------------------------------------------------------

    GB_Sauna Sauna ;
    GB_CALLOC_MEMORY (Sauna, 1, sizeof (struct GB_Sauna_struct)) ;
    if (Sauna == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // save it in the global table
    GB_Global_Saunas_set (Sauna_id, Sauna) ;

    //--------------------------------------------------------------------------
    // allocate the contents of the Sauna
    //--------------------------------------------------------------------------

    Sauna_n    = GB_IMAX (Sauna_n, 1) ;     // must have at least one entry
    Sauna_size = GB_IMAX (Sauna_size, 1) ;  // each entry must have size >= 1
    Sauna->Sauna_hiwater = 1 ;              // Sauna_Mark [0..n-1] < hiwater
    Sauna->Sauna_n = Sauna_n ;
    Sauna->Sauna_size = Sauna_size ;

    // note that Sauna_Work does not need to be initialized
    GB_CALLOC_MEMORY (Sauna->Sauna_Mark, Sauna_n+1, sizeof (int64_t)) ;
    GB_MALLOC_MEMORY (Sauna->Sauna_Work, Sauna_n+1, Sauna_size) ;

    if (Sauna->Sauna_Mark == NULL || Sauna->Sauna_Work == NULL)
    { 
        // out of memory
        GB_Sauna_free (Sauna_id) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}


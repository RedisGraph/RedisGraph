//------------------------------------------------------------------------------
// GB_Sauna.h: definitions for the Sauna, the sparse accumulator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_SAUNA_H
#define GB_SAUNA_H
#include "GB.h"

//------------------------------------------------------------------------------
// Sauna data structure
//------------------------------------------------------------------------------

struct GB_Sauna_struct      // sparse accumulator
{
    int64_t Sauna_hiwater ; // Sauna_Mark [0..Sauna_n-1] < hiwater holds when
                            // the Sauna_Mark is clear.
    int64_t Sauna_n ;       // size of Sauna_Mark and Sauna_Work
    int64_t *Sauna_Mark ;   // array of size Sauna_n
    void    *Sauna_Work ;   // array of size Sauna_n, each entry Sauna_size
    size_t  Sauna_size ;    // size of each entry in Sauna_Work
} ;

//------------------------------------------------------------------------------
// Sauna functions
//------------------------------------------------------------------------------

GrB_Info GB_Sauna_acquire
(
    int nthreads,           // number of internal threads that need a Sauna
    int *Sauna_ids,         // size nthreads, the Sauna id's acquired
    GrB_Desc_Value *AxB_methods_used,       // size nthreads
    GB_Context Context
) ;

GrB_Info GB_Sauna_alloc             // create a Sauna
(
    int Sauna_id,                   // id of Sauna to create
    int64_t Sauna_n,                // size of the Sauna
    size_t Sauna_size               // size of each entry in the Sauna
) ;

void GB_Sauna_free                  // free a Sauna
(
    int Sauna_id                    // id of Sauna to free
) ;

GrB_Info GB_Sauna_release
(
    int nthreads,           // number of internal threads that have a Sauna
    int *Sauna_ids          // size nthreads, the Sauna id's to release
) ;

//------------------------------------------------------------------------------
// ASSERT_SAUNA_IS_RESET
//------------------------------------------------------------------------------

// assert that all entries in Sauna_Mark are < Sauna_hiwater

#ifdef GB_DEBUG
    #define ASSERT_SAUNA_IS_RESET                                           \
    {                                                                       \
        for (int64_t i = 0 ; i < Sauna->Sauna_n ; i++)                      \
        {                                                                   \
            ASSERT (Sauna->Sauna_Mark [i] < Sauna->Sauna_hiwater) ;         \
        }                                                                   \
    }
#else
    #define ASSERT_SAUNA_IS_RESET
#endif

//------------------------------------------------------------------------------
// GB_Sauna_reset: increment the Sauna_hiwater and clear Sauna_Mark if needed
//------------------------------------------------------------------------------

static inline int64_t GB_Sauna_reset
(
    GB_Sauna Sauna,     // Sauna to reset
    int64_t reset,      // does Sauna_hiwater += reset
    int64_t range       // clear Mark if Sauna_hiwater+reset+range overflows
)
{ 

    ASSERT (Sauna != NULL) ;
    Sauna->Sauna_hiwater += reset ;     // increment the Sauna_hiwater

    if (Sauna->Sauna_hiwater + range <= 0 || reset == 0)
    { 
        // integer overflow has occurred; clear all of the Sauna_Mark array
        for (int64_t i = 0 ; i < Sauna->Sauna_n ; i++)
        { 
            Sauna->Sauna_Mark [i] = 0 ;
        }
        Sauna->Sauna_hiwater = 1 ;
    }

    // assertion for debugging only:
    ASSERT_SAUNA_IS_RESET ;         // assert that Sauna_Mark [...] < hiwater

    return (Sauna->Sauna_hiwater) ;
}

#endif


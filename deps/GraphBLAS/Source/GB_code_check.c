//------------------------------------------------------------------------------
// GB_code_check: print an entry using a type code
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Only prints entries of built-in types; user-defined types can't be printed.

#include "GB.h"

GrB_Info GB_code_check          // print an entry using a type code
(
    const GB_Type_code code,    // type code of value to print
    const void *x,              // entry to print
    FILE *f,                    // file to print to
    GB_Context Context
)
{

    ASSERT (code <= GB_UDT_code) ;

    switch (code)
    {
        case GB_BOOL_code   : GBPR ("bool %d",      *((int8_t   *) x)) ; break ;
        case GB_INT8_code   : GBPR ("int8 %d",      *((int8_t   *) x)) ; break ;
        case GB_UINT8_code  : GBPR ("uint8 %u",     *((uint8_t  *) x)) ; break ;
        case GB_INT16_code  : GBPR ("int16 %d",     *((int16_t  *) x)) ; break ;
        case GB_UINT16_code : GBPR ("uint16 %u",    *((uint16_t *) x)) ; break ;
        case GB_INT32_code  : GBPR ("int32 %d",     *((int32_t  *) x)) ; break ;
        case GB_UINT32_code : GBPR ("uint32 %u",    *((uint32_t *) x)) ; break ;
        case GB_INT64_code  : GBPR ("int64 "GBd,    *((int64_t  *) x)) ; break ;
        case GB_UINT64_code : GBPR ("uint64 "GBu,   *((uint64_t *) x)) ; break ;
        case GB_FP32_code   : GBPR ("float %.6g",   *((float    *) x)) ; break ;
        case GB_FP64_code   : GBPR ("double %.15g", *((double   *) x)) ; break ;
        case GB_UCT_code    :
        case GB_UDT_code    :
            { 
                GBPR ("[user-defined value]") ;
                // GraphBLAS does not have a method for the user to register
                // a 'printf' function for a user-defined type.  This can be
                // uncommented during code development if the user-defined type
                // is double complex:
                // double *a = (double *) x ;
                // GBPR (" real %.15g imag %.15g", a [0], a [1]) ;
            }
            break ;
        default: ;
    }

    return (GrB_SUCCESS) ;
}


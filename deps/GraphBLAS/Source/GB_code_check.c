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
    int64_t i ;
    uint64_t u ;
    double d ;

    switch (code)
    {
        case GB_BOOL_code   : i = *((bool     *) x) ; GBPR ("bool "    GBd, i) ; break ;
        case GB_INT8_code   : i = *((int8_t   *) x) ; GBPR ("int8 "    GBd, i) ; break ;
        case GB_UINT8_code  : u = *((uint8_t  *) x) ; GBPR ("uint8 "   GBu, u) ; break ;
        case GB_INT16_code  : i = *((int16_t  *) x) ; GBPR ("int16 "   GBd, i) ; break ;
        case GB_UINT16_code : u = *((uint16_t *) x) ; GBPR ("uint16 "  GBu, u) ; break ;
        case GB_INT32_code  : i = *((int32_t  *) x) ; GBPR ("int32 "   GBd, i) ; break ;
        case GB_UINT32_code : u = *((uint32_t *) x) ; GBPR ("uint32 "  GBu, u) ; break ;
        case GB_INT64_code  : i = *((int64_t  *) x) ; GBPR ("int64 "   GBd, i) ; break ;
        case GB_UINT64_code : u = *((uint64_t *) x) ; GBPR ("uint64 "  GBu, u) ; break ;
        case GB_FP32_code   : d = *((float    *) x) ; GBPR ("float %.6g"  , d) ; break ;
        case GB_FP64_code   : d = *((double   *) x) ; GBPR ("double %.15g", d) ; break ;
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


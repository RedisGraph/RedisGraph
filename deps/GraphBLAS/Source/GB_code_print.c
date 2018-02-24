//------------------------------------------------------------------------------
// GB_code_print: print an entry using a type code
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Only prints entries of built-in types; user-defined types can't be printed.

#include "GB.h"

void GB_code_print              // print an entry using a type code
(
    const GB_Type_code code,    // type code of value to print
    const void *x               // entry to print
)
{

    ASSERT (code <= GB_UDT_code) ;

    switch (code)
    {
        case GB_BOOL_code  : printf ((*((bool *) x)) ? "true" : "false"); break;
        case GB_INT8_code  : printf ("int8 %d",      *((int8_t   *) x)) ; break;
        case GB_UINT8_code : printf ("uint8 %u",     *((uint8_t  *) x)) ; break;
        case GB_INT16_code : printf ("int16 %d",     *((int16_t  *) x)) ; break;
        case GB_UINT16_code: printf ("uint16 %u",    *((uint16_t *) x)) ; break;
        case GB_INT32_code : printf ("int32 %d",     *((int32_t  *) x)) ; break;
        case GB_UINT32_code: printf ("uint32 %u",    *((uint32_t *) x)) ; break;
        case GB_INT64_code : printf ("int64 "GBd,    *((int64_t  *) x)) ; break;
        case GB_UINT64_code: printf ("uint64 "GBu,   *((uint64_t *) x)) ; break;
        case GB_FP32_code  : printf ("float %.6g",   *((float    *) x)) ; break;
        case GB_FP64_code  : printf ("double %.15g", *((double   *) x)) ; break;
        case GB_UDT_code   :
            {
                printf ("[user-defined value]") ;
                // GraphBLAS does not have a method for the user to register
                // a 'printf' function for a user-defined type.  This can be
                // uncommented during code development if the user-defined type
                // is double complex:
                // double *a = (double *) x ;
                // printf (" real %.15g imag %.15g", a [0], a [1]) ;
            }
            break ;
        default: ;
    }
}


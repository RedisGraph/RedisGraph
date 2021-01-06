//------------------------------------------------------------------------------
// GB_code_check: print an entry using a type code
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Only prints entries of built-in types; user-defined types can't be printed.

#include "GB.h"

#define GB_PRINT_INF(x) GBPR ((x < 0) ? "-Inf" : "Inf")

#define GB_PRINT_FLOAT(s)                                           \
{                                                                   \
    switch (fpclassify (s))                                         \
    {                                                               \
        case FP_NAN:      GBPR ("NaN") ; break ;                    \
        case FP_INFINITE: GB_PRINT_INF (s) ; break ;                \
        case FP_ZERO:     GBPR ("0") ; break ;                      \
        default:          GBPR ("%.6g", (double) s) ;               \
    }                                                               \
}

#define GB_PRINT_DOUBLE(d,pr_verbose)                               \
{                                                                   \
    switch (fpclassify (d))                                         \
    {                                                               \
        case FP_NAN:      GBPR ("NaN") ; break ;                    \
        case FP_INFINITE: GB_PRINT_INF (d) ; break ;                \
        case FP_ZERO:     GBPR ("0") ; break ;                      \
        default:                                                    \
            if (pr_verbose)                                         \
            {                                                       \
                /* long format */                                   \
                GBPR ("%.15g", d) ;                                 \
            }                                                       \
            else                                                    \
            {                                                       \
                /* short format */                                  \
                GBPR ("%.6g", d) ;                                  \
            }                                                       \
            break ;                                                 \
    }                                                               \
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_code_check          // print an entry using a type code
(
    const GB_Type_code code,    // type code of value to print
    const void *x,              // entry to print
    int pr,                     // print level
    FILE *f                     // file to print to
)
{

    ASSERT (code <= GB_UDT_code) ;
    int64_t i ;
    uint64_t u ;
    double d ;
    float s ;
    GxB_FC32_t c ;
    GxB_FC64_t z ;
    bool pr_verbose = (pr == GxB_SHORT_VERBOSE || pr == GxB_COMPLETE_VERBOSE) ;

    switch (code)
    {
        #if GB_DEVELOPER

        case GB_BOOL_code:   i = *((bool     *) x) ; GBPR ("bool "    GBd, i) ;
            break ;
        case GB_INT8_code:   i = *((int8_t   *) x) ; GBPR ("int8 "    GBd, i) ;
            break ;
        case GB_UINT8_code:  u = *((uint8_t  *) x) ; GBPR ("uint8 "   GBu, u) ;
            break ;
        case GB_INT16_code:  i = *((int16_t  *) x) ; GBPR ("int16 "   GBd, i) ;
            break ;
        case GB_UINT16_code: u = *((uint16_t *) x) ; GBPR ("uint16 "  GBu, u) ;
            break ;
        case GB_INT32_code:  i = *((int32_t  *) x) ; GBPR ("int32 "   GBd, i) ;
            break ;
        case GB_UINT32_code: u = *((uint32_t *) x) ; GBPR ("uint32 "  GBu, u) ;
            break ;
        case GB_INT64_code:  i = *((int64_t  *) x) ; GBPR ("int64 "   GBd, i) ;
            break ;
        case GB_UINT64_code: u = *((uint64_t *) x) ; GBPR ("uint64 "  GBu, u) ;
            break ;

        case GB_FP32_code:
            s = *((float *) x) ;
            GBPR ("float %.6g", (double) s) ;
            break ;

        case GB_FP64_code:
            d = *((double *) x) ;
            GBPR ("double %.15g", d) ;
            break ;

        case GB_FC32_code:
            c = *((GxB_FC32_t *) x) ;
            GBPR ("float complex (%.6g, %6.g)",
                (double) crealf (c), (double) cimagf (c)) ;
            break ;

        case GB_FC64_code:
            z = *((GxB_FC64_t *) x) ;
            GBPR ("double complex (%.15g, %.15g)", creal (z), cimag (z)) ;
            break ;

        #else

        case GB_BOOL_code   : i = *((bool     *) x) ; GBPR ("  " GBd, i) ;
            break ;
        case GB_INT8_code   : i = *((int8_t   *) x) ; GBPR ("  " GBd, i) ;
            break ;
        case GB_UINT8_code  : u = *((uint8_t  *) x) ; GBPR ("  " GBu, u) ;
            break ;
        case GB_INT16_code  : i = *((int16_t  *) x) ; GBPR ("  " GBd, i) ;
            break ;
        case GB_UINT16_code : u = *((uint16_t *) x) ; GBPR ("  " GBu, u) ;
            break ;
        case GB_INT32_code  : i = *((int32_t  *) x) ; GBPR ("  " GBd, i) ;
            break ;
        case GB_UINT32_code : u = *((uint32_t *) x) ; GBPR ("  " GBu, u) ;
            break ;
        case GB_INT64_code  : i = *((int64_t  *) x) ; GBPR ("  " GBd, i) ;
            break ;
        case GB_UINT64_code : u = *((uint64_t *) x) ; GBPR ("  " GBu, u) ;
            break ;

        case GB_FP32_code   : 
            s = *((float *) x) ;
            GBPR ("   ") ;
            GB_PRINT_FLOAT (s) ;
            break ;

        case GB_FP64_code   : 
            d = *((double *) x) ;
            GBPR ("   ") ;
            GB_PRINT_DOUBLE (d, pr_verbose) ;
            break ;

        case GB_FC32_code   : 
            c = *((GxB_FC32_t *) x) ;
            GBPR ("   ") ;
            GB_PRINT_FLOAT (crealf (c)) ;
            s = cimagf (c) ;
            if (s < 0)
            {
                GBPR (" - ") ;
                GB_PRINT_FLOAT (-s) ;
            }
            else
            {
                GBPR (" + ") ;
                GB_PRINT_FLOAT (s) ;
            }
            GBPR ("i") ;
            break ;

        case GB_FC64_code   : 
            z = *((GxB_FC64_t *) x) ;
            GBPR ("   ") ;
            GB_PRINT_DOUBLE (creal (z), pr_verbose) ;
            d = cimag (z) ;
            if (d < 0)
            {
                GBPR (" - ") ;
                GB_PRINT_DOUBLE (-d, pr_verbose) ;
            }
            else
            {
                GBPR (" + ") ;
                GB_PRINT_DOUBLE (d, pr_verbose) ;
            }
            GBPR ("i") ;
            break ;

        #endif

        case GB_UDT_code    : 
            { 
                GBPR ("[user-defined value]") ;
                // FUTURE: GraphBLAS does not have a method for the user to
                // register a print function for a user-defined type.
            }
            break ;
        default: ;
    }

    return (GrB_SUCCESS) ;
}


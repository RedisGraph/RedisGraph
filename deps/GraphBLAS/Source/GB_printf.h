//------------------------------------------------------------------------------
// GB_printf.h: definitions for printing by GraphBLAS check functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GxB_*_fprintf with f == NULL prints nothing by default.  However, if
// GB_printf_function has been set by the caller, then that function is used
// when f is NULL.

#ifndef GB_PRINTF_H
#define GB_PRINTF_H

#include "GB.h"

GB_PUBLIC int (* GB_printf_function ) (const char *format, ...) ;

// print to a file f, and check the result
#define GBPR(...)                                                           \
{                                                                           \
    int printf_result = 0 ;                                                 \
    if (f == NULL && GB_printf_function != NULL)                            \
    {                                                                       \
        printf_result = GB_printf_function (__VA_ARGS__) ;                  \
    }                                                                       \
    else if (f != NULL)                                                     \
    {                                                                       \
        printf_result =  fprintf (f, __VA_ARGS__)  ;                        \
    }                                                                       \
    if (printf_result < 0)                                                  \
    {                                                                       \
        int err = errno ;                                                   \
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,                       \
            "File output error (%d): %s", err, strerror (err)))) ;          \
    }                                                                       \
}

#define GBPR0(...)                  \
{                                   \
    if (pr > 0)                     \
    {                               \
        GBPR (__VA_ARGS__) ;        \
    }                               \
}
#endif

// check object->magic and print an error if invalid
#define GB_CHECK_MAGIC(object,kind)                                     \
{                                                                       \
    switch (object->magic)                                              \
    {                                                                   \
        case GB_MAGIC :                                                 \
            /* the object is valid */                                   \
            break ;                                                     \
                                                                        \
        case GB_FREED :                                                 \
            /* dangling pointer! */                                     \
            GBPR0 ("already freed!\n") ;                                \
            return (GB_ERROR (GrB_UNINITIALIZED_OBJECT, (GB_LOG,        \
                "%s is freed: [%s]", kind, name))) ;                    \
                                                                        \
        case GB_MAGIC2 :                                                \
            /* invalid */                                               \
            GBPR0 ("invalid\n") ;                                       \
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,              \
                "%s is invalid: [%s]", kind, name))) ;                  \
                                                                        \
        default :                                                       \
            /* uninitialized */                                         \
            GBPR0 ("uninititialized\n") ;                               \
            return (GB_ERROR (GrB_UNINITIALIZED_OBJECT, (GB_LOG,        \
                "%s is uninitialized: [%s]", kind, name))) ;            \
    }                                                                   \
}


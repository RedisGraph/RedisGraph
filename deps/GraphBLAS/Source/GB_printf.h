//------------------------------------------------------------------------------
// GB_printf.h: definitions for printing from GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_PRINTF_H
#define GB_PRINTF_H

#define GB_STRING_MATCH(s,t) (strcmp (s,t) == 0)

//------------------------------------------------------------------------------
// printing control
//------------------------------------------------------------------------------

// format strings, normally %llu and %lld, for GrB_Index values
#define GBu "%" PRIu64
#define GBd "%" PRId64

// print to the standard output, and flush the result.  No error check is done.
// This function is used for the BURBLE, and for debugging output. 
#define GBDUMP(...)                                                     \
{                                                                       \
    GB_printf_function_t printf_func = GB_Global_printf_get ( ) ;       \
    if (printf_func != NULL)                                            \
    {                                                                   \
        printf_func (__VA_ARGS__) ;                                     \
    }                                                                   \
    else                                                                \
    {                                                                   \
        printf (__VA_ARGS__) ;                                          \
    }                                                                   \
    GB_flush_function_t flush_func = GB_Global_flush_get ( ) ;          \
    if (flush_func != NULL)                                             \
    {                                                                   \
        flush_func ( ) ;                                                \
    }                                                                   \
    else                                                                \
    {                                                                   \
        fflush (stdout) ;                                               \
    }                                                                   \
}

// print to a file f, or to stdout if f is NULL, and check the result.  This
// macro is used by all user-callable GxB_*print and GB_*check functions.
#define GBPR(...)                                                           \
{                                                                           \
    int printf_result = 0 ;                                                 \
    if (f == NULL)                                                          \
    {                                                                       \
        GB_printf_function_t printf_func = GB_Global_printf_get ( ) ;       \
        if (printf_func != NULL)                                            \
        {                                                                   \
            printf_result = printf_func (__VA_ARGS__) ;                     \
        }                                                                   \
        else                                                                \
        {                                                                   \
            printf_result = printf (__VA_ARGS__) ;                          \
        }                                                                   \
        GB_flush_function_t flush_func = GB_Global_flush_get ( ) ;          \
        if (flush_func != NULL)                                             \
        {                                                                   \
            flush_func ( ) ;                                                \
        }                                                                   \
        else                                                                \
        {                                                                   \
            fflush (stdout) ;                                               \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        printf_result = fprintf (f, __VA_ARGS__)  ;                         \
        fflush (f) ;                                                        \
    }                                                                       \
    if (printf_result < 0)                                                  \
    {                                                                       \
        return (GrB_INVALID_VALUE) ;                                        \
    }                                                                       \
}

// print if the print level is greater than zero
#define GBPR0(...)                  \
{                                   \
    if (pr != GxB_SILENT)           \
    {                               \
        GBPR (__VA_ARGS__) ;        \
    }                               \
}

// check object->magic and print an error if invalid
#define GB_CHECK_MAGIC(object)                                          \
{                                                                       \
    switch (object->magic)                                              \
    {                                                                   \
        case GB_MAGIC :                                                 \
            /* the object is valid */                                   \
            break ;                                                     \
                                                                        \
        case GB_FREED :                                                 \
            /* dangling pointer! */                                     \
            GBPR0 (" object already freed!\n") ;                        \
            return (GrB_UNINITIALIZED_OBJECT) ;                         \
                                                                        \
        case GB_MAGIC2 :                                                \
            /* invalid */                                               \
            GBPR0 (" invalid object\n") ;                               \
            return (GrB_INVALID_OBJECT) ;                               \
                                                                        \
        default :                                                       \
            /* uninitialized */                                         \
            GBPR0 (" uninititialized object\n") ;                       \
            return (GrB_UNINITIALIZED_OBJECT) ;                         \
    }                                                                   \
}

//------------------------------------------------------------------------------
// burble
//------------------------------------------------------------------------------

// GBBURBLE and the GB_BURBLE_* methods provide diagnostic output.
// Use GxB_set (GxB_BURBLE, true) to turn it on
// and GxB_set (GxB_BURBLE, false) to turn it off.

void GB_burble_assign
(
    const bool C_replace,       // descriptor for C
    const int Ikind,
    const int Jkind,
    const GrB_Matrix M,         // mask matrix, which is not NULL here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present here
    const GrB_Matrix A,         // input matrix, not transposed
    const int assign_kind       // row assign, col assign, assign, or subassign
) ;

// define the function to use to burble
#define GBURBLE(...)                                \
{                                                   \
    if (GB_Global_burble_get ( ))                   \
    {                                               \
        GBDUMP (__VA_ARGS__) ;                      \
    }                                               \
}

// burble if a matrix is dense or full
#define GB_BURBLE_DENSE(A,format)                               \
{                                                               \
    if (GB_IS_FULL (A))                                         \
    {                                                           \
        GBURBLE (format, "full") ;                              \
    }                                                           \
    else if (GB_IS_BITMAP (A))                                  \
    {                                                           \
        GBURBLE (format, "bitmap") ;                            \
    }                                                           \
    else if (GB_is_dense (A) && !GB_PENDING_OR_ZOMBIES (A))     \
    {                                                           \
        GBURBLE (format, "dense") ;                             \
    }                                                           \
}

#if defined ( _OPENMP )

    // burble with timing
    #define GB_BURBLE_START(func)                       \
    double t_burble = 0 ;                               \
    {                                                   \
        if (GB_Global_burble_get ( ))                   \
        {                                               \
            GBURBLE (" [ " func " ") ;                  \
            t_burble = GB_OPENMP_GET_WTIME ;            \
        }                                               \
    }

    #define GB_BURBLE_END                               \
    {                                                   \
        if (GB_Global_burble_get ( ))                   \
        {                                               \
            t_burble = GB_OPENMP_GET_WTIME - t_burble ; \
            GBURBLE ("\n   %.3g sec ]\n", t_burble) ;   \
        }                                               \
    }

#else

    // burble with no timing

    #define GB_BURBLE_START(func)                       \
        GBURBLE (" [ " func " ")

    #define GB_BURBLE_END                               \
        GBURBLE ("]\n")

#endif

#define GB_BURBLE_N(n,...)                              \
{                                                       \
    if (n > 1) GBURBLE (__VA_ARGS__)                    \
}

#define GB_BURBLE_MATRIX(A, ...)                                    \
{                                                                   \
    if (!(A->vlen <= 1 && A->vdim <= 1)) GBURBLE (__VA_ARGS__)      \
}

#endif


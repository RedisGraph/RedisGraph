//------------------------------------------------------------------------------
// GB_atomics.h: definitions for atomic operations
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All atomic operations used by SuiteSparse:GraphBLAS appear in this file.

// These atomic operations assume either an ANSI C11 compiler that supports
// OpenMP 3.1 or later, or Microsoft Visual Studio on 64-bit Windows (which
// only supports OpenMP 2.0).  SuiteSparse:GraphBLAS is not supported on 32-bit
// Windows.

#ifndef GB_ATOMICS_H
#define GB_ATOMICS_H
#include "GB.h"

//------------------------------------------------------------------------------
// determine the architecture
//------------------------------------------------------------------------------

#if __x86_64__

    // on the x86, atomic updates can be more aggresive.  The MIN, MAX, EQ,
    // XNOR, and ANY monoids are implemented with atomic compare/exchange.

    #define GB_X86_64 1

#else

    // on the ARM, Power8/9, and others, only use built-in #pragma omp atomic
    // updates.  Do not use atomic compare/exchange.

    #define GB_X86_64 0

#endif

//------------------------------------------------------------------------------
// atomic updates
//------------------------------------------------------------------------------

// Whenever possible, the OpenMP pragma is used with a clause (as introduced in
// OpenMP 3.1), as follow:
//
//      #pragma omp atomic [clause]
//
// where [clause] is read, write, update, or capture.
//
// Microsoft Visual Studio only supports OpenMP 2.0, which does not have the
// [clause].  Without the [clause], #pragma omp atomic is like 
// #pragma omp atomic update, but the expression can only be one of:
//      
//      x binop= expression
//      x++
//      ++x
//      x--
//      --x
//
// where binop is one of these operators:    + * - / & ^ | << >> 
//
// OpenMP 3.0 and later support additional options for the "update" clause,
// but SuiteSparse:GraphBLAS uses only this form:
//
//      x binop= expression
//
// where binop is:  + * & ^ |
//
// This atomic update is used for the PLUS, TIMES, LAND, LXOR, and LOR monoids,
// when applied to the built-in types.  For PLUS and TIMES, these are the 10
// types INTx, UINTx, FP32, FP64 (for x = 8, 16, 32, and 64).  For the boolean
// monoids, only the BOOL type is used.
//
// As a result, the atomic updates are the same for gcc and icc (which support
// OpenMP 3.0 or later) with the "update" clause.  For MS Visual Studio, the
// "update" clause is removed since it supports OpenMP 2.0.

#if ( _OPENMP >= 201307 )

    // OpenMP 4.0 or later
    #define GB_ATOMIC_UPDATE GB_PRAGMA (omp atomic update seq_cst)

#elif ( _OPENMP >= 201107 )

    // OpenMP 3.1
    #define GB_ATOMIC_UPDATE GB_PRAGMA (omp atomic update)

#elif ( _OPENMP >= 199810 )

    // OpenMP 1.0 to 3.0: no optional clauses, "update" is assumed
    #define GB_ATOMIC_UPDATE GB_PRAGMA (omp atomic)

#else

    // no OpenMP at all
    #define GB_ATOMIC_UPDATE

#endif

//------------------------------------------------------------------------------
// atomic read and write
//------------------------------------------------------------------------------

// In Microsoft Visual Studio, simple reads and writes to properly aligned
// 64-bit values are already atomic on 64-bit Windows for any architecture
// supported by Windows (any Intel or ARM architecture). See:
// https://docs.microsoft.com/en-us/windows/win32/sync/interlocked-variable-access
// SuiteSparse:GraphBLAS is not supported on 32-bit Windows.  Thus, there
// is no need for atomic reads/writes when compiling GraphBLAS on Windows
// with MS Visual Studio.

// ARM, Power8/9, and others need the explicit atomic read/write.
// x86: no atomic read/write is needed.

#if GB_X86_64

    // x86: no atomic read/write is needed.
    #define GB_ATOMIC_READ
    #define GB_ATOMIC_WRITE

#elif ( _OPENMP >= 201811 )

    // OpenMP 5.0 or later
    #define GB_ATOMIC_READ    GB_PRAGMA (omp atomic read acquire)
    #define GB_ATOMIC_WRITE   GB_PRAGMA (omp atomic write release)

#elif ( _OPENMP >= 201307 )

    // OpenMP 4.0 and 4.5
    #define GB_ATOMIC_READ    GB_PRAGMA (omp atomic read seq_cst)
    #define GB_ATOMIC_WRITE   GB_PRAGMA (omp atomic write seq_cst)

#elif ( _OPENMP >= 201107 )

    // OpenMP 3.1
    #define GB_ATOMIC_READ    GB_PRAGMA (omp atomic read)
    #define GB_ATOMIC_WRITE   GB_PRAGMA (omp atomic write)

#else

    // OpenMP 3.0 or earlier, or no OpenMP at all
    #define GB_ATOMIC_READ
    #define GB_ATOMIC_WRITE

#endif

//------------------------------------------------------------------------------
// flush
//------------------------------------------------------------------------------

#if defined ( _OPENMP )

    // All versions of OpenMP have the #pragma omp flush
    #define GB_OMP_FLUSH GB_PRAGMA (omp flush)

#else

    // no OpenMP at all
    #define GB_OMP_FLUSH

#endif

//------------------------------------------------------------------------------
// atomic capture
//------------------------------------------------------------------------------

// An atomic capture loads the prior value of the target into a thread-local
// result, and then overwrites the target with the new value.  The target is a
// value that is shared between threads.  The value and result arguments are
// thread-local.  SuiteSparse:GraphBLAS uses four atomic capture methods,
// defined below, of the form:
//
//      { result = target ; target = value ; }      for int64_t and int8_t
//      { result = target ; target |= value ; }     for int64_t
//      { result = target++ ; }                     for int64_t
//
// OpenMP 3.1 and later supports atomic captures with a "capture" clause: 
//
//      #pragma omp atomic capture
//      { result = target ; target = value ; }
//
// or with a binary operator
//
//      #pragma omp atomic capture
//      { result = target ; target binop= value ; }
//
// MS Visual Studio supports only OpenMP 2.0, and does not support any
// "capture" clause.  Thus, on Windows, _InterlockedExchange* and
// _InterlockedOr* functions are used instead, as described here:
//
// https://docs.microsoft.com/en-us/cpp/intrinsics/interlockedexchange-intrinsic-functions
// https://docs.microsoft.com/en-us/cpp/intrinsics/interlockedor-intrinsic-functions

#if ( _OPENMP >= 201307 )

    // OpenMP 4.0 or later
    #define GB_ATOMIC_CAPTURE GB_PRAGMA (omp atomic capture seq_cst)

#elif ( _OPENMP >= 201107 )

    // OpenMP 3.1
    #define GB_ATOMIC_CAPTURE GB_PRAGMA (omp atomic capture)

#elif ( _OPENMP >= 199810 )

    // OpenMP 1.0 to 3.0: generate an intentional compile-time error if any
    // attempt is made to use the atomic capture.
    #define GB_ATOMIC_CAPTURE atomic capture not available

#else

    // no OpenMP at all
    #define GB_ATOMIC_CAPTURE

#endif

    //--------------------------------------------------------------------------
    // atomic capture for int64_t
    //--------------------------------------------------------------------------

    // int64_t result, target, value ;
    // do this atomically: { result = target ; target = value ; }

    #if GB_MICROSOFT

        #define GB_ATOMIC_CAPTURE_INT64(result, target, value)          \
        {                                                               \
            result = _InterlockedExchange64                             \
                ((int64_t volatile *) (&(target)), value) ;             \
        }

    #else

        #define GB_ATOMIC_CAPTURE_INT64(result, target, value)          \
        {                                                               \
            GB_ATOMIC_CAPTURE                                           \
            {                                                           \
                result = target ;                                       \
                target = value ;                                        \
            }                                                           \
        }

    #endif

    //--------------------------------------------------------------------------
    // atomic capture for int8_t
    //--------------------------------------------------------------------------

    // int8_t result, target, value ;
    // do this atomically: { result = target ; target = value ; }

    #if GB_MICROSOFT

        #define GB_ATOMIC_CAPTURE_INT8(result, target, value)           \
        {                                                               \
            result = _InterlockedExchange8                              \
                ((char volatile *) &(target), value) ;                  \
        }

    #else

        #define GB_ATOMIC_CAPTURE_INT8(result, target, value)           \
        {                                                               \
            GB_ATOMIC_CAPTURE                                           \
            {                                                           \
                result = target ;                                       \
                target = value ;                                        \
            }                                                           \
        }

    #endif

    //--------------------------------------------------------------------------
    // atomic capture with bitwise OR, for int64_t
    //--------------------------------------------------------------------------

    // int64_t result, target, value ;
    // do this atomically: { result = target ; target |= value ; }

    #if GB_MICROSOFT

        #define GB_ATOMIC_CAPTURE_INT64_OR(result, target, value)       \
        {                                                               \
            result = _InterlockedOr64                                   \
                ((int64_t volatile *) (&(target)), value) ;             \
        }

    #else

        #define GB_ATOMIC_CAPTURE_INT64_OR(result, target, value)       \
        {                                                               \
            GB_ATOMIC_CAPTURE                                           \
            {                                                           \
                result = target ;                                       \
                target |= value ;                                       \
            }                                                           \
        }

    #endif

    //--------------------------------------------------------------------------
    // atomic post-increment
    //--------------------------------------------------------------------------

    // Increment an int64_t value and return the value prior to being
    // incremented:
    //
    //      int64_t result = target++ ;
    //
    // See
    // https://docs.microsoft.com/en-us/cpp/intrinsics/interlockedincrement-intrinsic-functions?view=msvc-160
    // The MS Visual Studio version computes result = ++target, so result must
    // be decremented by one.

    #if GB_MICROSOFT

        #define GB_ATOMIC_CAPTURE_INC64(result,target)                  \
        {                                                               \
            result = _InterlockedIncrement64                            \
                ((int64_t volatile *) (&(target))) - 1 ;                \
        }

    #else

        #define GB_ATOMIC_CAPTURE_INC64(result,target)                  \
        {                                                               \
            GB_ATOMIC_CAPTURE                                           \
            result = (target)++ ;                                       \
        }

    #endif

//------------------------------------------------------------------------------
// atomic compare-and-exchange
//------------------------------------------------------------------------------

// Atomic compare-and-exchange is used to implement the MAX, MIN and EQ
// monoids, for the fine-grain saxpy-style matrix multiplication.  Ideally,
// OpenMP would be used for these atomic operation, but they are not supported.
// So compiler-specific functions are used instead.

// In gcc, icc, and clang, the atomic compare-and-exchange function
// __atomic_compare_exchange computes the following, as a single atomic
// operation, where type_t is any 8, 16, 32, or 64 bit scalar type.  In
// SuiteSparse:GraphBLAS, type_t can be bool, int8_t, uint8_t, int16_t,
// uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, or double.
//
//      bool __atomic_compare_exchange
//      (
//          type_t *target,         // input/output
//          type_t *expected,       // input/output
//          type_t *desired,        // input only, even though it is a pointer
//          bool weak,              // true, for SuiteSparse:GraphBLAS
//          int success_memorder,   // __ATOMIC_SEQ_CST is used here
//          int failure_memorder    // __ATOMIC_SEQ_CST is used here
//      )
//      {
//          bool result ;
//          if (*target == *expected)
//          {
//              *target = *desired ;
//              result = true ;
//          }
//          else
//          {
//              *expected = *target ;
//              result = false ;
//          }
//          return (result) ;
//      }
//
// The generic __atomic_compare_exchange function in gcc (also supported by
// icc) computes the above for any of these 8, 16, 32, or 64-bit scalar types
// needed in SuiteSparse:GraphBLAS.  SuiteSparse:GraphBLAS does not require the
// 'expected = target' assignment if the result is false.  It ignores the
// value of 'expected' after the operation completes.   The target, expected,
// and desired parameters are all provided as pointers:
//
// See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html

// Microsoft Visual Studio provides similar but not identical functionality in
// the _InterlockedCompareExchange functions, but they are named differently
// for different types.  Only int8_t, int16_t, int32_t, and int64_t types are
// supported.  For the int64_t case, the following is performed atomically:
//
//      int64_t _InterlockedCompareExchange64
//      (
//          int64_t volatile *target,       // input/output
//          int64_t desired                 // input only
//          int64_t expected
//      )
//      {
//          int64_t result = *target ;
//          if (*target == expected)
//          {
//              target = desired ;
//          }
//          return (result) ;
//      }
//
// It does not assign "expected = target" if the test is false, but
// SuiteSparse:GraphBLAS does not require this action.  It does not return a
// boolean result, but instead returns the original value of (*target).
// However, this can be compared with the expected value to obtain the
// same boolean result as __atomic_compare_exchange.
//
// Type punning is used to extend these signed integer types to unsigned
// integers of the same number of bytes, and to float and double.

#if GB_MICROSOFT

    //--------------------------------------------------------------------------
    // GB_PUN: type punning
    //--------------------------------------------------------------------------

    // With type punning, a value is treated as a different type, but with no
    // typecasting.  The address of the variable is first typecasted to a (type
    // *) pointer, and then the pointer is dereferenced.  Type punning is only
    // needed to extend the atomic compare/exchange functions for Microsoft
    // Visual Studio.

    #define GB_PUN(type,value) (*((type *) (&(value))))

    //--------------------------------------------------------------------------
    // compare/exchange for MS Visual Studio
    //--------------------------------------------------------------------------

    // bool, int8_t, and uint8_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_8(target, expected, desired)         \
    (                                                                       \
        GB_PUN (int8_t, expected) ==                                        \
            _InterlockedCompareExchange8 ((int8_t volatile *) (target),     \
                GB_PUN (int8_t, desired), GB_PUN (int8_t, expected))        \
    )

    // int16_t and uint16_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_16(target, expected, desired)        \
    (                                                                       \
        GB_PUN (int16_t, expected) ==                                       \
            _InterlockedCompareExchange16 ((int16_t volatile *) (target),   \
            GB_PUN (int16_t, desired), GB_PUN (int16_t, expected))          \
    )

    // float, int32_t, and uint32_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_32(target, expected, desired)        \
    (                                                                       \
        GB_PUN (int32_t, expected) ==                                       \
            _InterlockedCompareExchange ((int32_t volatile *) (target),     \
            GB_PUN (int32_t, desired), GB_PUN (int32_t, expected))          \
    )

    // double, int64_t, and uint64_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_64(target, expected, desired)        \
    (                                                                       \
        GB_PUN (int64_t, expected) ==                                       \
            _InterlockedCompareExchange64 ((int64_t volatile *) (target),   \
            GB_PUN (int64_t, desired), GB_PUN (int64_t, expected))          \
    )

#else

    //--------------------------------------------------------------------------
    // compare/exchange for gcc, icc, and clang on x86 and Power8/9
    //--------------------------------------------------------------------------

    // the compare/exchange function is generic for any type
    #define GB_ATOMIC_COMPARE_EXCHANGE_X(target, expected, desired)     \
        __atomic_compare_exchange (target, &expected, &desired,         \
            true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)                   \

    // bool, int8_t, and uint8_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_8(target, expected, desired)     \
            GB_ATOMIC_COMPARE_EXCHANGE_X(target, expected, desired)

    // int16_t and uint16_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_16(target, expected, desired)    \
            GB_ATOMIC_COMPARE_EXCHANGE_X (target, expected, desired)

    // float, int32_t, and uint32_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_32(target, expected, desired)    \
            GB_ATOMIC_COMPARE_EXCHANGE_X (target, expected, desired)

    // double, int64_t, and uint64_t
    #define GB_ATOMIC_COMPARE_EXCHANGE_64(target, expected, desired)    \
            GB_ATOMIC_COMPARE_EXCHANGE_X (target, expected, desired)


#endif

#endif


//------------------------------------------------------------------------------
// GB_bitwise.h: definitions for bitwise operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_BITWISE_H
#define GB_BITWISE_H

//------------------------------------------------------------------------------
// bitget, bitset, bitclr
//------------------------------------------------------------------------------

// bitget (x,k) returns a single bit from x, as 0 or 1, whose position is given
// by k.  k = 1 is the least significant bit, and k = bits (64 for uint64)
// is the most significant bit. If k is outside this range, the result is zero.

#define GB_BITGET(x,k,type,bits)                                \
(                                                               \
    (k >= 1 && k <= bits) ?                                     \
    (                                                           \
        /* get the kth bit */                                   \
        ((x & (((type) 1) << (k-1))) ? 1 : 0)                   \
    )                                                           \
    :                                                           \
    (                                                           \
        0                                                       \
    )                                                           \
)

// bitset (x,k) returns x modified by setting a bit from x to 1, whose position
// is given by k.  If k is in the range 1 to bits, then k gives the position
// of the bit to set.  If k is outside the range 1 to bits, then z = x is
// returned, unmodified.

#define GB_BITSET(x,k,type,bits)                                \
(                                                               \
    (k >= 1 && k <= bits) ?                                     \
    (                                                           \
        /* set the kth bit to 1 */                              \
        (x | (((type) 1) << (k-1)))                             \
    )                                                           \
    :                                                           \
    (                                                           \
        x                                                       \
    )                                                           \
)

// bitclr (x,k) returns x modified by setting a bit from x to 0, whose position
// is given by k.  If k is in the range 1 to bits, then k gives the position of
// the bit to clear.  If k is outside the range 1 to GB_BITS, then z = x is
// returned, unmodified.

#define GB_BITCLR(x,k,type,bits)                                \
(                                                               \
    (k >= 1 && k <= bits) ?                                     \
    (                                                           \
        /* set the kth bit to 0 */                              \
        (x & ~(((type) 1) << (k-1)))                            \
    )                                                           \
    :                                                           \
    (                                                           \
        x                                                       \
    )                                                           \
)

//------------------------------------------------------------------------------
// z = bitshift (x,y) when x and z are unsigned
//------------------------------------------------------------------------------

inline uint8_t GB_bitshift_uint8 (uint8_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift to do at all
        return (x) ;
    }
    else if (k >= 8 || k <= -8)
    {
        // ANSI C11 states that the result of x << k is undefined if k is
        // negative or if k is greater than the # of bits in x.  Here, the
        // result is defined to be zero (the same as if shifting left
        // or right by 8).
        return (0) ;
    }
    else if (k > 0)
    {
        // left shift x by k bits.  z is defined by ANSI C11 as
        // (x * (2^k)) mod (uintmax + 1).
        return (x << k) ;
    }
    else
    {
        k = -k ;
        // right shift x by k bits.  z is defined by ANSI C11 as the
        // integral part of the quotient of x / (2^k).
        return (x >> k) ;
    }
}

inline uint16_t GB_bitshift_uint16 (uint16_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift
        return (x) ;
    }
    else if (k >= 16 || k <= -16)
    {
        return (0) ;
    }
    else if (k > 0)
    {
        // left shift
        return (x << k) ;
    }
    else
    {
        // right shift
        return (x >> (-k)) ;
    }
}

inline uint32_t GB_bitshift_uint32 (uint32_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift
        return (x) ;
    }
    else if (k >= 32 || k <= -32)
    {
        return (0) ;
    }
    else if (k > 0)
    {
        // left shift
        return (x << k) ;
    }
    else
    {
        // right shift
        return (x >> (-k)) ;
    }
}

inline uint64_t GB_bitshift_uint64 (uint64_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift
        return (x) ;
    }
    else if (k >= 64 || k <= -64)
    {
        return (0) ;
    }
    else if (k > 0)
    {
        // left shift
        return (x << k) ;
    }
    else
    {
        // right shift
        return (x >> (-k)) ;
    }
}

//------------------------------------------------------------------------------
// z = bitshift (x,y) when x and z are signed
//------------------------------------------------------------------------------

inline int8_t GB_bitshift_int8 (int8_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift to do at all
        return (x) ;
    }
    else if (k >= 8)
    {
        // ANSI C11 states that z = x << k is undefined if k is greater
        // than the # of bits in x.  Here, the result is defined to be zero.
        return (0) ;
    }
    else if (k <= -8)
    {
        // ANSI C11 states that z = x >> (-k) is undefined if (-k) is
        // greater than the # of bits in x.  Here, the result is defined to
        // be the sign of x (z = 0 if x >= 0 and z = -1 if x is negative).
        return ((x >= 0) ? 0 : -1) ;
    }
    else if (k > 0)
    {
        // left shift x by k bits (where k is in range 1 to #bits - 1).
        // ANSI C11 states that z is defined only if x is non-negative and
        // x * (2^k) is representable.  This computation assumes x and z
        // are represented in 2's complement.  The result depends on the
        // underlying machine architecture and the compiler.
        return (x << k) ;
    }
    else
    {
        k = -k ;
        // right shift x by k bits (where k is in range 1 to 8)
        if (x >= 0)
        {
            // ANSI C11 defines z as the integral part of the quotient
            // of x / (2^k).
            return (x >> k) ;
        }
        else
        {
            // ANSI C11 states that the result is implementation-defined if
            // x is negative.  This computation assumes x and z are in 2's
            // complement, so 1-bits are shifted in on the left, and thus
            // the sign bit is always preserved.  The result depends on the
            // underlying machine architecture and the compiler.
            return ((x >> k) | (~(UINT8_MAX >> k))) ;
        }
    }
}

inline int16_t GB_bitshift_int16 (int16_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift
        return (x) ;
    }
    else if (k >= 16)
    {
        return (0) ;
    }
    else if (k <= -16)
    {
        return ((x >= 0) ? 0 : -1) ;
    }
    else if (k > 0)
    {
        // left shift
        return (x << k) ;
    }
    else
    {
        // right shift
        k = -k ;
        if (x >= 0)
        {
            return (x >> k) ;
        }
        else
        {
            return ((x >> k) | (~(UINT16_MAX >> k))) ;
        }
    }
}

inline int32_t GB_bitshift_int32 (int32_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift
        return (x) ;
    }
    else if (k >= 32)
    {
        return (0) ;
    }
    else if (k <= -32)
    {
        return ((x >= 0) ? 0 : -1) ;
    }
    else if (k > 0)
    {
        // left shift
        return (x << k) ;
    }
    else
    {
        // right shift
        k = -k ;
        if (x >= 0)
        {
            return (x >> k) ;
        }
        else
        {
            return ((x >> k) | (~(UINT32_MAX >> k))) ;
        }
    }
}

inline int64_t GB_bitshift_int64 (int64_t x, int8_t k)
{
    if (k == 0)
    {
        // no shift
        return (x) ;
    }
    else if (k >= 64)
    {
        return (0) ;
    }
    else if (k <= -64)
    {
        return ((x >= 0) ? 0 : -1) ;
    }
    else if (k > 0)
    {
        // left shift
        return (x << k) ;
    }
    else
    {
        // right shift
        k = -k ;
        if (x >= 0)
        {
            return (x >> k) ;
        }
        else
        {
            return ((x >> k) | (~(UINT64_MAX >> k))) ;
        }
    }
}

#endif


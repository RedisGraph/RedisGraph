//------------------------------------------------------------------------------
// GB_casting.h: define the unary typecasting functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// TODO: complex code is #ifdef'd out when using CUDA.

#ifndef GB_CASTING_H
#define GB_CASTING_H

//------------------------------------------------------------------------------
// pointer casting function, returned by GB_cast_factory.
//------------------------------------------------------------------------------

typedef void (*GB_cast_function) (void *, const void *, size_t) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GB_cast_function GB_cast_factory   // returns pointer to function to cast x to z
(
    const GB_Type_code code1,      // the type of z, the output value
    const GB_Type_code code2       // the type of x, the input value
) ;

//------------------------------------------------------------------------------
// typecasting from double to integer
//------------------------------------------------------------------------------

// The GraphBLAS C API states that typecasting follows the rules of the C
// language.  However, the ANSI C11 language specification states that results
// are undefined when typecasting a float or double to an integer value that is
// outside the range of the integer type.  MATLAB handles this case by
// typecasting a float or double that is larger than the maximum integer to the
// max integer, and a value less than the minimum integer to the min integer.
// NaN's are typecasted to the integer value zero.  GraphBLAS follows the same
// rules as MATLAB.

inline int8_t GB_cast_to_int8_t (double x)
{ 
    if (isnan (x)) return (0) ;
    if (x <= (double) INT8_MIN) return (INT8_MIN) ;
    if (x >= (double) INT8_MAX) return (INT8_MAX) ;
    return ((int8_t) x) ;
}

inline int16_t GB_cast_to_int16_t (double x)
{ 
    if (isnan (x)) return (0) ;
    if (x <= (double) INT16_MIN) return (INT16_MIN) ;
    if (x >= (double) INT16_MAX) return (INT16_MAX) ;
    return ((int16_t) x) ;
}

inline int32_t GB_cast_to_int32_t (double x)
{ 
    if (isnan (x)) return (0) ;
    if (x <= (double) INT32_MIN) return (INT32_MIN) ;
    if (x >= (double) INT32_MAX) return (INT32_MAX) ;
    return ((int32_t) x) ;
}

inline int64_t GB_cast_to_int64_t (double x)
{ 
    if (isnan (x)) return (0) ;
    if (x <= (double) INT64_MIN) return (INT64_MIN) ;
    if (x >= (double) INT64_MAX) return (INT64_MAX) ;
    return ((int64_t) x) ;
}

inline uint8_t GB_cast_to_uint8_t (double x)
{ 
    if (isnan (x) || x <= 0) return (0) ;
    if (x >= (double) UINT8_MAX) return (UINT8_MAX) ;
    return ((uint8_t) x) ;
}

inline uint16_t GB_cast_to_uint16_t (double x)
{ 
    if (isnan (x) || x <= 0) return (0) ;
    if (x >= (double) UINT16_MAX) return (UINT16_MAX) ;
    return ((uint16_t) x) ;
}

inline uint32_t GB_cast_to_uint32_t (double x)
{ 
    if (isnan (x) || x <= 0) return (0) ;
    if (x >= (double) UINT32_MAX) return (UINT32_MAX) ;
    return ((uint32_t) x) ;
}

inline uint64_t GB_cast_to_uint64_t (double x)
{ 
    if (isnan (x) || x <= 0) return (0) ;
    if (x >= (double) UINT64_MAX) return (UINT64_MAX) ;
    return ((uint64_t) x) ;
}

//------------------------------------------------------------------------------
// unary typecast operators, used in GB_cast_factory.c.
//------------------------------------------------------------------------------

// construct the typecast functions: z = (ztype) x, where x has type xtype.

// GB_cast_ztype_xtype casts a single value ((xtype) x) into the result
// ((ztype) z).  Functions with the xtype and ztype the same, such as
// GB_cast_double_double, simply copy their input to their output.

// The s parameter is not used in these functions.  It is present because one
// function returned by GB_cast_factory requires it (GB_copy_user_user).

void GB_copy_user_user (void *z, const void *x, size_t s) ;

#define GB_CAST_FUNCTION(ztype,xtype)                                   \
inline void GB_cast_ ## ztype ## _ ## xtype                             \
(                                                                       \
    void *z,            /* typecasted output, of type ztype */          \
    const void *x,      /* input value to typecast, of type xtype */    \
    size_t s            /* size of type, for GB_copy_user_user only */  \
)                                                                       \
{                                                                       \
    xtype xx = (*((xtype *) x)) ;                                       \
    ztype zz = GB_CAST (ztype, xx) ;                                    \
    (*((ztype *) z)) = zz ;                                             \
}

//------------------------------------------------------------------------------
// typecast to boolean
//------------------------------------------------------------------------------

// Typecasting a NaN to a bool results in 'true', as defined by the ANSI C11
// standard (NaN converts to true, since Nan != 0 is true).  MATLAB throws an
// error when trying to convert NaN's to logical.  GraphBLAS follows the ANSI
// C11 standard in this case.

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (bool      , bool      )
#undef  GB_CAST
#define GB_CAST(ztype,x) (x != 0)
GB_CAST_FUNCTION (bool      , int8_t    )
GB_CAST_FUNCTION (bool      , int16_t   )
GB_CAST_FUNCTION (bool      , int32_t   )
GB_CAST_FUNCTION (bool      , int64_t   )
GB_CAST_FUNCTION (bool      , uint8_t   )
GB_CAST_FUNCTION (bool      , uint16_t  )
GB_CAST_FUNCTION (bool      , uint32_t  )
GB_CAST_FUNCTION (bool      , uint64_t  )
GB_CAST_FUNCTION (bool      , float     )
GB_CAST_FUNCTION (bool      , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) (crealf (x) != 0 || cimagf (x) != 0)
GB_CAST_FUNCTION (bool      , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) (creal (x) != 0 || cimag (x) != 0)
GB_CAST_FUNCTION (bool      , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to int8_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (int8_t    , bool      )
GB_CAST_FUNCTION (int8_t    , int8_t    )
GB_CAST_FUNCTION (int8_t    , int16_t   )
GB_CAST_FUNCTION (int8_t    , int32_t   )
GB_CAST_FUNCTION (int8_t    , int64_t   )
GB_CAST_FUNCTION (int8_t    , uint8_t   )
GB_CAST_FUNCTION (int8_t    , uint16_t  )
GB_CAST_FUNCTION (int8_t    , uint32_t  )
GB_CAST_FUNCTION (int8_t    , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int8_t ((double) x)
GB_CAST_FUNCTION (int8_t    , float     )
GB_CAST_FUNCTION (int8_t    , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int8_t ((double) crealf (x))
GB_CAST_FUNCTION (int8_t    , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int8_t (creal (x))
GB_CAST_FUNCTION (int8_t    , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to int16_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (int16_t   , bool      )
GB_CAST_FUNCTION (int16_t   , int8_t    )
GB_CAST_FUNCTION (int16_t   , int16_t   )
GB_CAST_FUNCTION (int16_t   , int32_t   )
GB_CAST_FUNCTION (int16_t   , int64_t   )
GB_CAST_FUNCTION (int16_t   , uint8_t   )
GB_CAST_FUNCTION (int16_t   , uint16_t  )
GB_CAST_FUNCTION (int16_t   , uint32_t  )
GB_CAST_FUNCTION (int16_t   , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int16_t ((double) x)
GB_CAST_FUNCTION (int16_t   , float     )
GB_CAST_FUNCTION (int16_t   , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int16_t ((double) crealf (x))
GB_CAST_FUNCTION (int16_t   , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int16_t (creal (x))
GB_CAST_FUNCTION (int16_t   , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to int32_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (int32_t   , bool      )
GB_CAST_FUNCTION (int32_t   , int8_t    )
GB_CAST_FUNCTION (int32_t   , int16_t   )
GB_CAST_FUNCTION (int32_t   , int32_t   )
GB_CAST_FUNCTION (int32_t   , int64_t   )
GB_CAST_FUNCTION (int32_t   , uint8_t   )
GB_CAST_FUNCTION (int32_t   , uint16_t  )
GB_CAST_FUNCTION (int32_t   , uint32_t  )
GB_CAST_FUNCTION (int32_t   , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int32_t ((double) x)
GB_CAST_FUNCTION (int32_t   , float     )
GB_CAST_FUNCTION (int32_t   , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int32_t ((double) crealf (x))
GB_CAST_FUNCTION (int32_t   , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int32_t (creal (x))
GB_CAST_FUNCTION (int32_t   , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to int64_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (int64_t   , bool      )
GB_CAST_FUNCTION (int64_t   , int8_t    )
GB_CAST_FUNCTION (int64_t   , int16_t   )
GB_CAST_FUNCTION (int64_t   , int32_t   )
GB_CAST_FUNCTION (int64_t   , int64_t   )
GB_CAST_FUNCTION (int64_t   , uint8_t   )
GB_CAST_FUNCTION (int64_t   , uint16_t  )
GB_CAST_FUNCTION (int64_t   , uint32_t  )
GB_CAST_FUNCTION (int64_t   , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int64_t ((double) x)
GB_CAST_FUNCTION (int64_t   , float     )
GB_CAST_FUNCTION (int64_t   , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int64_t ((double) crealf (x))
GB_CAST_FUNCTION (int64_t   , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_int64_t (creal (x))
GB_CAST_FUNCTION (int64_t   , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to uint8_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (uint8_t   , bool      )
GB_CAST_FUNCTION (uint8_t   , int8_t    )
GB_CAST_FUNCTION (uint8_t   , int16_t   )
GB_CAST_FUNCTION (uint8_t   , int32_t   )
GB_CAST_FUNCTION (uint8_t   , int64_t   )
GB_CAST_FUNCTION (uint8_t   , uint8_t   )
GB_CAST_FUNCTION (uint8_t   , uint16_t  )
GB_CAST_FUNCTION (uint8_t   , uint32_t  )
GB_CAST_FUNCTION (uint8_t   , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint8_t ((double) x)
GB_CAST_FUNCTION (uint8_t   , float     )
GB_CAST_FUNCTION (uint8_t   , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint8_t ((double) crealf (x))
GB_CAST_FUNCTION (uint8_t   , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint8_t (creal (x))
GB_CAST_FUNCTION (uint8_t   , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to uint16_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (uint16_t  , bool      )
GB_CAST_FUNCTION (uint16_t  , int8_t    )
GB_CAST_FUNCTION (uint16_t  , int16_t   )
GB_CAST_FUNCTION (uint16_t  , int32_t   )
GB_CAST_FUNCTION (uint16_t  , int64_t   )
GB_CAST_FUNCTION (uint16_t  , uint8_t   )
GB_CAST_FUNCTION (uint16_t  , uint16_t  )
GB_CAST_FUNCTION (uint16_t  , uint32_t  )
GB_CAST_FUNCTION (uint16_t  , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint16_t ((double) x)
GB_CAST_FUNCTION (uint16_t  , float     )
GB_CAST_FUNCTION (uint16_t  , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint16_t ((double) crealf (x))
GB_CAST_FUNCTION (uint16_t  , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint16_t (creal (x))
GB_CAST_FUNCTION (uint16_t  , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to uint32_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (uint32_t  , bool      )
GB_CAST_FUNCTION (uint32_t  , int8_t    )
GB_CAST_FUNCTION (uint32_t  , int16_t   )
GB_CAST_FUNCTION (uint32_t  , int32_t   )
GB_CAST_FUNCTION (uint32_t  , int64_t   )
GB_CAST_FUNCTION (uint32_t  , uint8_t   )
GB_CAST_FUNCTION (uint32_t  , uint16_t  )
GB_CAST_FUNCTION (uint32_t  , uint32_t  )
GB_CAST_FUNCTION (uint32_t  , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint32_t ((double) x)
GB_CAST_FUNCTION (uint32_t  , float     )
GB_CAST_FUNCTION (uint32_t  , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint32_t ((double) crealf (x))
GB_CAST_FUNCTION (uint32_t  , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint32_t (creal (x))
GB_CAST_FUNCTION (uint32_t  , GxB_FC64_t)
#endif 

//------------------------------------------------------------------------------
// typecast to uint64_t
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (uint64_t  , bool      )
GB_CAST_FUNCTION (uint64_t  , int8_t    )
GB_CAST_FUNCTION (uint64_t  , int16_t   )
GB_CAST_FUNCTION (uint64_t  , int32_t   )
GB_CAST_FUNCTION (uint64_t  , int64_t   )
GB_CAST_FUNCTION (uint64_t  , uint8_t   )
GB_CAST_FUNCTION (uint64_t  , uint16_t  )
GB_CAST_FUNCTION (uint64_t  , uint32_t  )
GB_CAST_FUNCTION (uint64_t  , uint64_t  )
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint64_t ((double) x)
GB_CAST_FUNCTION (uint64_t  , float     )
GB_CAST_FUNCTION (uint64_t  , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint64_t ((double) crealf (x))
GB_CAST_FUNCTION (uint64_t  , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GB_cast_to_uint64_t (creal (x))
GB_CAST_FUNCTION (uint64_t  , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to float
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (float     , bool      )
GB_CAST_FUNCTION (float     , int8_t    )
GB_CAST_FUNCTION (float     , int16_t   )
GB_CAST_FUNCTION (float     , int32_t   )
GB_CAST_FUNCTION (float     , int64_t   )
GB_CAST_FUNCTION (float     , uint8_t   )
GB_CAST_FUNCTION (float     , uint16_t  )
GB_CAST_FUNCTION (float     , uint32_t  )
GB_CAST_FUNCTION (float     , uint64_t  )
GB_CAST_FUNCTION (float     , float     )
GB_CAST_FUNCTION (float     , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) crealf (x)
GB_CAST_FUNCTION (float     , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) ((float) creal (x))
GB_CAST_FUNCTION (float     , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to double
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) (ztype) x
GB_CAST_FUNCTION (double    , bool      )
GB_CAST_FUNCTION (double    , int8_t    )
GB_CAST_FUNCTION (double    , int16_t   )
GB_CAST_FUNCTION (double    , int32_t   )
GB_CAST_FUNCTION (double    , int64_t   )
GB_CAST_FUNCTION (double    , uint8_t   )
GB_CAST_FUNCTION (double    , uint16_t  )
GB_CAST_FUNCTION (double    , uint32_t  )
GB_CAST_FUNCTION (double    , uint64_t  )
GB_CAST_FUNCTION (double    , float     )
GB_CAST_FUNCTION (double    , double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) ((double) crealf (x))
GB_CAST_FUNCTION (double    , GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) creal (x)
GB_CAST_FUNCTION (double    , GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to float complex
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) GxB_CMPLXF ((float) x, (float) 0)
GB_CAST_FUNCTION (GxB_FC32_t, bool      )
GB_CAST_FUNCTION (GxB_FC32_t, int8_t    )
GB_CAST_FUNCTION (GxB_FC32_t, int16_t   )
GB_CAST_FUNCTION (GxB_FC32_t, int32_t   )
GB_CAST_FUNCTION (GxB_FC32_t, int64_t   )
GB_CAST_FUNCTION (GxB_FC32_t, uint8_t   )
GB_CAST_FUNCTION (GxB_FC32_t, uint16_t  )
GB_CAST_FUNCTION (GxB_FC32_t, uint32_t  )
GB_CAST_FUNCTION (GxB_FC32_t, uint64_t  )
GB_CAST_FUNCTION (GxB_FC32_t, float     )
GB_CAST_FUNCTION (GxB_FC32_t, double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) x
GB_CAST_FUNCTION (GxB_FC32_t, GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) GxB_CMPLXF ((float) creal (x), (float) cimag (x))
GB_CAST_FUNCTION (GxB_FC32_t, GxB_FC64_t)
#endif

//------------------------------------------------------------------------------
// typecast to double complex
//------------------------------------------------------------------------------

#undef  GB_CAST
#define GB_CAST(ztype,x) GxB_CMPLX ((double) x, (double) 0)
GB_CAST_FUNCTION (GxB_FC64_t, bool      )
GB_CAST_FUNCTION (GxB_FC64_t, int8_t    )
GB_CAST_FUNCTION (GxB_FC64_t, int16_t   )
GB_CAST_FUNCTION (GxB_FC64_t, int32_t   )
GB_CAST_FUNCTION (GxB_FC64_t, int64_t   )
GB_CAST_FUNCTION (GxB_FC64_t, uint8_t   )
GB_CAST_FUNCTION (GxB_FC64_t, uint16_t  )
GB_CAST_FUNCTION (GxB_FC64_t, uint32_t  )
GB_CAST_FUNCTION (GxB_FC64_t, uint64_t  )
GB_CAST_FUNCTION (GxB_FC64_t, float     )
GB_CAST_FUNCTION (GxB_FC64_t, double    )
#ifndef GBCUDA
// TODO: this does not work on CUDA yet
#undef  GB_CAST
#define GB_CAST(ztype,x) GxB_CMPLX ((double) crealf (x), (double) cimagf (x))
GB_CAST_FUNCTION (GxB_FC64_t, GxB_FC32_t)
#undef  GB_CAST
#define GB_CAST(ztype,x) x
GB_CAST_FUNCTION (GxB_FC64_t, GxB_FC64_t)

#undef  GB_CAST
#undef  GB_CAST_FUNCTION
#endif

//------------------------------------------------------------------------------
// GB_mcast: cast a mask entry from any native type to boolean
//------------------------------------------------------------------------------

// The mask matrix M must be one of the native data types, which have sizes of
// 1, 2, 4, 8, or 16 bytes.  The value could be properly typecasted to bool,
// but this requires a function pointer to the proper GB_cast_function.
// Instead, it is faster to simply use type punning, based on the size of the
// data type, and use the inline GB_mcast function instead.

static inline bool GB_mcast         // return the value of M(i,j)
(
    const GB_void *GB_RESTRICT Mx,  // mask values
    const int64_t pM,               // extract boolean value of Mx [pM]
    const size_t msize              // size of each data type
)
{
    if (Mx == NULL)
    {
        // If Mx is NULL, then values in the mask matrix M are ignored, and
        // only the structural pattern is used.  This function is only called
        // for entries M(i,j) in the structure of M, so the result is always
        // true if Mx is NULL.
        return (true) ;
    }
    else
    {
        // check the value of M(i,j)
        switch (msize)
        {
            default:
            case 1: return ((*(uint8_t  *) (Mx +((pM)*1))) != 0) ;
            case 2: return ((*(uint16_t *) (Mx +((pM)*2))) != 0) ;
            case 4: return ((*(uint32_t *) (Mx +((pM)*4))) != 0) ;
            case 8: return ((*(uint64_t *) (Mx +((pM)*8))) != 0) ;
            case 16:
            {
                const uint64_t *GB_RESTRICT Zx = (uint64_t *) Mx ;
                return (Zx [2*pM] != 0 || Zx [2*pM+1] != 0) ;
            }
        }
    }
}

#endif

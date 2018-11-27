//------------------------------------------------------------------------------
// GB_mx_Monoid: construct a monoid from a built-in operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Also defines the identity of the monoid

// See Source/GB_AxB_Gustavson_builtin.c for a description of the built-in
// monoids.  This function can construct all 52 of them (note that 8 of those
// are redudant).

#define GET_DEEP_COPY ;
#define FREE_DEEP_COPY ;
#define FREE_ALL ;

#include "GB_mex.h"

bool GB_mx_Monoid               // true if successful, false otherwise
(
    GrB_Monoid *handle,         // monoid to construct
    const GrB_BinaryOp add,     // monoid operator
    const bool malloc_debug     // true if malloc debug should be done
)
{

    GrB_Monoid M = NULL ;
    (*handle) = NULL ;
    if (add == NULL)
    {
        mexWarnMsgIdAndTxt ("GB:warn", "monoid operator missing") ;
        return (false) ;
    }

    switch (add->opcode)
    {
        case GB_MIN_opcode     :

            // 11 MIN monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with AND
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) true)) ;        break ;
                case GB_INT8_code   : METHOD (GrB_Monoid_new (&M, add, (int8_t  ) INT8_MAX)) ;    break ;
                case GB_UINT8_code  : METHOD (GrB_Monoid_new (&M, add, (uint8_t ) UINT8_MAX)) ;   break ;
                case GB_INT16_code  : METHOD (GrB_Monoid_new (&M, add, (int16_t ) INT16_MAX)) ;   break ;
                case GB_UINT16_code : METHOD (GrB_Monoid_new (&M, add, (uint16_t) UINT16_MAX)) ;  break ;
                case GB_INT32_code  : METHOD (GrB_Monoid_new (&M, add, (int32_t ) INT32_MAX)) ;   break ;
                case GB_UINT32_code : METHOD (GrB_Monoid_new (&M, add, (uint32_t) UINT32_MAX)) ;  break ;
                case GB_INT64_code  : METHOD (GrB_Monoid_new (&M, add, (int64_t ) INT64_MAX)) ;   break ;
                case GB_UINT64_code : METHOD (GrB_Monoid_new (&M, add, (uint64_t) UINT64_MAX)) ;  break ;
                case GB_FP32_code   : METHOD (GrB_Monoid_new (&M, add, (float   ) INFINITY)) ;    break ;
                case GB_FP64_code   : METHOD (GrB_Monoid_new (&M, add, (double  ) INFINITY)) ;    break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (false) ;
            }
            break ;

        case GB_MAX_opcode     :

            // 11 MAX monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with OR
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) false)) ;       break ;
                case GB_INT8_code   : METHOD (GrB_Monoid_new (&M, add, (int8_t  ) INT8_MIN)) ;    break ;
                case GB_UINT8_code  : METHOD (GrB_Monoid_new (&M, add, (uint8_t ) 0)) ;           break ;
                case GB_INT16_code  : METHOD (GrB_Monoid_new (&M, add, (int16_t ) INT16_MIN)) ;   break ;
                case GB_UINT16_code : METHOD (GrB_Monoid_new (&M, add, (uint16_t) 0)) ;           break ;
                case GB_INT32_code  : METHOD (GrB_Monoid_new (&M, add, (int32_t ) INT32_MIN)) ;   break ;
                case GB_UINT32_code : METHOD (GrB_Monoid_new (&M, add, (uint32_t) 0)) ;           break ;
                case GB_INT64_code  : METHOD (GrB_Monoid_new (&M, add, (int64_t ) INT64_MIN)) ;   break ;
                case GB_UINT64_code : METHOD (GrB_Monoid_new (&M, add, (uint64_t) 0)) ;           break ;
                case GB_FP32_code   : METHOD (GrB_Monoid_new (&M, add, (float   ) -INFINITY)) ;   break ;
                case GB_FP64_code   : METHOD (GrB_Monoid_new (&M, add, (double  ) -INFINITY)) ;   break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (false) ;
            }
            break ;

        case GB_PLUS_opcode    :

            // 11 PLUS monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with OR
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) 0)) ;           break ;
                case GB_INT8_code   : METHOD (GrB_Monoid_new (&M, add, (int8_t  ) 0)) ;           break ;
                case GB_UINT8_code  : METHOD (GrB_Monoid_new (&M, add, (uint8_t ) 0)) ;           break ;
                case GB_INT16_code  : METHOD (GrB_Monoid_new (&M, add, (int16_t ) 0)) ;           break ;
                case GB_UINT16_code : METHOD (GrB_Monoid_new (&M, add, (uint16_t) 0)) ;           break ;
                case GB_INT32_code  : METHOD (GrB_Monoid_new (&M, add, (int32_t ) 0)) ;           break ;
                case GB_UINT32_code : METHOD (GrB_Monoid_new (&M, add, (uint32_t) 0)) ;           break ;
                case GB_INT64_code  : METHOD (GrB_Monoid_new (&M, add, (int64_t ) 0)) ;           break ;
                case GB_UINT64_code : METHOD (GrB_Monoid_new (&M, add, (uint64_t) 0)) ;           break ;
                case GB_FP32_code   : METHOD (GrB_Monoid_new (&M, add, (float   ) 0)) ;           break ;
                case GB_FP64_code   : METHOD (GrB_Monoid_new (&M, add, (double  ) 0)) ;           break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (false) ;
            }
            break ;

        case GB_TIMES_opcode   :

            // 11 TIMES monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with AND
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) true)) ;        break ;
                case GB_INT8_code   : METHOD (GrB_Monoid_new (&M, add, (int8_t  ) 1)) ;           break ;
                case GB_UINT8_code  : METHOD (GrB_Monoid_new (&M, add, (uint8_t ) 1)) ;           break ;
                case GB_INT16_code  : METHOD (GrB_Monoid_new (&M, add, (int16_t ) 1)) ;           break ;
                case GB_UINT16_code : METHOD (GrB_Monoid_new (&M, add, (uint16_t) 1)) ;           break ;
                case GB_INT32_code  : METHOD (GrB_Monoid_new (&M, add, (int32_t ) 1)) ;           break ;
                case GB_UINT32_code : METHOD (GrB_Monoid_new (&M, add, (uint32_t) 1)) ;           break ;
                case GB_INT64_code  : METHOD (GrB_Monoid_new (&M, add, (int64_t ) 1)) ;           break ;
                case GB_UINT64_code : METHOD (GrB_Monoid_new (&M, add, (uint64_t) 1)) ;           break ;
                case GB_FP32_code   : METHOD (GrB_Monoid_new (&M, add, (float   ) 1)) ;           break ;
                case GB_FP64_code   : METHOD (GrB_Monoid_new (&M, add, (double  ) 1)) ;           break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (false) ;
            }
            break ;

        case GB_LOR_opcode      :

            // 2 OR boolean monoids
            // both GrB_LOR and GxB_LOR_BOOL (same opcode)
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) false)) ;        break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (false) ;
            }
            break ;

        case GB_LAND_opcode     :

            // 2 AND boolean monoids
            // both GrB_LAND and GxB_LAND_BOOL (same opcode)
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) true)) ;        break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (false) ;
            }
            break ;

        // both GrB_LXOR and GxB_LXOR_BOOL (same opcode)
        case GB_LXOR_opcode     :

            // 2 XOR boolean monoids
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) false)) ;        break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (false) ;
            }
            break ;

        // both GrB_EQ_BOOL and GxB_ISEQ_BOOL (same opcode)
        case GB_ISEQ_opcode     :
        case GB_EQ_opcode     :

            // EQ and ISEQ boolean monoids
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : METHOD (GrB_Monoid_new (&M, add, (bool    ) true)) ;         break ;
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (false) ;
            }
            break ;

        default: 
            mexWarnMsgIdAndTxt ("GB:warn", "unsupported add operator") ;
            return (false) ;
    }

    ASSERT_OK (GB_check (M, "monoid", GB0)) ;
    (*handle) = M ;
    return (true) ;
}


//------------------------------------------------------------------------------
// GB_mx_builtin_monoid: return a built-in monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns one of 44 unique built-in monoids.  8 more are allowed on input,
// and are renamed; the renamed monoid is returned

#include "GB_mex.h"

GrB_Monoid GB_mx_builtin_monoid     // built-in monoid, or NULL if error
(
    const GrB_BinaryOp add          // monoid operator
)
{

    if (add == NULL)
    {
        mexWarnMsgIdAndTxt ("GB:warn", "monoid operator missing") ;
        return (NULL) ;
    }

    switch (add->opcode)
    {
        case GB_MIN_opcode     :

            // 11 MIN monoids
            switch (add->xtype->code)
            {
                // bool case redudant with AND
                case GB_BOOL_code   : return (GxB_LAND_BOOL_MONOID    ) ;   // identity: true
                case GB_INT8_code   : return (GxB_MIN_INT8_MONOID     ) ;   // identity: INT8_MAX
                case GB_UINT8_code  : return (GxB_MIN_UINT8_MONOID    ) ;   // identity: UINT8_MAX
                case GB_INT16_code  : return (GxB_MIN_INT16_MONOID    ) ;   // identity: INT16_MAX
                case GB_UINT16_code : return (GxB_MIN_UINT16_MONOID   ) ;   // identity: UINT16_MAX
                case GB_INT32_code  : return (GxB_MIN_INT32_MONOID    ) ;   // identity: INT32_MAX
                case GB_UINT32_code : return (GxB_MIN_UINT32_MONOID   ) ;   // identity: UINT32_MAX 
                case GB_INT64_code  : return (GxB_MIN_INT64_MONOID    ) ;   // identity: INT64_MAX
                case GB_UINT64_code : return (GxB_MIN_UINT64_MONOID   ) ;   // identity: UINT64_MAX
                case GB_FP32_code   : return (GxB_MIN_FP32_MONOID     ) ;   // identity: INFINITY
                case GB_FP64_code   : return (GxB_MIN_FP64_MONOID     ) ;   // identity: INFINITY
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (NULL) ;
            }
            break ;

        case GB_MAX_opcode     :

            // 11 MAX monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with OR
                case GB_BOOL_code   : return (GxB_LOR_BOOL_MONOID     ) ;   // identity: false
                case GB_INT8_code   : return (GxB_MAX_INT8_MONOID     ) ;   // identity: INT8_MIN
                case GB_UINT8_code  : return (GxB_MAX_UINT8_MONOID    ) ;   // identity: 0
                case GB_INT16_code  : return (GxB_MAX_INT16_MONOID    ) ;   // identity: INT16_MIN
                case GB_UINT16_code : return (GxB_MAX_UINT16_MONOID   ) ;   // identity: 0
                case GB_INT32_code  : return (GxB_MAX_INT32_MONOID    ) ;   // identity: INT32_MIN
                case GB_UINT32_code : return (GxB_MAX_UINT32_MONOID   ) ;   // identity: 0
                case GB_INT64_code  : return (GxB_MAX_INT64_MONOID    ) ;   // identity: INT64_MIN
                case GB_UINT64_code : return (GxB_MAX_UINT64_MONOID   ) ;   // identity: 0
                case GB_FP32_code   : return (GxB_MAX_FP32_MONOID     ) ;   // identity: -INFINITY
                case GB_FP64_code   : return (GxB_MAX_FP64_MONOID     ) ;   // identity: -INFINITY
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (NULL) ;
            }
            break ;

        case GB_PLUS_opcode    :

            // 11 PLUS monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with OR
                case GB_BOOL_code   : return (GxB_LOR_BOOL_MONOID     ) ;   // identity: false
                case GB_INT8_code   : return (GxB_PLUS_INT8_MONOID    ) ;   // identity: 0
                case GB_UINT8_code  : return (GxB_PLUS_UINT8_MONOID   ) ;   // identity: 0
                case GB_INT16_code  : return (GxB_PLUS_INT16_MONOID   ) ;   // identity: 0
                case GB_UINT16_code : return (GxB_PLUS_UINT16_MONOID  ) ;   // identity: 0
                case GB_INT32_code  : return (GxB_PLUS_INT32_MONOID   ) ;   // identity: 0
                case GB_UINT32_code : return (GxB_PLUS_UINT32_MONOID  ) ;   // identity: 0
                case GB_INT64_code  : return (GxB_PLUS_INT64_MONOID   ) ;   // identity: 0
                case GB_UINT64_code : return (GxB_PLUS_UINT64_MONOID  ) ;   // identity: 0
                case GB_FP32_code   : return (GxB_PLUS_FP32_MONOID    ) ;   // identity: 0
                case GB_FP64_code   : return (GxB_PLUS_FP64_MONOID    ) ;   // identity: 0
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (NULL) ;
            }
            break ;

        case GB_TIMES_opcode   :

            // 11 TIMES monoids
            switch (add->xtype->code)
            {
                // bool case redudandt with AND
                case GB_BOOL_code   : return (GxB_LAND_BOOL_MONOID    ) ;   // identity: true
                case GB_INT8_code   : return (GxB_TIMES_INT8_MONOID   ) ;   // identity: 1
                case GB_UINT8_code  : return (GxB_TIMES_UINT8_MONOID  ) ;   // identity: 1
                case GB_INT16_code  : return (GxB_TIMES_INT16_MONOID  ) ;   // identity: 1
                case GB_UINT16_code : return (GxB_TIMES_UINT16_MONOID ) ;   // identity: 1
                case GB_INT32_code  : return (GxB_TIMES_INT32_MONOID  ) ;   // identity: 1
                case GB_UINT32_code : return (GxB_TIMES_UINT32_MONOID ) ;   // identity: 1
                case GB_INT64_code  : return (GxB_TIMES_INT64_MONOID  ) ;   // identity: 1
                case GB_UINT64_code : return (GxB_TIMES_UINT64_MONOID ) ;   // identity: 1
                case GB_FP32_code   : return (GxB_TIMES_FP32_MONOID   ) ;   // identity: 1
                case GB_FP64_code   : return (GxB_TIMES_FP64_MONOID   ) ;   // identity: 1
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "unknown type") ;
                    return (NULL) ;
            }
            break ;

        case GB_LOR_opcode      :

            // 2 OR boolean monoids
            // both GrB_LOR and GxB_LOR_BOOL (same opcode)
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : return (GxB_LOR_BOOL_MONOID     ) ;   // identity: false
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (NULL) ;
            }
            break ;

        case GB_LAND_opcode     :

            // 2 AND boolean monoids
            // both GrB_LAND and GxB_LAND_BOOL (same opcode)
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : return (GxB_LAND_BOOL_MONOID    ) ;   // identity: true
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (NULL) ;
            }
            break ;

        // both GrB_LXOR and GxB_LXOR_BOOL (same opcode)
        case GB_LXOR_opcode     :

            // 2 XOR boolean monoids
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : return (GxB_LXOR_BOOL_MONOID    ) ;   // identity: false
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (NULL) ;
            }
            break ;


        // both GrB_EQ_BOOL and GxB_ISEQ_BOOL (same opcode)
        case GB_ISEQ_opcode     :
        case GB_EQ_opcode     :

            // EQ and ISEQ boolean monoids
            switch (add->xtype->code)
            {
                case GB_BOOL_code   : return (GxB_EQ_BOOL_MONOID      ) ;   // identity: true
                default: 
                    mexWarnMsgIdAndTxt ("GB:warn", "invalid monoid") ;
                    return (NULL) ;
            }
            break ;

        default: 
            mexWarnMsgIdAndTxt ("GB:warn", "unsupported add operator") ;
            return (NULL) ;
    }

    mexWarnMsgIdAndTxt ("GB:warn", "unknown monoid") ;
    return (NULL) ;
}


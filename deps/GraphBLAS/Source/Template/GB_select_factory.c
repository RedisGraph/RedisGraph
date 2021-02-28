//------------------------------------------------------------------------------
// GB_select_factory: switch factory for C=select(A,thunk)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

switch (opcode)
{

    case GB_TRIL_opcode          : GB_SEL_WORKER (_tril    , _any, GB_void)
    case GB_TRIU_opcode          : GB_SEL_WORKER (_triu    , _any, GB_void)
    case GB_DIAG_opcode          : GB_SEL_WORKER (_diag    , _any, GB_void)
    case GB_OFFDIAG_opcode       : GB_SEL_WORKER (_offdiag , _any, GB_void)
    case GB_RESIZE_opcode        : GB_SEL_WORKER (_resize  , _any, GB_void)
    case GB_USER_SELECT_opcode   : GB_SEL_WORKER (_user    , _any, GB_void)

    case GB_NONZOMBIE_opcode :  // A(i,j) not a zombie

        #ifdef GB_SELECT_PHASE1
        // phase1: use a single worker for all types, since the test does not
        // depend on the values, just Ai.
        GB_SEL_WORKER (_nonzombie, _any, GB_void)
        #else
        // phase2:
        switch (typecode)
        {
            case GB_BOOL_code   : GB_SEL_WORKER (_nonzombie, _bool  , bool    )
            case GB_INT8_code   : GB_SEL_WORKER (_nonzombie, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_nonzombie, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_nonzombie, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_nonzombie, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_nonzombie, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_nonzombie, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_nonzombie, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_nonzombie, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_nonzombie, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_nonzombie, _fp64  , double  )
            default             : GB_SEL_WORKER (_nonzombie, _any   , GB_void )
        }
        break ;
        #endif

    case GB_NONZERO_opcode   :  // A(i,j) != 0

        switch (typecode)
        {
            case GB_BOOL_code   : GB_SEL_WORKER (_nonzero, _bool  , bool    )
            case GB_INT8_code   : GB_SEL_WORKER (_nonzero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_nonzero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_nonzero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_nonzero, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_nonzero, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_nonzero, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_nonzero, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_nonzero, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_nonzero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_nonzero, _fp64  , double  )
            default             : GB_SEL_WORKER (_nonzero, _any   , GB_void )
        }
        break ;

    case GB_EQ_ZERO_opcode   :  // A(i,j) == 0

        switch (typecode)
        {
            case GB_BOOL_code   : GB_SEL_WORKER (_eq_zero, _bool  , bool    )
            case GB_INT8_code   : GB_SEL_WORKER (_eq_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_eq_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_eq_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_eq_zero, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_eq_zero, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_eq_zero, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_eq_zero, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_eq_zero, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_eq_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_eq_zero, _fp64  , double  )
            default             : GB_SEL_WORKER (_eq_zero, _any   , GB_void )
        }
        break ;

    case GB_GT_ZERO_opcode   :  // A(i,j) > 0

        // bool and uint: renamed GxB_GT_ZERO to GxB_NONZERO
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_gt_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_gt_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_gt_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_gt_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_gt_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_gt_zero, _fp64  , double  )
            default: ;          // not for uint, bool, or user-defined ttypes
        }
        break ;

    case GB_GE_ZERO_opcode   :  // A(i,j) >= 0

        // bool and uint: always true; use GB_dup
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_ge_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_ge_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_ge_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_ge_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_ge_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_ge_zero, _fp64  , double  )
            default: ;          // not for uint, bool, or user-defined ttypes
        }
        break ;

    case GB_LT_ZERO_opcode   :  // A(i,j) < 0

        // bool and uint: always false; return an empty matrix
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_lt_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_lt_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_lt_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_lt_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_lt_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_lt_zero, _fp64  , double  )
            default: ;          // not for uint, bool, or user-defined ttypes
        }
        break ;

    case GB_LE_ZERO_opcode   :  // A(i,j) <= 0

        // bool and uint: renamed GxB_LE_ZERO to GxB_EQ_ZERO
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_le_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_le_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_le_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_le_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_le_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_le_zero, _fp64  , double  )
            default: ;          // not for uint, bool, or user-defined ttypes
        }
        break ;

    case GB_NE_THUNK_opcode   : // A(i,j) != thunk

        // bool: if thunk is true,  renamed GxB_NE_THUNK to GxB_EQ_ZERO 
        //       if thunk is false, renamed GxB_NE_THUNK to GxB_NONZERO 
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_ne_thunk, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_ne_thunk, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_ne_thunk, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_ne_thunk, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_ne_thunk, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_ne_thunk, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_ne_thunk, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_ne_thunk, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_ne_thunk, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_ne_thunk, _fp64  , double  )
            default             :
                GB_SEL_WORKER (_ne_thunk, _any   , GB_void )
        }
        break ;

    case GB_EQ_THUNK_opcode   : // A(i,j) == thunk

        // bool: if thunk is true,  renamed GxB_NE_THUNK to GxB_NONZERO 
        //       if thunk is false, renamed GxB_NE_THUNK to GxB_EQ_ZERO 
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_eq_thunk, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_eq_thunk, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_eq_thunk, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_eq_thunk, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_eq_thunk, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_eq_thunk, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_eq_thunk, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_eq_thunk, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_eq_thunk, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_eq_thunk, _fp64  , double  )
            default             : GB_SEL_WORKER (_eq_thunk, _any   , GB_void )
        }
        break ;

    case GB_GT_THUNK_opcode   : // A(i,j) > thunk

        // bool: if thunk is false, renamed GxB_GT_THUNK to GxB_NONZERO
        //       if thunk is true,  return an empty matrix
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_gt_thunk, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_gt_thunk, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_gt_thunk, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_gt_thunk, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_gt_thunk, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_gt_thunk, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_gt_thunk, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_gt_thunk, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_gt_thunk, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_gt_thunk, _fp64  , double  )
            default: ;          // not for bool or user-defined ttypes
        }
        break ;

    case GB_GE_THUNK_opcode   : // A(i,j) >= thunk

        // bool: if thunk is false, use GB_dup
        //       if thunk is true,  renamed GxB_GE_THUNK to GxB_NONZERO
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_ge_thunk, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_ge_thunk, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_ge_thunk, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_ge_thunk, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_ge_thunk, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_ge_thunk, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_ge_thunk, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_ge_thunk, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_ge_thunk, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_ge_thunk, _fp64  , double  )
            default: ;          // not for bool or user-defined ttypes
        }
        break ;

    case GB_LT_THUNK_opcode   : // A(i,j) < thunk

        // bool: if thunk is true,  renamed GxB_LT_THUNK to GxB_EQ_ZERO
        //       if thunk is false, return an empty matrix
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_lt_thunk, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_lt_thunk, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_lt_thunk, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_lt_thunk, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_lt_thunk, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_lt_thunk, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_lt_thunk, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_lt_thunk, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_lt_thunk, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_lt_thunk, _fp64  , double  )
            default: ;          // not for bool or user-defined ttypes
        }
        break ;

    case GB_LE_THUNK_opcode   : // A(i,j) <= thunk

        // bool: if thunk is true,  use GB_dup
        //       if thunk is false, renamed GxB_LE_ZERO to GxB_EQ_ZERO
        // user type: return error
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_le_thunk, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_le_thunk, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_le_thunk, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_le_thunk, _int64 , int64_t )
            case GB_UINT8_code  : GB_SEL_WORKER (_le_thunk, _uint8 , uint8_t )
            case GB_UINT16_code : GB_SEL_WORKER (_le_thunk, _uint16, uint16_t)
            case GB_UINT32_code : GB_SEL_WORKER (_le_thunk, _uint32, uint32_t)
            case GB_UINT64_code : GB_SEL_WORKER (_le_thunk, _uint64, uint64_t)
            case GB_FP32_code   : GB_SEL_WORKER (_le_thunk, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_le_thunk, _fp64  , double  )
            default: ;          // not for bool or user-defined ttypes
        }
        break ;

    default:  ;
}


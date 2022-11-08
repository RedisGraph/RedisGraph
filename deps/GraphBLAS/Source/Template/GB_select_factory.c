//------------------------------------------------------------------------------
// GB_select_factory: switch factory for C=select(A,thunk)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

switch (opcode)
{

    case GB_TRIL_selop_code          :  // C = tril (A,k)

        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_tril, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_tril, _iso, GB_void)
            default              : GB_SEL_WORKER (_tril, _any, GB_void)
        }
        break ;
        #endif

    case GB_TRIU_selop_code          :  // C = triu (A,k)

        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_triu, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_triu, _iso, GB_void)
            default              : GB_SEL_WORKER (_triu, _any, GB_void)
        }
        break ;
        #endif

    case GB_DIAG_selop_code          :  // C = diag (A,k)

        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_diag, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_diag, _iso, GB_void)
            default              : GB_SEL_WORKER (_diag, _any, GB_void)
        }
        break ;
        #endif

    case GB_OFFDIAG_selop_code       :  // C = offdiag (A,k)

        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_offdiag, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_offdiag, _iso, GB_void)
            default              : GB_SEL_WORKER (_offdiag, _any, GB_void)
        }
        break ;
        #endif

    case GB_ROWINDEX_idxunop_code     :  // C = rowindex (A,k)

        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_rowindex, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_rowindex, _iso, GB_void)
            default              : GB_SEL_WORKER (_rowindex, _any, GB_void)
        }
        break ;
        #endif

    case GB_ROWLE_idxunop_code     :  // C = rowle (A,k)

        // also used for resize
        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_rowle, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_rowle, _iso, GB_void)
            default              : GB_SEL_WORKER (_rowle, _any, GB_void)
        }
        break ;
        #endif

    case GB_ROWGT_idxunop_code     :  // C = rowgt (A,k)

        #ifdef GB_SELECT_PHASE1
        GB_SEL_WORKER (_rowgt, _iso, GB_void)
        #else
        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_rowgt, _iso, GB_void)
            default              : GB_SEL_WORKER (_rowgt, _any, GB_void)
        }
        break ;
        #endif

    case GB_VALUEEQ_idxunop_code : // C = value_select (A,k)
    case GB_VALUENE_idxunop_code :
    case GB_VALUEGT_idxunop_code : 
    case GB_VALUEGE_idxunop_code : 
    case GB_VALUELT_idxunop_code : 
    case GB_VALUELE_idxunop_code : 

        // A is not iso, and typecasting is required, so use the
        // idxunop_function, just as if this were a user-defined operator.
        // Typecasting is costly; both the typecast and the idxunop function
        // are used via function pointers, so this is a generic method.
        #ifdef GB_SELECT_PHASE1
        GBURBLE ("(generic select) ") ;
        #endif
        ASSERT (op != NULL) ;
        ASSERT (op->ztype != NULL) ;
        ASSERT (op->xtype != NULL) ;
        ASSERT (op->ytype != NULL) ;
        GB_SEL_WORKER (_idxunop_cast, _any, GB_void)

    case GB_USER_idxunop_code   : // C = user_idxunop (A,k)

        ASSERT (op != NULL) ;
        ASSERT (op->ztype != NULL) ;
        ASSERT (op->xtype != NULL) ;
        ASSERT (op->ytype != NULL) ;
        if ((op->ztype != GrB_BOOL) ||
           ((typecode != GB_ignore_code) && (op->xtype != A->type)))
        {
            // typecasting is required
            #ifdef GB_SELECT_PHASE1
            GBURBLE ("(generic select) ") ;
            #endif
            switch (typecode)
            {
                case GB_ignore_code :   // A is iso
                    GB_SEL_WORKER (_idxunop_cast, _iso, GB_void)
                default             :   // A is non-iso
                    GB_SEL_WORKER (_idxunop_cast, _any, GB_void)
            }
        }
        else
        {
            // no typecasting
            switch (typecode)
            {
                case GB_ignore_code : GB_SEL_WORKER (_idxunop, _iso, GB_void)
                default             : GB_SEL_WORKER (_idxunop, _any, GB_void)
            }
        }
        break ;

    case GB_USER_selop_code     : // C = user_select (A,k)

        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_user, _iso, GB_void)
            default              : GB_SEL_WORKER (_user, _any, GB_void)
        }
        break ;

    //--------------------------------------------------------------------------
    // COL selectors are used only for the bitmap case
    //--------------------------------------------------------------------------

    #ifdef GB_BITMAP_SELECTOR

    case GB_COLINDEX_idxunop_code     :  // C = colindex (A,k)

        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_colindex, _iso, GB_void)
            default              : GB_SEL_WORKER (_colindex, _any, GB_void)
        }
        break ;

    case GB_COLLE_idxunop_code     :  // C = colle (A,k)

        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_colle, _iso, GB_void)
            default              : GB_SEL_WORKER (_colle, _any, GB_void)
        }
        break ;

    case GB_COLGT_idxunop_code     :  // C = colgt (A,k)

        switch (typecode)
        {
            case GB_ignore_code  : GB_SEL_WORKER (_colgt, _iso, GB_void)
            default              : GB_SEL_WORKER (_colgt, _any, GB_void)
        }
        break ;

    #endif

    //--------------------------------------------------------------------------
    // nonzombie selectors are not used for the bitmap case
    //--------------------------------------------------------------------------

    #ifndef GB_BITMAP_SELECTOR

    case GB_NONZOMBIE_selop_code     :  // C = all entries A(i,j) not a zombie

        #ifdef GB_SELECT_PHASE1
        // phase1: use a single worker for all types, since the test does not
        // depend on the values, just Ai.
        GB_SEL_WORKER (_nonzombie, _iso, GB_void)
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
            case GB_FC32_code   : GB_SEL_WORKER (_nonzombie, _fc32, GxB_FC32_t)
            case GB_FC64_code   : GB_SEL_WORKER (_nonzombie, _fc64, GxB_FC64_t)
            case GB_UDT_code    : GB_SEL_WORKER (_nonzombie, _any   , GB_void )
            case GB_ignore_code : GB_SEL_WORKER (_nonzombie, _iso   , GB_void )
            default: ;          // not used
        }
        break ;
        #endif

    #endif

    //--------------------------------------------------------------------------
    // none of these selectop workers are needed when A is iso
    //--------------------------------------------------------------------------

    case GB_NONZERO_selop_code   :  // A(i,j) != 0

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
            case GB_FC32_code   : GB_SEL_WORKER (_nonzero, _fc32, GxB_FC32_t)
            case GB_FC64_code   : GB_SEL_WORKER (_nonzero, _fc64, GxB_FC64_t)
            case GB_UDT_code    : GB_SEL_WORKER (_nonzero, _any   , GB_void )
            default: ;          // not used
        }
        break ;

    case GB_EQ_ZERO_selop_code   :  // A(i,j) == 0

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
            case GB_FC32_code   : GB_SEL_WORKER (_eq_zero, _fc32, GxB_FC32_t)
            case GB_FC64_code   : GB_SEL_WORKER (_eq_zero, _fc64, GxB_FC64_t)
            case GB_UDT_code    : GB_SEL_WORKER (_eq_zero, _any   , GB_void )
            default: ;          // not used
        }
        break ;

    case GB_GT_ZERO_selop_code   :  // A(i,j) > 0

        // bool and uint: renamed GxB_GT_ZERO to GxB_NONZERO
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_gt_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_gt_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_gt_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_gt_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_gt_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_gt_zero, _fp64  , double  )
            default: ;          // not used
        }
        break ;

    case GB_GE_ZERO_selop_code   :  // A(i,j) >= 0

        // bool and uint: always true; use GB_dup
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_ge_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_ge_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_ge_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_ge_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_ge_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_ge_zero, _fp64  , double  )
            default: ;          // not used
        }
        break ;

    case GB_LT_ZERO_selop_code   :  // A(i,j) < 0

        // bool and uint: always false; return an empty matrix
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_lt_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_lt_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_lt_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_lt_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_lt_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_lt_zero, _fp64  , double  )
            default: ;          // not used
        }
        break ;

    case GB_LE_ZERO_selop_code   :  // A(i,j) <= 0

        // bool and uint: renamed GxB_LE_ZERO to GxB_EQ_ZERO
        switch (typecode)
        {
            case GB_INT8_code   : GB_SEL_WORKER (_le_zero, _int8  , int8_t  )
            case GB_INT16_code  : GB_SEL_WORKER (_le_zero, _int16 , int16_t )
            case GB_INT32_code  : GB_SEL_WORKER (_le_zero, _int32 , int32_t )
            case GB_INT64_code  : GB_SEL_WORKER (_le_zero, _int64 , int64_t )
            case GB_FP32_code   : GB_SEL_WORKER (_le_zero, _fp32  , float   )
            case GB_FP64_code   : GB_SEL_WORKER (_le_zero, _fp64  , double  )
            default: ;          // not used
        }
        break ;

    case GB_NE_THUNK_selop_code   : // A(i,j) != thunk

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
            case GB_FC32_code   : GB_SEL_WORKER (_ne_thunk, _fc32, GxB_FC32_t)
            case GB_FC64_code   : GB_SEL_WORKER (_ne_thunk, _fc64, GxB_FC64_t)
            case GB_UDT_code    : GB_SEL_WORKER (_ne_thunk, _any   , GB_void )
            default: ;          // not used
        }
        break ;

    case GB_EQ_THUNK_selop_code   : // A(i,j) == thunk

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
            case GB_FC32_code   : GB_SEL_WORKER (_eq_thunk, _fc32, GxB_FC32_t)
            case GB_FC64_code   : GB_SEL_WORKER (_eq_thunk, _fc64, GxB_FC64_t)
            case GB_UDT_code    : GB_SEL_WORKER (_eq_thunk, _any   , GB_void )
            default: ;          // not used
        }
        break ;

    case GB_GT_THUNK_selop_code   : // A(i,j) > thunk

        // bool: if thunk is false, renamed GxB_GT_THUNK to GxB_NONZERO
        //       if thunk is true,  return an empty matrix
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
            default: ;          // not used
        }
        break ;

    case GB_GE_THUNK_selop_code   : // A(i,j) >= thunk

        // bool: if thunk is false, use GB_dup
        //       if thunk is true,  renamed GxB_GE_THUNK to GxB_NONZERO
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
            default: ;          // not used
        }
        break ;

    case GB_LT_THUNK_selop_code   : // A(i,j) < thunk

        // bool: if thunk is true,  renamed GxB_LT_THUNK to GxB_EQ_ZERO
        //       if thunk is false, return an empty matrix
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
            default: ;          // not used
        }
        break ;

    case GB_LE_THUNK_selop_code   : // A(i,j) <= thunk

        // bool: if thunk is true,  use GB_dup
        //       if thunk is false, renamed GxB_LE_ZERO to GxB_EQ_ZERO
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
            default: ;          // not used
        }
        break ;

    default: ;
}


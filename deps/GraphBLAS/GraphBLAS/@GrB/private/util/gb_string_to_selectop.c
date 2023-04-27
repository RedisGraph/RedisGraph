//------------------------------------------------------------------------------
// gb_string_to_selectop: get an index_unop or selectop from a string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

#include "gb_interface.h"

// GrB_IndexUnaryOp and GxB_SelectOp operators, with their equivalent aliases

void gb_string_to_selectop
(
    // outputs: one of the outputs is non-NULL and the other NULL
    GrB_IndexUnaryOp *idxunop,          // GrB_IndexUnaryOp, if found
    GxB_SelectOp *selop,                // GxB_SelectOp if found
    bool *thunk_required,               // true if op requires a thunk scalar
    bool *op_is_positional,             // true if op is positional
    // input/output:
    int64_t *ithunk,
    // inputs:
    char *opstring,                     // string defining the operator
    const GrB_Type atype                // type of A, or NULL if not present
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (opstring == NULL || opstring [0] == '\0', "invalid selectop") ;

    //--------------------------------------------------------------------------
    // get the opstring and parse it
    //--------------------------------------------------------------------------

    int32_t position [2] ;
    gb_find_dot (position, opstring) ;

    char *op_name = opstring ;
    char *op_typename = NULL ;
    if (position [0] >= 0)
    { 
        opstring [position [0]] = '\0' ;
        op_typename = opstring + position [0] + 1 ;
    }

    //--------------------------------------------------------------------------
    // get the operator type for VALUE* operators
    //--------------------------------------------------------------------------

    GrB_Type type ;
    if (op_typename == NULL)
    { 
        // no type in the opstring; select the type from A
        type = atype ;
    }
    else
    { 
        // type is explicitly present in the opstring
        type = gb_string_to_type (op_typename) ;
    }

    // type may still be NULL, which is OK; GxB_SelectOps are untyped.
    GB_Type_code typecode = (type == NULL) ? GB_ignore_code : type->code ;

    //--------------------------------------------------------------------------
    // convert the string to a GrB_IndexUnaryOp or GxB_SelectOp
    //--------------------------------------------------------------------------

    (*idxunop) = NULL ;
    (*selop) = NULL ;
    (*thunk_required) = true ;
    (*op_is_positional) = false ;

    if (MATCH (opstring, "tril"))
    { 
        (*idxunop) = GrB_TRIL ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "triu"))
    { 
        (*idxunop) = GrB_TRIU ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "diag"))
    { 
        (*idxunop) = GrB_DIAG ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "offdiag"))
    { 
        (*idxunop) = GrB_OFFDIAG ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "rowne"))
    { 
        (*idxunop) = GrB_ROWINDEX_INT64 ;
        (*ithunk) = - (*ithunk - 1) ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "rowle"))
    { 
        (*idxunop) = GrB_ROWLE ;
        (*ithunk)-- ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "rowgt"))
    { 
        (*idxunop) = GrB_ROWGT ;
        (*ithunk)-- ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "colne"))
    { 
        (*idxunop) = GrB_COLINDEX_INT64 ;
        (*ithunk) = - (*ithunk - 1) ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "colle"))
    { 
        (*idxunop) = GrB_COLLE ;
        (*ithunk)-- ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "colgt"))
    { 
        (*idxunop) = GrB_COLGT ;
        (*ithunk)-- ;
        (*op_is_positional) = true ;
    }
    else if (MATCH (opstring, "nonzero") || MATCH (opstring, "~=0"))
    { 
        (*selop) = (GxB_NONZERO) ;
        (*thunk_required) = false ;
    }
    else if (MATCH (opstring, "zero") || MATCH (opstring, "==0"))
    { 
        (*selop) = (GxB_EQ_ZERO) ;
        (*thunk_required) = false ;
    }
    else if (MATCH (opstring, "positive") || MATCH (opstring, ">0"))
    { 
        (*selop) = (GxB_GT_ZERO) ;
        (*thunk_required) = false ;
    }
    else if (MATCH (opstring, "nonnegative") || MATCH (opstring, ">=0"))
    { 
        (*selop) = (GxB_GE_ZERO) ;
        (*thunk_required) = false ;
    }
    else if (MATCH (opstring, "negative") || MATCH (opstring, "<0"))
    { 
        (*selop) = (GxB_LT_ZERO) ;
        (*thunk_required) = false ;
    }
    else if (MATCH (opstring, "nonpositive") || MATCH (opstring, "<=0"))
    { 
        (*selop) = (GxB_LE_ZERO) ;
        (*thunk_required) = false ;
    }
    else if (MATCH (opstring, "~="))
    { 
        switch (typecode)
        {
            case GB_BOOL_code   : (*idxunop) = GrB_VALUENE_BOOL    ; break ;
            case GB_INT8_code   : (*idxunop) = GrB_VALUENE_INT8    ; break ;
            case GB_INT16_code  : (*idxunop) = GrB_VALUENE_INT16   ; break ;
            case GB_INT32_code  : (*idxunop) = GrB_VALUENE_INT32   ; break ;
            case GB_INT64_code  : (*idxunop) = GrB_VALUENE_INT64   ; break ;
            case GB_UINT8_code  : (*idxunop) = GrB_VALUENE_UINT8   ; break ;
            case GB_UINT16_code : (*idxunop) = GrB_VALUENE_UINT16  ; break ;
            case GB_UINT32_code : (*idxunop) = GrB_VALUENE_UINT32  ; break ;
            case GB_UINT64_code : (*idxunop) = GrB_VALUENE_UINT64  ; break ;
            case GB_FP32_code   : (*idxunop) = GrB_VALUENE_FP32    ; break ;
            case GB_FP64_code   : (*idxunop) = GrB_VALUENE_FP64    ; break ;
            case GB_FC32_code   : (*idxunop) = GxB_VALUENE_FC32    ; break ;
            case GB_FC64_code   : (*idxunop) = GxB_VALUENE_FC64    ; break ;
            default             : (*selop  ) = GxB_NE_THUNK        ; break ;
        }
    }
    else if (MATCH (opstring, "=="))
    { 
        switch (typecode)
        {
            case GB_BOOL_code   : (*idxunop) = GrB_VALUEEQ_BOOL    ; break ;
            case GB_INT8_code   : (*idxunop) = GrB_VALUEEQ_INT8    ; break ;
            case GB_INT16_code  : (*idxunop) = GrB_VALUEEQ_INT16   ; break ;
            case GB_INT32_code  : (*idxunop) = GrB_VALUEEQ_INT32   ; break ;
            case GB_INT64_code  : (*idxunop) = GrB_VALUEEQ_INT64   ; break ;
            case GB_UINT8_code  : (*idxunop) = GrB_VALUEEQ_UINT8   ; break ;
            case GB_UINT16_code : (*idxunop) = GrB_VALUEEQ_UINT16  ; break ;
            case GB_UINT32_code : (*idxunop) = GrB_VALUEEQ_UINT32  ; break ;
            case GB_UINT64_code : (*idxunop) = GrB_VALUEEQ_UINT64  ; break ;
            case GB_FP32_code   : (*idxunop) = GrB_VALUEEQ_FP32    ; break ;
            case GB_FP64_code   : (*idxunop) = GrB_VALUEEQ_FP64    ; break ;
            case GB_FC32_code   : (*idxunop) = GxB_VALUEEQ_FC32    ; break ;
            case GB_FC64_code   : (*idxunop) = GxB_VALUEEQ_FC64    ; break ;
            default             : (*selop  ) = GxB_EQ_THUNK        ; break ;
        }
    }
    else if (MATCH (opstring, ">"))
    { 
        switch (typecode)
        {
            case GB_BOOL_code   : (*idxunop) = GrB_VALUEGT_BOOL    ; break ;
            case GB_INT8_code   : (*idxunop) = GrB_VALUEGT_INT8    ; break ;
            case GB_INT16_code  : (*idxunop) = GrB_VALUEGT_INT16   ; break ;
            case GB_INT32_code  : (*idxunop) = GrB_VALUEGT_INT32   ; break ;
            case GB_INT64_code  : (*idxunop) = GrB_VALUEGT_INT64   ; break ;
            case GB_UINT8_code  : (*idxunop) = GrB_VALUEGT_UINT8   ; break ;
            case GB_UINT16_code : (*idxunop) = GrB_VALUEGT_UINT16  ; break ;
            case GB_UINT32_code : (*idxunop) = GrB_VALUEGT_UINT32  ; break ;
            case GB_UINT64_code : (*idxunop) = GrB_VALUEGT_UINT64  ; break ;
            case GB_FP32_code   : (*idxunop) = GrB_VALUEGT_FP32    ; break ;
            case GB_FP64_code   : (*idxunop) = GrB_VALUEGT_FP64    ; break ;
            default             : (*selop  ) = GxB_GT_THUNK        ; break ;
        }
    }
    else if (MATCH (opstring, ">="))
    { 
        switch (typecode)
        {
            case GB_BOOL_code   : (*idxunop) = GrB_VALUEGE_BOOL    ; break ;
            case GB_INT8_code   : (*idxunop) = GrB_VALUEGE_INT8    ; break ;
            case GB_INT16_code  : (*idxunop) = GrB_VALUEGE_INT16   ; break ;
            case GB_INT32_code  : (*idxunop) = GrB_VALUEGE_INT32   ; break ;
            case GB_INT64_code  : (*idxunop) = GrB_VALUEGE_INT64   ; break ;
            case GB_UINT8_code  : (*idxunop) = GrB_VALUEGE_UINT8   ; break ;
            case GB_UINT16_code : (*idxunop) = GrB_VALUEGE_UINT16  ; break ;
            case GB_UINT32_code : (*idxunop) = GrB_VALUEGE_UINT32  ; break ;
            case GB_UINT64_code : (*idxunop) = GrB_VALUEGE_UINT64  ; break ;
            case GB_FP32_code   : (*idxunop) = GrB_VALUEGE_FP32    ; break ;
            case GB_FP64_code   : (*idxunop) = GrB_VALUEGE_FP64    ; break ;
            default             : (*selop  ) = GxB_GE_THUNK        ; break ;
        }
    }
    else if (MATCH (opstring, "<"))
    { 
        switch (typecode)
        {
            case GB_BOOL_code   : (*idxunop) = GrB_VALUELT_BOOL    ; break ;
            case GB_INT8_code   : (*idxunop) = GrB_VALUELT_INT8    ; break ;
            case GB_INT16_code  : (*idxunop) = GrB_VALUELT_INT16   ; break ;
            case GB_INT32_code  : (*idxunop) = GrB_VALUELT_INT32   ; break ;
            case GB_INT64_code  : (*idxunop) = GrB_VALUELT_INT64   ; break ;
            case GB_UINT8_code  : (*idxunop) = GrB_VALUELT_UINT8   ; break ;
            case GB_UINT16_code : (*idxunop) = GrB_VALUELT_UINT16  ; break ;
            case GB_UINT32_code : (*idxunop) = GrB_VALUELT_UINT32  ; break ;
            case GB_UINT64_code : (*idxunop) = GrB_VALUELT_UINT64  ; break ;
            case GB_FP32_code   : (*idxunop) = GrB_VALUELT_FP32    ; break ;
            case GB_FP64_code   : (*idxunop) = GrB_VALUELT_FP64    ; break ;
            default             : (*selop  ) = GxB_LT_THUNK        ; break ;
        }
    }
    else if (MATCH (opstring, "<="))
    { 
        switch (typecode)
        {
            case GB_BOOL_code   : (*idxunop) = GrB_VALUELE_BOOL    ; break ;
            case GB_INT8_code   : (*idxunop) = GrB_VALUELE_INT8    ; break ;
            case GB_INT16_code  : (*idxunop) = GrB_VALUELE_INT16   ; break ;
            case GB_INT32_code  : (*idxunop) = GrB_VALUELE_INT32   ; break ;
            case GB_INT64_code  : (*idxunop) = GrB_VALUELE_INT64   ; break ;
            case GB_UINT8_code  : (*idxunop) = GrB_VALUELE_UINT8   ; break ;
            case GB_UINT16_code : (*idxunop) = GrB_VALUELE_UINT16  ; break ;
            case GB_UINT32_code : (*idxunop) = GrB_VALUELE_UINT32  ; break ;
            case GB_UINT64_code : (*idxunop) = GrB_VALUELE_UINT64  ; break ;
            case GB_FP32_code   : (*idxunop) = GrB_VALUELE_FP32    ; break ;
            case GB_FP64_code   : (*idxunop) = GrB_VALUELE_FP64    ; break ;
            default             : (*selop  ) = GxB_LE_THUNK        ; break ;
        }
    }

    if ((*idxunop) == NULL && (*selop) == NULL)
    { 
        ERROR2 ("idxunop/selectop unknown: %s\n", opstring) ;
    }
}


//------------------------------------------------------------------------------
// GB_stringify_binop: convert a binary op into a string or enum
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The binop macro generates an expression, not a full statement.  There
// is no semicolon or assignment.  For example:

// #define GB_MULT(x,y) ((x) * (y))

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_stringify_binop: construct the binop macro
//------------------------------------------------------------------------------

void GB_stringify_binop
(
    // input:
    FILE *fp,                 // File to write macros, assumed open already
    const char *macro_name,   // name of macro to construct
    GB_Opcode opcode,   // opcode of GraphBLAS operator to convert into a macro
    GB_Type_code xcode, // op->xtype->code of the operator
    bool for_semiring,  // if true: op is a multiplier in a semiring
    bool flipxy         // if true, use mult(y,x) else mult(x,y)
)
{

    const char *op_string ;
    int ecode ;

    // get ecode from opcode, xcode, and for_semiring
    GB_enumify_binop (&ecode, opcode, xcode, for_semiring) ;

    printf("ecode in stringify binop: %d\n", ecode);

    // convert ecode to string
    GB_charify_binop (&op_string, ecode) ;

    // convert string to macro
    GB_macrofy_binop ( fp, macro_name, op_string, flipxy) ;
}

//------------------------------------------------------------------------------
// GB_enumify_binop: convert binary opcode and xcode into a single enum
//------------------------------------------------------------------------------

// ecodes 0 to 31 can be used as a monoid, but only 0:22 are currently in use.
// ecodes 32 and up are not valid for use in a monoid; only 32:139 are in use.

void GB_enumify_binop
(
    // output:
    int *ecode,         // enumerated operator, range 0 to 139; -1 on failure
    // input:
    GB_Opcode opcode,   // opcode of GraphBLAS operator to convert into a macro
    GB_Type_code xcode, // op->xtype->code of the operator
    bool for_semiring   // true for A*B, false for A+B or A.*B
)
{

    int e = -1 ;

    switch (opcode)
    {

        //----------------------------------------------------------------------
        // user-defined operator
        //----------------------------------------------------------------------

        case GB_USER_binop_code :

            e = 0 ; break ;

        //----------------------------------------------------------------------
        // built-in ops that can be used in a monoid:
        //----------------------------------------------------------------------

        case GB_FIRST_binop_code :  // z = x, can be used as the ANY monoid

            e = 1 ; break ;

        case GB_ANY_binop_code :    // z = y (same as SECOND)

            e = 2 ; break ;

        case GB_MIN_binop_code :    // z = min(x,y)

            switch (xcode)
            {
                case GB_BOOL_code   : e = 18 ; break ; // x && y
                case GB_FP32_code   : e =  3 ; break ; // fminf (x,y)
                case GB_FP64_code   : e =  4 ; break ; // fmin (x,y)
                case GB_FC32_code   : e = -1 ; break ; // invalid
                case GB_FC64_code   : e = -1 ; break ; // invalid
                default             : e =  5 ; break ; // GB_IMIN (x,y)
            }
            break ;

        case GB_MAX_binop_code :    // z = max(x,y)

            printf("INside max binop code\n");
            switch (xcode)
            {
                case GB_BOOL_code   : e = 17 ; break ; // x || y
                case GB_FP32_code   : e =  6 ; break ; // fmaxf (x,y)
                case GB_FP64_code   : e =  7 ; break ; // fmax (x,y)
                case GB_FC32_code   : e = -1 ; break ; // invalid
                case GB_FC64_code   : e = -1 ; break ; // invalid
                default             : e =  8 ; break ; // GB_IMAX (x,y)
            }
            break ;

        case GB_PLUS_binop_code :   // z = x + y

            printf("Inside plus binop code\n");

            switch (xcode)
            {
                case GB_BOOL_code   : e = 17 ; break ; // x || y
                case GB_FC32_code   : e =  9 ; break ; // GB_FC32_add(x,y)
                case GB_FC64_code   : e = 10 ; break ; // GB_FC64_add(x,y)
                default             : e = 11 ; break ; // x + y
            }

            break ;

        case GB_TIMES_binop_code :  // z = x * y

            switch (xcode)
            {
                case GB_BOOL_code   : e = 18 ; break ; // x && y
                case GB_FC32_code   : e = 12 ; break ; // GB_FC32_mul(x,y)
                case GB_FC64_code   : e = 13 ; break ; // GB_FC64_mul(x,y)
                default             : e = 14 ; break ; // x * y
            }
            break ;

        case GB_EQ_binop_code :     // z = (x == y), the LXNOR monoid for bool

            // only a monoid for bool (lxnor)
            switch (xcode)
            {
                case GB_BOOL_code   : e = 15 ; break ; // x == y
                case GB_FC32_code   : e = 32 ; break ; // GB_FC32_eq(x,y)
                case GB_FC64_code   : e = 33 ; break ; // GB_FC64_eq(x,y)
                default             : e = 15 ; break ; // x == y, not a monoid
            }
            break ;

        case GB_NE_binop_code :     // z = (x != y), the LXOR monoid for bool

            // only a monoid for bool (lxor)
            switch (xcode)
            {
                case GB_BOOL_code   : e = 16 ; break ; // x != y
                case GB_FC32_code   : e = 36 ; break ; // GB_FC32_ne(x,y)
                case GB_FC64_code   : e = 37 ; break ; // GB_FC64_ne(x,y)
                default             : e = 16 ; break ; // x != y
            }
            break ;

        case GB_LOR_binop_code :    // z = (x || y)

            switch (xcode)
            {
                case GB_BOOL_code  : e = 17 ; break ; // x || y
                default            : e = 40 ; break ; // not a monoid
            }
            break ;

        case GB_LAND_binop_code :   // z = (x && y)

            switch (xcode)
            {
                case GB_BOOL_code   : e = 18 ; break ; // x && y
                default             : e = 41 ; break ; // not a monoid
            }
            break ;

        case GB_LXOR_binop_code :   // z = (x != y)

            switch (xcode)
            {
                case GB_BOOL_code   : e = 16 ; break ; // x != y
                default             : e = 42 ; break ; // not a monoid
            }
            break ;

        case GB_BOR_binop_code :    // z = (x | y), bitwise or

            if (xcode >= GB_INT8_code && xcode <= GB_UINT64_code) e = 19 ;
            break ;

        case GB_BAND_binop_code :   // z = (x & y), bitwise and

            if (xcode >= GB_INT8_code && xcode <= GB_UINT64_code) e = 20 ;
            break ;

        case GB_BXOR_binop_code :   // z = (x ^ y), bitwise xor

            if (xcode >= GB_INT8_code && xcode <= GB_UINT64_code) e = 21 ;
            break ;

        case GB_BXNOR_binop_code :  // z = ~(x ^ y), bitwise xnor

            if (xcode >= GB_INT8_code && xcode <= GB_UINT64_code) e = 22 ;
            break ;

        //----------------------------------------------------------------------
        // built-in ops that cannot be used in a monoid:
        //----------------------------------------------------------------------

        case GB_SECOND_binop_code : // z = y (same as ANY, but not a monoid)

            e = 2 ; break ;

        case GB_ISEQ_binop_code :   // z = (x == y), but not a monoid

            switch (xcode)
            {
                case GB_BOOL_code   : e = 15 ; break ; // x == y
                case GB_FC32_code   : e = 34 ; break ; // GB_FC32_iseq(x,y)
                case GB_FC64_code   : e = 35 ; break ; // GB_FC64_iseq(x,y)
                default             : e = 15 ; break ; // x == y, not a monoid
            }
            break ;

        case GB_ISNE_binop_code :   // z = (x != y), but not a monoid

            switch (xcode)
            {
                case GB_BOOL_code   : e = 16 ; break ; // x != y
                case GB_FC32_code   : e = 38 ; break ; // GB_FC32_isne(x,y)
                case GB_FC64_code   : e = 39 ; break ; // GB_FC64_isne(x,y)
                default             : e = 16 ; break ; // x != y
            }
            break ;

        case GB_MINUS_binop_code :  // z = x - y

            switch (xcode)
            {
                case GB_BOOL_code   : e = 16 ; break ; // x != y
                case GB_FC32_code   : e = 43 ; break ; // GB_FC32_minus(x,y)
                case GB_FC64_code   : e = 44 ; break ; // GB_FC64_minus(x,y)
                default             : e = 45 ; break ; // x - y
            }
            break ;

        case GB_RMINUS_binop_code : // z = y - x

            switch (xcode)
            {
                case GB_BOOL_code   : e = 16 ; break ; // x != y
                case GB_FC32_code   : e = 46 ; break ; // GB_FC32_minus(y,x)
                case GB_FC64_code   : e = 47 ; break ; // GB_FC64_minus(y,x)
                default             : e = 48 ; break ; // y - x
            }
            break ;

        case GB_DIV_binop_code :    // z = x / y ;

            switch (xcode)
            {
                case GB_BOOL_code   : e =  1 ; break ; // x
                case GB_INT8_code   : e = 49 ; break ; // GB_IDIV_SIGNED(x,y,8)
                case GB_INT16_code  : e = 50 ; break ; // GB_IDIV_SIGNED(x,y,16)
                case GB_INT32_code  : e = 51 ; break ; // GB_IDIV_SIGNED(x,y,32)
                case GB_INT64_code  : e = 52 ; break ; // GB_IDIV_SIGNED(x,y,64)
                case GB_UINT8_code  : e = 53 ; break ; // GB_IDIV_UN...(x,y,8)
                case GB_UINT16_code : e = 54 ; break ; // GB_IDIV_UN...(x,y,16)
                case GB_UINT32_code : e = 55 ; break ; // GB_IDIV_UN...(x,y,32)
                case GB_UINT64_code : e = 56 ; break ; // GB_IDIV_UN...(x,y,64)
                case GB_FC32_code   : e = 57 ; break ; // GB_FC32_div(x,y)
                case GB_FC64_code   : e = 58 ; break ; // GB_FC64_div(x,y)
                default             : e = 59 ; break ; // (x) / (y)
            }
            break ;

        case GB_RDIV_binop_code :   // z = y / x ;

            switch (xcode)
            {
                case GB_BOOL_code   : e =  2 ; break ; // y
                case GB_INT8_code   : e = 60 ; break ; // GB_IDIV_SIGNED(y,x,8)
                case GB_INT16_code  : e = 61 ; break ; // GB_IDIV_SIGNED(y,x,16)
                case GB_INT32_code  : e = 62 ; break ; // GB_IDIV_SIGNED(y,x,32)
                case GB_INT64_code  : e = 63 ; break ; // GB_IDIV_SIGNED(y,x,64)
                case GB_UINT8_code  : e = 64 ; break ; // GB_IDIV_UN...(y,x,8)
                case GB_UINT16_code : e = 65 ; break ; // GB_IDIV_UN...(y,x,16)
                case GB_UINT32_code : e = 66 ; break ; // GB_IDIV_UN...(y,x,32)
                case GB_UINT64_code : e = 67 ; break ; // GB_IDIV_UN...(y,x,64)
                case GB_FC32_code   : e = 68 ; break ; // GB_FC32_div(y,x)
                case GB_FC64_code   : e = 69 ; break ; // GB_FC64_div(y,x)
                default             : e = 70 ; break ; // (y) / (x)
            }
            break ;

        case GB_GT_binop_code :
        case GB_ISGT_binop_code :   // z = (x > y)

            e = 71 ; break ;

        case GB_LT_binop_code :
        case GB_ISLT_binop_code :   // z = (x < y)

            e = 72 ; break ;

        case GB_GE_binop_code :
        case GB_ISGE_binop_code :   // z = (x >= y)

            e = 73 ; break ;

        case GB_LE_binop_code :
        case GB_ISLE_binop_code :   // z = (x <= y)

            e = 74 ; break ;

        case GB_BGET_binop_code :   // z = bitget (x,y)

            switch (xcode)
            {
                case GB_INT8_code   : e = 75 ; break ;
                case GB_INT16_code  : e = 76 ; break ;
                case GB_INT32_code  : e = 77 ; break ;
                case GB_INT64_code  : e = 78 ; break ;
                case GB_UINT8_code  : e = 79 ; break ;
                case GB_UINT16_code : e = 80 ; break ;
                case GB_UINT32_code : e = 81 ; break ;
                case GB_UINT64_code : e = 82 ; break ;
                default             : e = -1 ; break ;
            }
            break ;

        case GB_BSET_binop_code :   // z = bitset (x,y)

            switch (xcode)
            {
                case GB_INT8_code   : e = 83 ; break ;
                case GB_INT16_code  : e = 84 ; break ;
                case GB_INT32_code  : e = 85 ; break ;
                case GB_INT64_code  : e = 86 ; break ;
                case GB_UINT8_code  : e = 87 ; break ;
                case GB_UINT16_code : e = 88 ; break ;
                case GB_UINT32_code : e = 89 ; break ;
                case GB_UINT64_code : e = 90 ; break ;
                default             : e = -1 ; break ;
            }
            break ;

        case GB_BCLR_binop_code :   // z = bitclr (x,y)

            switch (xcode)
            {
                case GB_INT8_code   : e = 91 ; break ;
                case GB_INT16_code  : e = 92 ; break ;
                case GB_INT32_code  : e = 93 ; break ;
                case GB_INT64_code  : e = 94 ; break ;
                case GB_UINT8_code  : e = 95 ; break ;
                case GB_UINT16_code : e = 96 ; break ;
                case GB_UINT32_code : e = 97 ; break ;
                case GB_UINT64_code : e = 98 ; break ;
                default             : e = -1 ; break ;
            }
            break ;

        case GB_BSHIFT_binop_code : // z = bitshift (x,y)

            switch (xcode)
            {
                case GB_INT8_code   : e =  99 ; break ;
                case GB_INT16_code  : e = 100 ; break ;
                case GB_INT32_code  : e = 101 ; break ;
                case GB_INT64_code  : e = 102 ; break ;
                case GB_UINT8_code  : e = 103 ; break ;
                case GB_UINT16_code : e = 104 ; break ;
                case GB_UINT32_code : e = 105 ; break ;
                case GB_UINT64_code : e = 106 ; break ;
                default             : e = -1 ; break ;
            }
            break ;

        case GB_POW_binop_code :    // z = pow (x,y)

            switch (xcode)
            {
                case GB_BOOL_code   : e =  71 ; break ; // x >= y
                case GB_INT8_code   : e = 107 ; break ;
                case GB_INT16_code  : e = 108 ; break ;
                case GB_INT32_code  : e = 109 ; break ;
                case GB_INT64_code  : e = 110 ; break ;
                case GB_UINT8_code  : e = 111 ; break ;
                case GB_UINT16_code : e = 112 ; break ;
                case GB_UINT32_code : e = 113 ; break ;
                case GB_UINT64_code : e = 114 ; break ;
                case GB_FP32_code   : e = 115 ; break ;
                case GB_FP64_code   : e = 116 ; break ;
                case GB_FC32_code   : e = 117 ; break ;
                case GB_FC64_code   : e = 118 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_ATAN2_binop_code :  // z = atan2 (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 119 ; break ;
                case GB_FP64_code   : e = 120 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_HYPOT_binop_code :  // z = hypot (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 121 ; break ;
                case GB_FP64_code   : e = 122 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_FMOD_binop_code :   // z = fmod (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 123 ; break ;
                case GB_FP64_code   : e = 124 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_REMAINDER_binop_code :  // z = remainder (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 125 ; break ;
                case GB_FP64_code   : e = 126 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_COPYSIGN_binop_code :   // z = copysign (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 127 ; break ;
                case GB_FP64_code   : e = 128 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_LDEXP_binop_code :  // z = ldexp (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 129 ; break ;
                case GB_FP64_code   : e = 130 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_CMPLX_binop_code :  // z = cmplx (x,y)

            switch (xcode)
            {
                case GB_FP32_code   : e = 131 ; break ;
                case GB_FP64_code   : e = 132 ; break ;
                default             : e =  -1 ; break ;
            }
            break ;

        case GB_PAIR_binop_code :   // z = 1

            e = 133 ; break ;

        //----------------------------------------------------------------------
        // positional ops
        //----------------------------------------------------------------------

        case GB_FIRSTI_binop_code :     // z = i

            e = 134 ; break ;

        case GB_FIRSTI1_binop_code :    // z = i+1

            e = 137 ; break ;

        case GB_FIRSTJ_binop_code :     // z = for_semiring ? (k) : (j)

            e = for_semiring ? 135 : 136 ; break ;

        case GB_FIRSTJ1_binop_code :    // z = for_semiring ? (k+1) : (j+1)

            e = for_semiring ? 138 : 139 ; break ;

        case GB_SECONDI_binop_code :    // z = for_semiring ? (k) : (i)

            e = for_semiring ? 135 : 134 ; break ;

        case GB_SECONDI1_binop_code :   // z = for_semiring ? (k+1) : (i+1)

            e = for_semiring ? 138 : 139 ; break ;

        case GB_SECONDJ_binop_code :    // z = j

            e = 136 ; break ;

        case GB_SECONDJ1_binop_code :   // z = j+1

            e = 139 ; break ;

        default : break ;
    }

    printf("e %d\n", e);
    (*ecode) = e ;
}

//------------------------------------------------------------------------------
// GB_charify_binop: convert an ecode into a string
//------------------------------------------------------------------------------

void GB_charify_binop
(
    // output:
    const char **op_string,   // string defining the operator (NULL if failure)
    // input:
    int ecode           // from GB_enumify_binop
)
{

    const char *f ;

    printf("ecode in charify binop: %d\n", ecode);

    switch (ecode)
    {

        //----------------------------------------------------------------------
        // user-defined ops
        //----------------------------------------------------------------------

        // f must be defined by a string provided by the user.  This is
        // only a place-holder

        case   0 : f = "user-defined"               ; break ;

        //----------------------------------------------------------------------
        // built-in ops, can be used in a monoid
        //----------------------------------------------------------------------

        // first
        case   1 : f = "x"                          ; break ;

        // any, second
        case   2 : f = "y"                          ; break ;

        // min
        case   3 : f = "fminf (x,y)"                ; break ;
        case   4 : f = "fmin (x,y)"                 ; break ;
        case   5 : f = "GB_IMIN (x,y)"              ; break ;

        // max
        case   6 : f = "fmaxf (x,y)"                ; break ;
        case   7 : f = "fmax (x,y)"                 ; break ;
        case   8 : f = "GB_IMAX (x,y)"              ; break ;

        // plus
        case   9 : f = "GB_FC32_add(x,y)"           ; break ;
        case  10 : f = "GB_FC64_add(x,y)"           ; break ;
        case  11 : f = "(x) + (y)"                  ; break ;

        // times
        case  12 : f = "GB_FC32_mul(x,y)"           ; break ;
        case  13 : f = "GB_FC64_mul(x,y)"           ; break ;
        case  14 : f = "(x) * (y)"                  ; break ;

        // eq, iseq, lxnor
        case  15 : f = "(x) == (y)"                 ; break ;

        // ne, isne, lxor
        case  16 : f = "(x) != (y)"                 ; break ;

        // lor
        case  17 : f = "(x) || (y)"                 ; break ;

        // land
        case  18 : f = "(x) && (y)"                 ; break ;

        // bor
        case  19 : f = "(x) | (y)"                  ; break ;

        // band
        case  20 : f = "(x) & (y)"                  ; break ;

        // bxor
        case  21 : f = "(x) ^ (y)"                  ; break ;

        // bxnor
        case  22 : f = "~((x) ^ (y))"               ; break ;

        // 23 to 31 are unused, but reserved for future monoids

        //----------------------------------------------------------------------
        // built-in ops, cannot be used in a monoid
        //----------------------------------------------------------------------

        // eq for complex
        case  32 : f = "GB_FC32_eq(x,y)"            ; break ;
        case  33 : f = "GB_FC64_eq(x,y)"            ; break ;

        // iseq for complex
        case  34 : f = "GB_FC32_iseq(x,y)"          ; break ;
        case  35 : f = "GB_FC64_iseq(x,y)"          ; break ;

        // ne for complex
        case  36 : f = "GB_FC32_ne(x,y)"            ; break ;
        case  37 : f = "GB_FC64_ne(x,y)"            ; break ;

        // isne for complex
        case  38 : f = "GB_FC32_isne(x,y)"          ; break ;
        case  39 : f = "GB_FC64_isne(x,y)"          ; break ;

        // lor for non-boolean
        case  40 : f = "((x)!=0) || ((y)!=0)"       ; break ;

        // land for non-boolean
        case  41 : f = "((x)!=0) && ((y)!=0)"       ; break ;

        // lxor for non-boolean
        case  42 : f = "((x)!=0) != ((y)!=0)"       ; break ;

        // minus
        case  43 : f = "GB_FC32_minus(x,y)"         ; break ;
        case  44 : f = "GB_FC64_minus(x,y)"         ; break ;
        case  45 : f = "(x) - (y)"                  ; break ;

        // rminus
        case  46 : f = "GB_FC32_minus(y,x)"         ; break ;
        case  47 : f = "GB_FC64_minus(y,x)"         ; break ;
        case  48 : f = "(y) - (x)"                  ; break ;

        // div
        case  49 : f = "GB_IDIV_SIGNED(x,y,8)"      ; break ;
        case  50 : f = "GB_IDIV_SIGNED(x,y,16)"     ; break ;
        case  51 : f = "GB_IDIV_SIGNED(x,y,32)"     ; break ;
        case  52 : f = "GB_IDIV_SIGNED(x,y,64)"     ; break ;
        case  53 : f = "GB_IDIV_UNSIGNED(x,y,8)"    ; break ;
        case  54 : f = "GB_IDIV_UNSIGNED(x,y,16)"   ; break ;
        case  55 : f = "GB_IDIV_UNSIGNED(x,y,32)"   ; break ;
        case  56 : f = "GB_IDIV_UNSIGNED(x,y,64)"   ; break ;
        case  57 : f = "GB_FC32_div(x,y)"           ; break ;
        case  58 : f = "GB_FC64_div(x,y)"           ; break ;
        case  59 : f = "(x) / (y)"                  ; break ;

        // rdiv
        case  60 : f = "GB_IDIV_SIGNED(y,x,8)"      ; break ;
        case  61 : f = "GB_IDIV_SIGNED(y,x,16)"     ; break ;
        case  62 : f = "GB_IDIV_SIGNED(y,x,32)"     ; break ;
        case  63 : f = "GB_IDIV_SIGNED(y,x,64)"     ; break ;
        case  64 : f = "GB_IDIV_UNSIGNED(y,x,8)"    ; break ;
        case  65 : f = "GB_IDIV_UNSIGNED(y,x,16)"   ; break ;
        case  66 : f = "GB_IDIV_UNSIGNED(y,x,32)"   ; break ;
        case  67 : f = "GB_IDIV_UNSIGNED(y,x,64)"   ; break ;
        case  68 : f = "GB_FC32_div(x,y)"           ; break ;
        case  69 : f = "GB_FC64_div(x,y)"           ; break ;
        case  70 : f = "(y) / (x)"                  ; break ;

        // gt, isgt
        case  71 : f = "(x) > (y)"                  ; break ;

        // lt, islt
        case  72 : f = "(x) < (y)"                  ; break ;

        // ge, isget
        case  73 : f = "(x) >= (y)"                 ; break ;

        // le, isle
        case  74 : f = "(x) <= (y)"                 ; break ;

        // bget
        case  75 : f = "GB_BITGET(x,y,int8_t, 8)"   ; break ;
        case  76 : f = "GB_BITGET(x,y,int16_t,16)"  ; break ;
        case  77 : f = "GB_BITGET(x,y,int32_t,32)"  ; break ;
        case  78 : f = "GB_BITGET(x,y,int64_t,64)"  ; break ;
        case  79 : f = "GB_BITGET(x,y,uint8_t,8)"   ; break ;
        case  80 : f = "GB_BITGET(x,y,uint16_t,16)" ; break ;
        case  81 : f = "GB_BITGET(x,y,uint32_t,32)" ; break ;
        case  82 : f = "GB_BITGET(x,y,uint64_t,64)" ; break ;

        // bset
        case  83 : f = "GB_BITSET(x,y,int8_t, 8)"   ; break ;
        case  84 : f = "GB_BITSET(x,y,int16_t,16)"  ; break ;
        case  85 : f = "GB_BITSET(x,y,int32_t,32)"  ; break ;
        case  86 : f = "GB_BITSET(x,y,int64_t,64)"  ; break ;
        case  87 : f = "GB_BITSET(x,y,uint8_t,8)"   ; break ;
        case  88 : f = "GB_BITSET(x,y,uint16_t,16)" ; break ;
        case  89 : f = "GB_BITSET(x,y,uint32_t,32)" ; break ;
        case  90 : f = "GB_BITSET(x,y,uint64_t,64)" ; break ;

        // bclr
        case  91 : f = "GB_BITCLR(x,y,int8_t, 8)"   ; break ;
        case  92 : f = "GB_BITCLR(x,y,int16_t,16)"  ; break ;
        case  93 : f = "GB_BITCLR(x,y,int32_t,32)"  ; break ;
        case  94 : f = "GB_BITCLR(x,y,int64_t,64)"  ; break ;
        case  95 : f = "GB_BITCLR(x,y,uint8_t,8)"   ; break ;
        case  96 : f = "GB_BITCLR(x,y,uint16_t,16)" ; break ;
        case  97 : f = "GB_BITCLR(x,y,uint32_t,32)" ; break ;
        case  98 : f = "GB_BITCLR(x,y,uint64_t,64)" ; break ;

        // bshift
        case  99 : f = "GB_bitshift_int8(x,y)"      ; break ;
        case 100 : f = "GB_bitshift_int16(x,y)"     ; break ;
        case 101 : f = "GB_bitshift_int32(x,y)"     ; break ;
        case 102 : f = "GB_bitshift_int64(x,y)"     ; break ;
        case 103 : f = "GB_bitshift_uint8(x,y)"     ; break ;
        case 104 : f = "GB_bitshift_uint16(x,y)"    ; break ;
        case 105 : f = "GB_bitshift_uint32(x,y)"    ; break ;
        case 106 : f = "GB_bitshift_uint64(x,y)"    ; break ;

        // pow
        case 107 : f = "GB_pow_int8 (x, y)"         ; break ;
        case 108 : f = "GB_pow_int16 (x, y)"        ; break ;
        case 109 : f = "GB_pow_int32 (x, y)"        ; break ;
        case 110 : f = "GB_pow_int64 (x, y)"        ; break ;
        case 111 : f = "GB_pow_uint8 (x, y)"        ; break ;
        case 112 : f = "GB_pow_uint16 (x, y)"       ; break ;
        case 113 : f = "GB_pow_uint32 (x, y)"       ; break ;
        case 114 : f = "GB_pow_uint64 (x, y)"       ; break ;
        case 115 : f = "GB_powf (x, y)"             ; break ;
        case 116 : f = "GB_pow (x, y)"              ; break ;
        case 117 : f = "GB_cpowf (x, y)"            ; break ;
        case 118 : f = "GB_cpow (x, y)"             ; break ;

        // atan2
        case 119 : f = "atan2f (x, y)"              ; break ;
        case 120 : f = "atan2 (x, y)"               ; break ;

        // hypot
        case 121 : f = "hypotf (x, y)"              ; break ;
        case 122 : f = "hypot (x, y)"               ; break ;

        // fmod
        case 123 : f = "fmodf (x, y)"               ; break ;
        case 124 : f = "fmod (x, y)"                ; break ;

        // remainder
        case 125 : f = "remainderf (x, y)"          ; break ;
        case 126 : f = "remainder (x, y)"           ; break ;

        // copysign
        case 127 : f = "copysignf (x, y)"           ; break ;
        case 128 : f = "copysign (x, y)"            ; break ;

        // ldexp
        case 129 : f = "ldexpf (x, y)"              ; break ;
        case 130 : f = "ldexp (x, y)"               ; break ;

        // cmplex
        case 131 : f = "GxB_CMPLXF (x, y)"          ; break ;
        case 132 : f = "GxB_CMPLX (x, y)"           ; break ;

        // pair
        case 133 : f = "1"                          ; break ;

        //----------------------------------------------------------------------
        // positional ops
        //----------------------------------------------------------------------

        // in a semiring:  cij += aik * bkj
        //      firsti is i, firstj is k, secondi k, secondj is j

        // in an ewise operation:  cij = aij + bij
        //      firsti is i, firstj is j, secondi i, secondj is j

        case 134 : f = "i"                          ; break ;
        case 135 : f = "k"                          ; break ;
        case 136 : f = "j"                          ; break ;
        case 137 : f = "i+1"                        ; break ;
        case 138 : f = "k+1"                        ; break ;
        case 139 : f = "j+1"                        ; break ;

        default  : f = NULL ;                       ; break ;
    }

    printf("f = %s, %d\n", f, ecode);
    (*op_string) = f ;
}

//------------------------------------------------------------------------------
// GB_macrofy_binop: convert an opstring into a macro
//------------------------------------------------------------------------------

void GB_macrofy_binop
(
    // input:
    FILE *fp,                   // File to write macros, assumed open already
    const char *macro_name,     // name of macro to construct
    const char *op_string,            // string defining the operator
    bool flipxy                 // if true, use mult(y,x) else mult(x,y)
)
{
    if (flipxy)
    {
        // reverse the x and y arguments to flip the operator
        fprintf ( fp,
            "#define %s(y,x) (%s)\n", macro_name, op_string) ;
    }
    else
    {
        // operator is not flipped
        fprintf ( fp, 
            "#define %s(x,y) (%s)\n", macro_name, op_string) ;
    }
}


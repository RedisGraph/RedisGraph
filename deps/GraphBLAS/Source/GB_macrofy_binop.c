//------------------------------------------------------------------------------
// GB_macrofy_binop: construct the macro and defn for a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_binop
(
    FILE *fp,
    // input:
    const char *macro_name,
    bool flipxy,                // if true: op is f(y,x), multipicative only
    bool is_monoid,             // if true: additive operator for monoid
    int ecode,
    GrB_BinaryOp op,            // may be NULL (for GB_wait)
    bool skip_defn
)
{
    if (ecode == 0)
    {

        //----------------------------------------------------------------------
        // user-defined operator
        //----------------------------------------------------------------------

        if (is_monoid)
        {
            // additive operator: no i,k,j parameters
            fprintf (fp, "#define %s(z,x,y) %s (&(z), &(x), &(y))\n",
                macro_name, op->name) ;
        }
        else if (flipxy)
        {
            // flipped multiplicative or ewise operator
            // note: no positional operands for user-defined ops (yet)
            fprintf (fp, "#define %s(z,y,x,j,k,i) %s (&(z), &(x), &(y))\n",
                macro_name, op->name) ;
        }
        else
        {
            // unflipped multiplicative or ewise operator
            fprintf (fp, "#define %s(z,x,y,i,k,j) %s (&(z), &(x), &(y))\n",
                macro_name, op->name) ;
        }

        if (!skip_defn && op->defn != NULL)
        {
            fprintf (fp, "%s\n", op->defn) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // built-in operator
        //----------------------------------------------------------------------

        const char *f ;

        switch (ecode)
        {

            //------------------------------------------------------------------
            // built-in ops, can be used in a monoid
            //------------------------------------------------------------------

            // first
            case   1 : f = "z = (x)"                         ; break ;

            // any, second
            case   2 : f = "z = (y)"                         ; break ;

            // min
            case   3 : f = "z = fminf (x,y)"                 ; break ;
            case   4 : f = "z = fmin (x,y)"                  ; break ;
            case   5 : f = "z = GB_IMIN (x,y)"               ; break ;

            // max
            case   6 : f = "z = fmaxf (x,y)"                 ; break ;
            case   7 : f = "z = fmax (x,y)"                  ; break ;
            case   8 : f = "z = GB_IMAX (x,y)"               ; break ;

            // plus
            case   9 : f = "z = GB_FC32_add (x,y)"           ; break ;
            case  10 : f = "z = GB_FC64_add (x,y)"           ; break ;
            case  11 : f = "z = (x) + (y)"                   ; break ;

            // times
            case  12 : f = "z = GB_FC32_mul (x,y)"           ; break ;
            case  13 : f = "z = GB_FC64_mul (x,y)"           ; break ;
            case  14 : f = "z = (x) * (y)"                   ; break ;

            // eq, iseq, lxnor
            case  15 : f = "z = ((x) == (y))"                ; break ;

            // ne, isne, lxor
            case  16 : f = "z = ((x) != (y))"                ; break ;

            // lor
            case  17 : f = "z = ((x) || (y))"                ; break ;

            // land
            case  18 : f = "z = ((x) && (y))"                ; break ;

            // bor
            case  19 : f = "z = ((x) | (y))"                 ; break ;

            // band
            case  20 : f = "z = ((x) & (y))"                 ; break ;

            // bxor
            case  21 : f = "z = ((x) ^ (y))"                 ; break ;

            // bxnor
            case  22 : f = "z = (~((x) ^ (y)))"              ; break ;

            // 23 to 31 are unused, but reserved for future monoids

            //------------------------------------------------------------------
            // built-in ops, cannot be used in a monoid
            //------------------------------------------------------------------

            // eq for complex
            case  32 : f = "z = GB_FC32_eq (x,y)"            ; break ;
            case  33 : f = "z = GB_FC64_eq (x,y)"            ; break ;

            // iseq for complex
            case  34 : f = "z = GB_FC32_iseq (x,y)"          ; break ;
            case  35 : f = "z = GB_FC64_iseq (x,y)"          ; break ;

            // ne for complex
            case  36 : f = "z = GB_FC32_ne (x,y)"            ; break ;
            case  37 : f = "z = GB_FC64_ne (x,y)"            ; break ;

            // isne for complex
            case  38 : f = "z = GB_FC32_isne (x,y)"          ; break ;
            case  39 : f = "z = GB_FC64_isne (x,y)"          ; break ;

            // lor for non-boolean
            case  40 : f = "z = (((x)!=0) || ((y)!=0))"      ; break ;

            // land for non-boolean
            case  41 : f = "z = (((x)!=0) && ((y)!=0))"      ; break ;

            // lxor for non-boolean
            case  42 : f = "z = (((x)!=0) != ((y)!=0))"      ; break ;

            // minus
            case  43 : f = "z = GB_FC32_minus (x,y)"         ; break ;
            case  44 : f = "z = GB_FC64_minus (x,y)"         ; break ;
            case  45 : f = "z = (x) - (y)"                   ; break ;

            // rminus
            case  46 : f = "z = GB_FC32_minus (y,x)"         ; break ;
            case  47 : f = "z = GB_FC64_minus (y,x)"         ; break ;
            case  48 : f = "z = (y) - (x)"                   ; break ;

            // div:
            case  49 : f = "z = GB_idiv_int8 (x,y)"          ; break ;
            case  50 : f = "z = GB_idiv_int16 (x,y)"         ; break ;
            case  51 : f = "z = GB_idiv_int32 (x,y)"         ; break ;
            case  52 : f = "z = GB_idiv_int64 (x,y)"         ; break ;
            case  53 : f = "z = GB_idiv_uint8 (x,y)"         ; break ;
            case  54 : f = "z = GB_idiv_uint16 (x,y)"        ; break ;
            case  55 : f = "z = GB_idiv_uint32 (x,y)"        ; break ;
            case  56 : f = "z = GB_idiv_uint64 (x,y)"        ; break ;
            case  57 : f = "z = GB_FC32_div (x,y)"           ; break ;
            case  58 : f = "z = GB_FC64_div (x,y)"           ; break ;
            case  59 : f = "z = (x) / (y)"                   ; break ;

            // rdiv
            case  60 : f = "z = GB_idiv_int8 (y,x)"          ; break ;
            case  61 : f = "z = GB_idiv_int16 (y,x)"         ; break ;
            case  62 : f = "z = GB_idiv_int32 (y,x)"         ; break ;
            case  63 : f = "z = GB_idiv_int64 (y,x)"         ; break ;
            case  64 : f = "z = GB_idiv_uint8 (y,x)"         ; break ;
            case  65 : f = "z = GB_idiv_uint16 (y,x)"        ; break ;
            case  66 : f = "z = GB_idiv_uint32 (y,x)"        ; break ;
            case  67 : f = "z = GB_idiv_uint64 (y,x)"        ; break ;
            case  68 : f = "z = GB_FC32_div (x,y)"           ; break ;
            case  69 : f = "z = GB_FC64_div (x,y)"           ; break ;
            case  70 : f = "z = (y) / (x)"                   ; break ;

            // gt, isgt
            case  71 : f = "z = ((x) > (y))"                 ; break ;

            // lt, islt
            case  72 : f = "z = ((x) < (y))"                 ; break ;

            // ge, isget
            case  73 : f = "z = ((x) >= (y))"                ; break ;

            // le, isle
            case  74 : f = "z = ((x) <= (y))"                ; break ;

            // bget
            case  75 : f = "z = GB_BITGET (x,y,int8_t, 8)"   ; break ;
            case  76 : f = "z = GB_BITGET (x,y,int16_t,16)"  ; break ;
            case  77 : f = "z = GB_BITGET (x,y,int32_t,32)"  ; break ;
            case  78 : f = "z = GB_BITGET (x,y,int64_t,64)"  ; break ;
            case  79 : f = "z = GB_BITGET (x,y,uint8_t,8)"   ; break ;
            case  80 : f = "z = GB_BITGET (x,y,uint16_t,16)" ; break ;
            case  81 : f = "z = GB_BITGET (x,y,uint32_t,32)" ; break ;
            case  82 : f = "z = GB_BITGET (x,y,uint64_t,64)" ; break ;

            // bset
            case  83 : f = "z = GB_BITSET (x,y,int8_t, 8)"   ; break ;
            case  84 : f = "z = GB_BITSET (x,y,int16_t,16)"  ; break ;
            case  85 : f = "z = GB_BITSET (x,y,int32_t,32)"  ; break ;
            case  86 : f = "z = GB_BITSET (x,y,int64_t,64)"  ; break ;
            case  87 : f = "z = GB_BITSET (x,y,uint8_t,8)"   ; break ;
            case  88 : f = "z = GB_BITSET (x,y,uint16_t,16)" ; break ;
            case  89 : f = "z = GB_BITSET (x,y,uint32_t,32)" ; break ;
            case  90 : f = "z = GB_BITSET (x,y,uint64_t,64)" ; break ;

            // bclr
            case  91 : f = "z = GB_BITCLR (x,y,int8_t, 8)"   ; break ;
            case  92 : f = "z = GB_BITCLR (x,y,int16_t,16)"  ; break ;
            case  93 : f = "z = GB_BITCLR (x,y,int32_t,32)"  ; break ;
            case  94 : f = "z = GB_BITCLR (x,y,int64_t,64)"  ; break ;
            case  95 : f = "z = GB_BITCLR (x,y,uint8_t,8)"   ; break ;
            case  96 : f = "z = GB_BITCLR (x,y,uint16_t,16)" ; break ;
            case  97 : f = "z = GB_BITCLR (x,y,uint32_t,32)" ; break ;
            case  98 : f = "z = GB_BITCLR (x,y,uint64_t,64)" ; break ;

            // bshift
            case  99 : f = "z = GB_bitshift_int8 (x,y)"      ; break ;
            case 100 : f = "z = GB_bitshift_int16 (x,y)"     ; break ;
            case 101 : f = "z = GB_bitshift_int32 (x,y)"     ; break ;
            case 102 : f = "z = GB_bitshift_int64 (x,y)"     ; break ;
            case 103 : f = "z = GB_bitshift_uint8 (x,y)"     ; break ;
            case 104 : f = "z = GB_bitshift_uint16 (x,y)"    ; break ;
            case 105 : f = "z = GB_bitshift_uint32 (x,y)"    ; break ;
            case 106 : f = "z = GB_bitshift_uint64 (x,y)"    ; break ;

            // pow
            case 107 : f = "z = GB_pow_int8 (x, y)"          ; break ;
            case 108 : f = "z = GB_pow_int16 (x, y)"         ; break ;
            case 109 : f = "z = GB_pow_int32 (x, y)"         ; break ;
            case 110 : f = "z = GB_pow_int64 (x, y)"         ; break ;
            case 111 : f = "z = GB_pow_uint8 (x, y)"         ; break ;
            case 112 : f = "z = GB_pow_uint16 (x, y)"        ; break ;
            case 113 : f = "z = GB_pow_uint32 (x, y)"        ; break ;
            case 114 : f = "z = GB_pow_uint64 (x, y)"        ; break ;
            case 115 : f = "z = GB_powf (x, y)"              ; break ;
            case 116 : f = "z = GB_pow (x, y)"               ; break ;
            case 117 : f = "z = GB_cpowf (x, y)"             ; break ;
            case 118 : f = "z = GB_cpow (x, y)"              ; break ;

            // atan2
            case 119 : f = "z = atan2f (x, y)"               ; break ;
            case 120 : f = "z = atan2 (x, y)"                ; break ;

            // hypot
            case 121 : f = "z = hypotf (x, y)"               ; break ;
            case 122 : f = "z = hypot (x, y)"                ; break ;

            // fmod
            case 123 : f = "z = fmodf (x, y)"                ; break ;
            case 124 : f = "z = fmod (x, y)"                 ; break ;

            // remainder
            case 125 : f = "z = remainderf (x, y)"           ; break ;
            case 126 : f = "z = remainder (x, y)"            ; break ;

            // copysign
            case 127 : f = "z = copysignf (x, y)"            ; break ;
            case 128 : f = "z = copysign (x, y)"             ; break ;

            // ldexp
            case 129 : f = "z = ldexpf (x, y)"               ; break ;
            case 130 : f = "z = ldexp (x, y)"                ; break ;

            // cmplex
            case 131 : f = "z = GxB_CMPLXF (x, y)"           ; break ;
            case 132 : f = "z = GxB_CMPLX (x, y)"            ; break ;

            // pair
            case 133 : f = "z = 1"                           ; break ;

            //------------------------------------------------------------------
            // positional ops
            //------------------------------------------------------------------

            // in a semiring:  cij += aik * bkj
            //      firsti is i, firstj is k, secondi k, secondj is j

            // in an ewise operation:  cij = aij + bij
            //      firsti is i, firstj is j, secondi i, secondj is j

            case 134 : f = "z = (i)"                         ; break ;
            case 135 : f = "z = (k)"                         ; break ;
            case 136 : f = "z = (j)"                         ; break ;
            case 137 : f = "z = (i) + 1"                     ; break ;
            case 138 : f = "z = (k) + 1"                     ; break ;
            case 139 : f = "z = (j) + 1"                     ; break ;

            //------------------------------------------------------------------
            // no-op for GB_wait (an implicit 2nd operator)
            //------------------------------------------------------------------

            case 140 : f = "z = y"                           ; break ;

            default  : f = "" ;                              ; break ;
        }

        //----------------------------------------------------------------------
        // create the macro
        //----------------------------------------------------------------------

        if (is_monoid)
        {
            // additive operator: no i,k,j parameters
            fprintf (fp, "#define %s(z,x,y) %s\n", macro_name, f) ;
        }
        else if (flipxy)
        {
            // flipped multiplicative or ewise operator
            fprintf (fp, "#define %s(z,y,x,j,k,i) %s\n", macro_name, f) ;
        }
        else
        {
            // unflipped multiplicative or ewise operator
            fprintf (fp, "#define %s(z,x,y,i,k,j) %s\n", macro_name, f) ;
        }
    }
}


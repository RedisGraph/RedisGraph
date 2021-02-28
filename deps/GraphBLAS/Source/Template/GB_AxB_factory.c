//------------------------------------------------------------------------------
// GB_AxB_factory: switch factory for C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is used by GB_AxB_Gustavson_builtin.c, GB_AxB_dot.c and GB_AxB_heap.c
// to create built-in versions of sparse matrix-matrix multiplication.  The
// #include'ing file defines the GB_AxB_WORKER macro, and mult_opcode,
// add_opcode, xycode, and zcode.

// Two 2nd level switch factories are used:

//      GB_AxB_type_factory: handles all semirings where the multiply operator
//          is TxT->T (as is the monoid).

//      GB_AxB_compare_factory: handles all semirings where the multiply
//          operator is TxT->bool (for the comparison operators, LT, GT, etc),
//          and where the monoid is bool x bool -> bool.

// If the multiplicative operator is ANY, then it is replaced here by SECOND,
// since that is faster for the saxpy-based methods (y is the value of B(k,j),
// which is loaded less frequently from memory than A(i,k)).

{
    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    switch (mult_opcode)
    {

        //----------------------------------------------------------------------
        case GB_FIRST_opcode   :    // z = x
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define GB_MULT_NAME _first
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_SECOND_opcode  :    // z = y
        case GB_ANY_opcode     :    // z = y
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define GB_MULT_NAME _second
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_PAIR_opcode   :    // z = 1
        //----------------------------------------------------------------------

            // land_pair, lor_pair, max_pair, min_pair, times_pair
            // all become any_pair.

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define GB_MULT_IS_PAIR_OPERATOR
            #define GB_MULT_NAME _pair
            #include "GB_AxB_type_factory.c"
            #undef  GB_MULT_IS_PAIR_OPERATOR
            break ;

        //----------------------------------------------------------------------
        case GB_MIN_opcode     :    // z = min(x,y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MIN == TIMES == AND for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _min
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_MAX_opcode     :    // z = max(x,y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MAX == PLUS == OR for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _max
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_PLUS_opcode    :    // z = x + y
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MAX == PLUS == OR for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _plus
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_MINUS_opcode   :    // z = x - y
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MINUS == RMINUS == NE == ISNE == XOR for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _minus
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_RMINUS_opcode   :    // z = y - x (reverse minus)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MINUS == RMINUS == NE == ISNE == XOR for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _rminus
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_TIMES_opcode   :    // z = x * y
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MIN == TIMES == AND for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _times
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_DIV_opcode   :      // z = x / y
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // FIRST == DIV for boolean
            // See Source/GB.h for discusion on integer division
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _div
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_RDIV_opcode   :     // z = y / x (reverse division)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // SECOND == RDIV for boolean
            // See Source/GB.h for discusion on integer division
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _rdiv
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISEQ_opcode    :    // z = (x == y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISEQ == EQ for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _iseq
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISNE_opcode    :    // z = (x != y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MINUS == RMINUS == NE == ISNE == XOR for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _isne
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISGT_opcode    :    // z = (x >  y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISGT == GT for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _isgt
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISLT_opcode    :    // z = (x <  y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISLT == LT for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _islt
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISGE_opcode    :    // z = (x >= y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISGE == GE for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _isge
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISLE_opcode     :    // z = (x <= y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISLE == LE for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _isle
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_EQ_opcode      :    // z = (x == y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define GB_MULT_NAME _eq
            #include "GB_AxB_compare_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_NE_opcode      :    // z = (x != y)
        //----------------------------------------------------------------------

            // 40 semirings: (and,or,xor,eq) * (10 types)
            // MINUS == RMINUS == NE == ISNE == XOR for boolean
            #define GB_NO_BOOLEAN
            #define GB_MULT_NAME _ne
            #include "GB_AxB_compare_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_GT_opcode      :    // z = (x >  y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define GB_MULT_NAME _gt
            #include "GB_AxB_compare_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LT_opcode      :    // z = (x <  y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define GB_MULT_NAME _lt
            #include "GB_AxB_compare_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_GE_opcode      :    // z = (x >= y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define GB_MULT_NAME _ge
            #include "GB_AxB_compare_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LE_opcode      :    // z = (x <= y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define GB_MULT_NAME _le
            #include "GB_AxB_compare_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LOR_opcode     :    // z = x || y
        //----------------------------------------------------------------------

            // The boolean operators OR, AND, and XOR for the "multiply"
            // function need to typecast their inputs from the xytype into
            // boolean, so they need to have the "(x != 0) ..." form.

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define GB_MULT_NAME _lor
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LAND_opcode    :    // z = x && y
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define GB_MULT_NAME _land
            #include "GB_AxB_type_factory.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LXOR_opcode    :    // z = x != y
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define GB_MULT_NAME _lxor
            #include "GB_AxB_type_factory.c"
            break ;

        default: ;
    }
}


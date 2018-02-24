//------------------------------------------------------------------------------
// GB_AxB_factory
//------------------------------------------------------------------------------

// This is used by GB_AxB_builtin.c and GB_Matrix_AdotB.c to create built-in
// versions of sparse matrix-matrix multiplication.  The #include'ing file
// #define's the AxB macro, and mult_opcode, add_opcode, xycode, and zcode

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
            #define mult _first
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_SECOND_opcode  :    // z = y
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define mult _second
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_MIN_opcode     :    // z = min(x,y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MIN == TIMES == AND for boolean
            #define NO_BOOLEAN
            #define mult _min
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_MAX_opcode     :    // z = max(x,y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MAX == PLUS == OR for boolean
            #define NO_BOOLEAN
            #define mult _max
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_PLUS_opcode    :    // z = x + y
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MAX == PLUS == OR for boolean
            #define NO_BOOLEAN
            #define mult _plus
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_MINUS_opcode   :    // z = flipxy ? (y-x) : (x-y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MINUS == NE == ISNE == XOR for boolean
            #define NO_BOOLEAN
            #define mult _minus
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_TIMES_opcode   :    // z = x * y
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MIN == TIMES == AND for boolean
            #define NO_BOOLEAN
            #define mult _times
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_DIV_opcode   :      // z = flipxy ? (y / x) : (x / y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // FIRST == DIV for boolean
            // See Source/GB.h for disscusion on integer division
            #define NO_BOOLEAN
            #define mult _div
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISEQ_opcode    :    // z = (x == y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISEQ == EQ for boolean
            #define NO_BOOLEAN
            #define mult _iseq
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISNE_opcode    :    // z = (x != y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // MINUS == NE == ISNE == XOR for boolean
            #define NO_BOOLEAN
            #define mult _isne
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISGT_opcode    :    // z = (x >  y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISGT == GT for boolean
            #define NO_BOOLEAN
            #define mult _isgt
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISLT_opcode    :    // z = (x <  y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISLT == LT for boolean
            #define NO_BOOLEAN
            #define mult _islt
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISGE_opcode    :    // z = (x >= y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISGE == GE for boolean
            #define NO_BOOLEAN
            #define mult _isge
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_ISLE_opcode     :    // z = (x <= y)
        //----------------------------------------------------------------------

            // 40 semirings: (min,max,plus,times) for non-boolean
            // ISLE == LE for boolean
            #define NO_BOOLEAN
            #define mult _isle
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_EQ_opcode      :    // z = (x == y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define mult _eq
            #include "GB_AxB_compare_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_NE_opcode      :    // z = (x != y)
        //----------------------------------------------------------------------

            // 40 semirings: (and,or,xor,eq) * (10 types)
            // MINUS == NE == ISNE == XOR for boolean
            #define NO_BOOLEAN
            #define mult _ne
            #include "GB_AxB_compare_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_GT_opcode      :    // z = (x >  y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define mult _gt
            #include "GB_AxB_compare_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LT_opcode      :    // z = (x <  y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define mult _lt
            #include "GB_AxB_compare_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_GE_opcode      :    // z = (x >= y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define mult _ge
            #include "GB_AxB_compare_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LE_opcode      :    // z = (x <= y)
        //----------------------------------------------------------------------

            // 44 semirings: (and,or,xor,eq) * (11 types)
            #define mult _le
            #include "GB_AxB_compare_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LOR_opcode     :    // z = x || y
        //----------------------------------------------------------------------

            // The boolean operators OR, AND, and XOR for the "multiply"
            // function need to typecast their inputs from the xytype into
            // boolean, so they need to have the "(x != 0) ..." form.

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define mult _lor
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LAND_opcode    :    // z = x && y
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define mult _land
            #include "GB_AxB_template.c"
            break ;

        //----------------------------------------------------------------------
        case GB_LXOR_opcode    :    // z = x != y
        //----------------------------------------------------------------------

            // 44 semirings: (min,max,plus,times) for non-boolean, and
            // (or,and,xor,eq) for boolean
            #define mult _lxor
            #include "GB_AxB_template.c"
            break ;

        default: ;
    }
}


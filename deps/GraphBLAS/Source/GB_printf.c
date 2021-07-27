//------------------------------------------------------------------------------
// GB_printf.c: printing for GraphBLAS *check functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

#if GB_BURBLE

void GB_burble_assign
(
    const bool C_replace,       // descriptor for C
    const int Ikind,
    const int Jkind,
    const GrB_Matrix M,         // mask matrix, which is not NULL here
    const bool Mask_comp,       // true for !M, false for M
    const bool Mask_struct,     // true if M is structural, false if valued
    const GrB_BinaryOp accum,   // present here
    const GrB_Matrix A,         // input matrix, not transposed
    const int assign_kind       // row assign, col assign, assign, or subassign
)
{

    //--------------------------------------------------------------------------
    // quick return if burble is disabled
    //--------------------------------------------------------------------------

    if (!GB_Global_burble_get ( ))
    {
        return ;
    }

    //--------------------------------------------------------------------------
    // construct the accum operator string
    //--------------------------------------------------------------------------

    char *Op ;
    if (accum == NULL)
    {
        // no accum operator is present
        Op = "" ;
    }
    else
    {
        // use a simpler version of accum->name
        if (accum->opcode == GB_USER_opcode) Op = "op" ;
        else if (GB_STRING_MATCH (accum->name, "plus")) Op = "+" ;
        else if (GB_STRING_MATCH (accum->name, "minus")) Op = "-" ;
        else if (GB_STRING_MATCH (accum->name, "times")) Op = "*" ;
        else if (GB_STRING_MATCH (accum->name, "div")) Op = "/" ;
        else if (GB_STRING_MATCH (accum->name, "or")) Op = "|" ;
        else if (GB_STRING_MATCH (accum->name, "and")) Op = "&" ;
        else if (GB_STRING_MATCH (accum->name, "xor")) Op = "^" ;
        else Op = accum->name ;
    }

    //--------------------------------------------------------------------------
    // construct the Mask string
    //--------------------------------------------------------------------------

    const char *Mask ;
    char Mask_string [GB_LEN+1] ;
    if (M == NULL)
    {
        // M is not present
        if (Mask_comp)
        {
            Mask = C_replace ? "<!,replace>" : "<!>" ;
        }
        else
        {
            Mask = C_replace ? "<replace>" : "" ;
        }
    }
    else
    {
        // M is present
        snprintf (Mask_string, GB_LEN, "<%sM%s%s%s>",
            (Mask_comp) ? "!" : "",
            GB_IS_BITMAP (M) ? ",bitmap" : (GB_IS_FULL (M) ? ",full" : ""),
            Mask_struct ? ",struct" : "",
            C_replace ? ",replace" : "") ;
        Mask = Mask_string ;
    }

    //--------------------------------------------------------------------------
    // construct the string for A or the scalar
    //--------------------------------------------------------------------------

    const char *S = (A == NULL) ? "scalar" : "A" ;

    //--------------------------------------------------------------------------
    // construct the string for (I,J)
    //--------------------------------------------------------------------------

    const char *Istr = (Ikind == GB_ALL) ? ":" : "I" ;
    const char *Jstr = (Jkind == GB_ALL) ? ":" : "J" ;
    char IJ [GB_LEN+1] ;
    snprintf (IJ, GB_LEN, "(%s,%s)", Istr, Jstr) ;
    if (Ikind == GB_ALL && Jkind == GB_ALL)
    {
        // do not print the (I,J) indices
        IJ [0] = '\0' ;
    }

    //--------------------------------------------------------------------------
    // burble the final result
    //--------------------------------------------------------------------------

    switch (assign_kind)
    {
        case GB_ROW_ASSIGN:
            // C(i,J) = A
            snprintf (IJ, GB_LEN, "(i,%s)", Jstr) ;
            GBURBLE ("C%s%s %s= A ", Mask, IJ, Op) ;
            break ;

        case GB_COL_ASSIGN:
            // C(I,j) = A
            snprintf (IJ, GB_LEN, "(%s,j)", Istr) ;
            GBURBLE ("C%s%s %s= A ", Mask, IJ, Op) ;
            break ;

        case GB_ASSIGN:
            // C(I,J) = A
            GBURBLE ("C%s%s %s= %s ", Mask, IJ, Op, S) ;
            break ;

        case GB_SUBASSIGN:
            // C(i,J) = A
            GBURBLE ("C%s%s %s= %s ", IJ, Mask, Op, S) ;
            break ;

        default: ;
    }
}

#endif


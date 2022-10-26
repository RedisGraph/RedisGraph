//------------------------------------------------------------------------------
// GB_macrofy_monoid: build macros for a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_monoid  // construct the macros for a monoid
(
    FILE *fp,           // File to write macros, assumed open already
    // inputs:
    int add_ecode,      // binary op as an enum
    int id_ecode,       // identity value as an enum
    int term_ecode,     // terminal value as an enum (<= 28 is terminal)
    GrB_Monoid monoid,  // monoid to macrofy
    bool skip_defn      // if true, do not include the user-defined add function
)
{
    const char *ztype_name = monoid->op->ztype->name ;

    //--------------------------------------------------------------------------
    // create macros for the additive operator
    //--------------------------------------------------------------------------

    GB_macrofy_binop (fp, "GB_ADD", false, true, add_ecode, monoid->op,
        skip_defn) ;

    //--------------------------------------------------------------------------
    // create macros for the identity value
    //--------------------------------------------------------------------------

    if (id_ecode <= 28)
    {
        // built-in monoid: a simple assignment
        const char *id_value = GB_charify_identity_or_terminal (id_ecode) ;
        fprintf (fp, "#define GB_DECLARE_MONOID_IDENTITY(z) "
            "%s z = (%s) (%s) ;\n", ztype_name, ztype_name, id_value) ;
    }
    else
    {
        // user-defined monoid:  all we have are the bytes
        GB_macrofy_bytes (fp, "MONOID_IDENTITY", ztype_name,
            (uint8_t *) (monoid->identity), monoid->op->ztype->size) ;
    }

    //--------------------------------------------------------------------------
    // create macros for the terminal value and terminal conditions
    //--------------------------------------------------------------------------

    if (monoid->terminal == NULL)
    {
        // monoid is not terminal (either built-in or user-defined)
        fprintf (fp, "#define GB_DECLARE_MONOID_TERMINAL(z)\n") ;
        fprintf (fp, "#define GB_TERMINAL_CONDITION(cij,z) (false)\n") ;
        fprintf (fp, "#define GB_IF_TERMINAL_BREAK(cij,z)\n") ;
    }
    else if (term_ecode == 18)
    {
        // ANY monoid is terminal but with no specific terminal value
        fprintf (fp, "#define GB_DECLARE_MONOID_TERMINAL(z)\n") ;
        fprintf (fp, "#define GB_TERMINAL_CONDITION(cij,z) (true)\n") ;
        fprintf (fp, "#define GB_IF_TERMINAL_BREAK(cij,z) break\n") ;
    }
    else if (term_ecode <= 28)
    {
        // built-in terminal monoid: terminal value is a simple assignment
        const char *term_value = GB_charify_identity_or_terminal (term_ecode) ;
        fprintf (fp, "#define GB_DECLARE_MONOID_TERMINAL(z) "
            "%s z = (%s) (%s) ;\n", ztype_name, ztype_name, term_value) ;
        fprintf (fp, "#define GB_TERMINAL_CONDITION(cij,z) ((cij) == (z))\n") ;
        fprintf (fp, "#define GB_IF_TERMINAL_BREAK(cij,z) "
            "if ((cij) == (z)) break\n") ;
    }
    else
    {
        // user-defined terminal monoid
        GB_macrofy_bytes (fp, "MONOID_TERMINAL", ztype_name,
            monoid->terminal, monoid->op->ztype->size) ;
        fprintf (fp, "#define GB_TERMINAL_CONDITION(cij,z)"
            " (memcmp (&(cij), &(z), %d) == 0)\n",
            (int) monoid->op->ztype->size) ;
        fprintf (fp, "#define GB_IF_TERMINAL_BREAK(cij,z) "
            " if (memcmp (&(cij), &(z), %d) == 0) break\n",
            (int) monoid->op->ztype->size) ;
    }
}


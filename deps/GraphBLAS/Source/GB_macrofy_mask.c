//------------------------------------------------------------------------------
// GB_macrofy_mask: return string to define mask macros
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// User-defined types cannot be used as a valued mask.  They can be used
// as structural masks.

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_mask       // return enum to define mask macros
(
    // input
    FILE *fp,                   // File to write macros, assumed open already
    int mask_ecode              // enumified mask
)
{

    const char *f ;

    switch (mask_ecode)
    {

        //----------------------------------------------------------------------
        // no mask
        //----------------------------------------------------------------------

        case 0 :    // mask not complemented
            f = "#define GB_MTYPE (no mask present)\n"
                "#define MX(p)    (no mask present)\n"
                "#define GB_MASK_STRUCT 1\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 1" ;
            break ;

        case 1 :    // mask complemented
            f = "#define GB_MTYPE (no mask present)\n"
                "#define MX(p)    (no mask present)\n"
                "#define GB_MASK_STRUCT 1\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 1" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, and structural (not valued)
        //----------------------------------------------------------------------

        case 2 :
            // mask not complemented, type: structural
            f = "#define GB_MTYPE void\n"
                "#define MX(p) 1\n"
                "#define GB_MASK_STRUCT 1\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 0" ;
            break ;

        case 3 :
            // mask complemented, type: structural
            f = "#define GB_MTYPE void\n"
                "#define MX(p) 1\n"
                "#define GB_MASK_STRUCT 1\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 0" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 1 byte
        //----------------------------------------------------------------------

        case 4 :
            // mask not complemented, type: bool, int8, uint8
            f = "#define GB_MTYPE uint8_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 0" ;
            break ;

        case 5 :
            // mask complemented, type: bool, int8, uint8
            f = "#define GB_MTYPE uint8_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 0" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 2 bytes
        //----------------------------------------------------------------------

        case 6 :
            // mask not complemented, type: int16, uint16
            f = "#define GB_MTYPE uint16_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 0" ;
            break ;

        case 7 :
            // mask complemented, type: int16, uint16
            f = "#define GB_MTYPE uint16_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 0" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 4 bytes
        //----------------------------------------------------------------------

        case 8 :
            // mask not complemented, type: float, int32, uint32
            f = "#define GB_MTYPE uint32_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 0" ;
            break ;

        case 9 :
            // mask complemented, type: float, int32, uint32
            f = "#define GB_MTYPE uint32_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 0" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 8 bytes
        //----------------------------------------------------------------------

        case 10 :
            // mask not complemented, type: double, float complex, int64, uint64
            f = "#define GB_MTYPE uint64_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 0" ;
            break ;

        case 11 :
            // mask complemented, type: double, float complex, int64, uint64
            f = "#define GB_MTYPE uint64_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 0" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 16 bytes
        //----------------------------------------------------------------------

        case 12 :
            // mask not complemented, type: double complex
            f = "#define GB_MTYPE uint64_t\n"
                "#define MX(p) (Mx [2*(p)] != 0 || Mx [2*(p)+1] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 0\n"
                "#define GB_NO_MASK 0" ;
            break ;

        case 13 :
            // mask complemented, type: double complex
            f = "#define GB_MTYPE uint64_t\n"
                "#define MX(p) (Mx [2*(p)] != 0 || Mx [2*(p)+1] != 0)\n"
                "#define GB_MASK_STRUCT 0\n"
                "#define GB_MASK_COMP 1\n"
                "#define GB_NO_MASK 0" ;
            break ;

        //----------------------------------------------------------------------
        // invalid case
        //----------------------------------------------------------------------

        default: ;
            f = "#error undefined mask behavior" ;
            break ;
    }

    fprintf (fp, "// mask:\n%s\n", f) ;
}


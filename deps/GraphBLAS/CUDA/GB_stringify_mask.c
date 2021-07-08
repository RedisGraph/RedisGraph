//------------------------------------------------------------------------------
// GB_stringify_mask: construct macros to access the mask
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// User-defined types cannot be used as a valued mask.  They can be used
// as structural masks.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_stringify_mask: return string to define mask macros
//------------------------------------------------------------------------------

void GB_stringify_mask     // return string to define mask macros
(
    // output:
    char **mask_macros,         // string that defines the mask macros
    // input:
    const GB_Type_code mcode,   // typecode of the mask matrix M,
                                // or 0 if M is not present
    bool Mask_struct,           // true if M structural, false if valued
    bool Mask_comp              // true if M complemented
)
{

    int mask_ecode ;

    // get mask_ecode from mask type (mask_ecode) and mask descriptor
    GB_enumify_mask (&mask_ecode, mcode, Mask_struct, Mask_comp) ;

    // convert ecode to string containing mask macros
    GB_macrofy_mask (mask_macros, mask_ecode) ;
}

//------------------------------------------------------------------------------
// GB_enumify_mask: return mask_ecode to define mask macros
//------------------------------------------------------------------------------

void GB_enumify_mask       // return enum to define mask macros
(
    // output:
    int *mask_ecode,            // enumified mask
    // input
    const GB_Type_code mcode,   // typecode of the mask matrix M,
                                // or 0 if M is not present
    bool Mask_struct,           // true if M structural, false if valued
    bool Mask_comp              // true if M complemented
)
{

    // Mask_comp becomes the least significant bit, so that
    // Mask_comp = (mask_ecode & 0x1) can be computed later.
    // Mask_struct = (mask_ecode == 2 || mask_ecode == 3)

    int e = -1 ;

    if (mcode == 0)
    {

        //----------------------------------------------------------------------
        // no mask is present
        //----------------------------------------------------------------------

        // Mask_struct is ignored, but the mask may be complemented or not

        // user-defined types are OK.  The values of M are not accessed.
        e = (!Mask_comp) ? 0 : 1 ;

    }
    else if (Mask_struct)
    {

        //----------------------------------------------------------------------
        // M is present, and structural (not valued).
        //----------------------------------------------------------------------

        // user-defined types are OK.  The values of M are not accessed.
        e = (!Mask_comp) ? 2 : 3 ;

    }
    else
    {

        //----------------------------------------------------------------------
        // M is present, and valued
        //----------------------------------------------------------------------

        switch (mcode)
        {

            case GB_BOOL_code:
            case GB_INT8_code:
            case GB_UINT8_code:

                // valued mask, values are 1 byte in size
                e = (!Mask_comp) ? 4 : 5 ;
                break ;

            case GB_INT16_code:
            case GB_UINT16_code:

                // valued mask, values are 2 bytes in size
                e = (!Mask_comp) ? 6 : 7 ;
                break ;

            case GB_INT32_code:
            case GB_UINT32_code:
            case GB_FP32_code:

                // valued mask, values are 4 bytes in size
                e = (!Mask_comp) ? 8 : 9 ;
                break ;

            case GB_INT64_code:
            case GB_UINT64_code:
            case GB_FC32_code:
            case GB_FP64_code:

                // valued mask, values are 8 bytes in size
                e = (!Mask_comp) ? 10 : 11 ;
                break ;

            case GB_FC64_code:

                // valued mask, values are 16 bytes in size
                e = (!Mask_comp) ? 12 : 13 ;
                break ;

            default: ;
                // user-defined types are not allowed
                e = -1 ;
                break ;
        }
    }

    (*mask_ecode) = e ;
}

//------------------------------------------------------------------------------
// GB_macrofy_mask: return string to define mask macros
//------------------------------------------------------------------------------

void GB_macrofy_mask       // return enum to define mask macros
(
    // output:
    char **mask_macros,         // string that defines the mask macros
    // input
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
                "#define GB_MASK_COMP false" ;
            break ;

        case 1 :    // mask complemented
            f = "#define GB_MTYPE (no mask present)\n"
                "#define MX(p)    (no mask present)\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, and structural (not valued)
        //----------------------------------------------------------------------

        case 2 :
            // mask not complemented, type: structural
            f = "#define GB_MTYPE void\n"
                "#define MX(p) true\n"
                "#define GB_MASK_COMP false" ;
            break ;

        case 3 :
            // mask complemented, type: structural
            f = "#define GB_MTYPE void\n"
                "#define MX(p) true\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 1 byte
        //----------------------------------------------------------------------

        case 4 :
            // mask not complemented, type: bool, int8, uint8
            f = "#define GB_MTYPE uint8_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP false" ;
            break ;

        case 5 :
            // mask complemented, type: bool, int8, uint8
            f = "#define GB_MTYPE uint8_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 2 bytes
        //----------------------------------------------------------------------

        case 6 :
            // mask not complemented, type: int16, uint16
            f = "#define GB_MTYPE uint16_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP false" ;
            break ;

        case 7 :
            // mask complemented, type: int16, uint16
            f = "#define GB_MTYPE uint16_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 4 bytes
        //----------------------------------------------------------------------

        case 8 :
            // mask not complemented, type: float, int32, uint32
            f = "#define GB_MTYPE uint32_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP false" ;
            break ;

        case 9 :
            // mask complemented, type: float, int32, uint32
            f = "#define GB_MTYPE uint32_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 8 bytes
        //----------------------------------------------------------------------

        case 10 :
            // mask not complemented, type: double, float complex, int64, uint64
            f = "#define GB_MTYPE uint64_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP false" ;
            break ;

        case 11 :
            // mask complemented, type: double, float complex, int64, uint64
            f = "#define GB_MTYPE uint64_t\n" 
                "#define MX(p) (Mx [p] != 0)\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // M is present, valued, size: 16 bytes
        //----------------------------------------------------------------------

        case 12 :
            // mask not complemented, type: double complex
            f = "#define GB_MTYPE uint64_t\n"
                "#define MX(p) (Mx [2*(p)] != 0 || Mx [2*(p)+1] != 0)\n"
                "#define GB_MASK_COMP false" ;
            break ;

        case 13 :
            // mask complemented, type: double complex
            f = "#define GB_MTYPE uint64_t\n"
                "#define MX(p) (Mx [2*(p)] != 0 || Mx [2*(p)+1] != 0)\n"
                "#define GB_MASK_COMP true" ;
            break ;

        //----------------------------------------------------------------------
        // invalid case
        //----------------------------------------------------------------------

        default: ;
            f = NULL ;
            break ;
    }

    (*mask_macros) = f ;
}


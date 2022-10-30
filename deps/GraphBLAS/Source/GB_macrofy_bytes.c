//------------------------------------------------------------------------------
// GB_macrofy_bytes: create a single scalar from an array of bytes
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// When the macro is used, sizeof (ztype) must be the same as nbytes

#include "GB.h"
#include "GB_stringify.h"

void GB_macrofy_bytes
(
    // input:
    FILE *fp,           // File to write macros, assumed open already
    const char *Name,         // all-upper-case name
    const char *type_name,    // name of the type
    const uint8_t *value,     // array of size nbytes
    size_t nbytes
)
{

    fprintf (fp,
        "#define GB_DECLARE_%s(z) %s z ; \\\n"
        "    {                                                       \\\n"
        "        const uint8_t bytes [%d] =                          \\\n"
        "        {                                                   \\\n"
        "            ",
        Name, type_name, (int) nbytes) ;
    for (int k = 0 ; k < nbytes ; k++)
    {
        fprintf (fp, "0x%02x", (int) (value [k])) ;
        if (k < nbytes-1)
        {
            fprintf (fp, ", ") ;
            if (k > 0 && k % 8 == 7) fprintf (fp, "\\\n            ") ;
        }
    }
    // finalize the array and use memcpy to initialize the scalar
    fprintf (fp,
        "  \\\n"
        "        } ;                                                 \\\n"
        "        memcpy (&z, bytes, %d) ;                            \\\n"
        "    }\n",
        (int) nbytes) ;
}


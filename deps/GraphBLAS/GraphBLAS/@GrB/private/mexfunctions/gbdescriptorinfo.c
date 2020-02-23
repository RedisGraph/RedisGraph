//------------------------------------------------------------------------------
// gbdescriptorinfo: print a GraphBLAS descriptor (for illustration only)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin <= 1 && nargout == 0,
        "usage: GrB.descriptorinfo or GrB.descriptorinfo (d)") ;

    //--------------------------------------------------------------------------
    // construct the GraphBLAS descriptor
    //--------------------------------------------------------------------------

    base_enum_t base = BASE_DEFAULT ;
    kind_enum_t kind = KIND_GRB ;
    GxB_Format_Value fmt = GxB_NO_FORMAT ;
    GrB_Descriptor d = (nargin == 0) ? NULL :
        gb_mxarray_to_descriptor (pargin [nargin-1], &kind, &fmt, &base) ;

    if (d == NULL)
    { 
        printf ("\nDefault GraphBLAS descriptor:\n") ;
        OK (GrB_Descriptor_new (&d)) ;
    }

    //--------------------------------------------------------------------------
    // print the GraphBLAS descriptor
    //--------------------------------------------------------------------------

    OK (GxB_Descriptor_fprint (d, "", GxB_COMPLETE, NULL)) ;

    //--------------------------------------------------------------------------
    // print the extra terms in the MATLAB interface descriptor
    //--------------------------------------------------------------------------

    printf ("    d.kind     = ") ;
    switch (kind)
    {
        case KIND_SPARSE : printf ("sparse\n")  ; break ;
        case KIND_FULL   : printf ("full\n")    ; break ;
        case KIND_GRB    :
        default          : printf ("GrB\n")     ; break ;
    }

    printf ("    d.base     = ") ;
    switch (base)
    {
        case BASE_0_INT64  : printf ("zero-based\n")    ; break ;
        case BASE_1_INT64  : printf ("one-based int\n") ; break ;
        case BASE_1_DOUBLE : printf ("one-based\n")     ; break ;
        case BASE_DEFAULT  :
        default            : printf ("default\n")       ; break ;
    }

    printf ("    d.format   = ") ;
    switch (fmt)
    {
        case GxB_BY_ROW    : printf ("by row\n")    ; break ;
        case GxB_BY_COL    : printf ("by col\n")    ; break ;
        case GxB_NO_FORMAT :
        default            : printf ("default\n")   ; break ;
    }

    //--------------------------------------------------------------------------
    // free the descriptor
    //--------------------------------------------------------------------------

    OK (GrB_Descriptor_free (&d)) ;
    GB_WRAPUP ;
}


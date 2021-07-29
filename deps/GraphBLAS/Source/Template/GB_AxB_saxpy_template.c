//------------------------------------------------------------------------------
// GB_AxB_saxpy_template: C=A*B, C<M>=A*B, or C<!M>=A*B via saxpy method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All 4 matrices have any format: hypersparse, sparse, bitmap, or full.

{
    switch (saxpy_method)
    {

        case GB_SAXPY_METHOD_3 :
        {
            // C is sparse or hypersparse, using minimal workspace.
            ASSERT (GB_IS_SPARSE (C) || GB_IS_HYPERSPARSE (C)) ;

            if (M == NULL)
            { 
                // C = A*B, no mask
                #define GB_NO_MASK 1
                #define GB_MASK_COMP 0
                #include "GB_AxB_saxpy3_template.c"
            }
            else if (!Mask_comp)
            { 
                // C<M> = A*B
                #define GB_NO_MASK 0
                #define GB_MASK_COMP 0
                #include "GB_AxB_saxpy3_template.c"
            }
            else
            { 
                // C<!M> = A*B
                #define GB_NO_MASK 0
                #define GB_MASK_COMP 1
                #include "GB_AxB_saxpy3_template.c"
            }
        }
        break ;

        case GB_SAXPY_METHOD_BITMAP :
        { 
            // C is bitmap or full
            #include "GB_bitmap_AxB_saxpy_template.c"
        }

        default:;
    }
}


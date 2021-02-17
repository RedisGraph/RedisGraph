//------------------------------------------------------------------------------
// GxB_Desc_set_mkl_template: set a field in a descriptor for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

        case GxB_DESCRIPTOR_MKL:

            {
                va_start (ap, field) ;
                desc->use_mkl = va_arg (ap, int) ;
                va_end (ap) ;
            }
            break ;


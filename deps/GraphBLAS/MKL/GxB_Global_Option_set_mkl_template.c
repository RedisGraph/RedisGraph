//------------------------------------------------------------------------------
// GxB_Global_Option_set_mkl_template: set a global option for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

        case GxB_GLOBAL_MKL:           // same as GxB_MKL

            {
                va_start (ap, field) ;
                int use_mkl = va_arg (ap, int) ;
                va_end (ap) ;
                GB_Global_use_mkl_set (use_mkl != 0) ;
            }
            break ;


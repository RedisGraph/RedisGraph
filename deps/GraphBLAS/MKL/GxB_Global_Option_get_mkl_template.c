//------------------------------------------------------------------------------
// GxB_Global_Option_get_mkl_template: get a global option for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

        case GxB_GLOBAL_MKL:            // same as GxB_MKL

            {
                va_start (ap, field) ;
                int *use_mkl = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (use_mkl) ;
                (*use_mkl) = GB_Global_use_mkl_get ( ) ;
            }
            break ;


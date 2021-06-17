//------------------------------------------------------------------------------
// GxB_Desc_get_mkl_template: get a field in a descriptor for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

        case GxB_DESCRIPTOR_MKL:

            {
                va_start (ap, field) ;
                int *use_mkl = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (use_mkl) ;
                (*use_mkl) = (desc == NULL) ? false : desc->use_mkl ;
            }
            break ;


//------------------------------------------------------------------------------
// GxB_Desc_get: get a field in a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GxB_Desc_get is a single va_arg-based method for any descriptor option,
// of any type.  The following functions are alternative methods that do not
// use va_arg (useful for compilers and interfaces that do not support va_arg):
//
//  GxB_Desc_get_INT32         int32_t scalars
//  GxB_Desc_get_FP64          double scalars

#include "GB.h"

//------------------------------------------------------------------------------
// GxB_Desc_get_INT32:  get a descriptor option (int32_t)
//------------------------------------------------------------------------------

GrB_Info GxB_Desc_get_INT32     // get a parameter from a descriptor
(
    GrB_Descriptor desc,        // descriptor to query; NULL is ok
    GrB_Desc_Field field,       // parameter to query
    int32_t *value              // return value of the descriptor
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Desc_get_INT32 (desc, field, &value)") ;
    GB_RETURN_IF_FAULTY (desc) ;
    GB_RETURN_IF_NULL (value) ;

    //--------------------------------------------------------------------------
    // get the parameter
    //--------------------------------------------------------------------------

    switch (field)
    {
        case GrB_OUTP : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->out) ;
            break ;

        case GrB_MASK : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->mask) ;
            break ;

        case GrB_INP0 : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->in0) ;
            break ;

        case GrB_INP1 : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->in1) ;
            break ;

        case GxB_DESCRIPTOR_NTHREADS :  // DEPRECATED

            (*value) = (int32_t) ((desc == NULL) ?
                    GxB_DEFAULT : desc->nthreads_max) ;
            break ;

        case GxB_AxB_METHOD : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->axb) ;
            break ;

        case GxB_SORT : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->do_sort);
            break ;

        case GxB_COMPRESSION : 

            (*value) = (int32_t) ((desc == NULL) ?
                GxB_DEFAULT : desc->compression) ; 
            break ;

        case GxB_IMPORT : 

            (*value) = (int32_t) ((desc == NULL) ? GxB_DEFAULT : desc->import) ;
            if ((*value) != GxB_DEFAULT) (*value) = GxB_SECURE_IMPORT ;
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GxB_Desc_get_FP64:  get a descriptor option (double)
//------------------------------------------------------------------------------

GrB_Info GxB_Desc_get_FP64      // get a parameter from a descriptor
(
    GrB_Descriptor desc,        // descriptor to query; NULL is ok
    GrB_Desc_Field field,       // parameter to query
    double *value               // return value of the descriptor
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Desc_get_FP64 (desc, field, &value)") ;
    GB_RETURN_IF_FAULTY (desc) ;
    GB_RETURN_IF_NULL (value) ;

    //--------------------------------------------------------------------------
    // get the parameter
    //--------------------------------------------------------------------------

    switch (field)
    {

        case GxB_DESCRIPTOR_CHUNK :     // DEPRECATED

            (*value) = (double) ((desc == NULL) ? GxB_DEFAULT : desc->chunk) ;
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GxB_Desc_get: based on va_arg
//------------------------------------------------------------------------------

GrB_Info GxB_Desc_get           // get a parameter from a descriptor
(
    GrB_Descriptor desc,        // descriptor to query; NULL is ok
    GrB_Desc_Field field,       // parameter to query
    ...                         // return value of the descriptor
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Desc_get (desc, field, &value)") ;
    GB_RETURN_IF_FAULTY (desc) ;

    //--------------------------------------------------------------------------
    // get the parameter
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {
        case GrB_OUTP : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value *value = va_arg (ap, GrB_Desc_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (value) ;
                (*value) = (desc == NULL) ? GxB_DEFAULT : desc->out ;
            }
            break ;

        case GrB_MASK : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value *value = va_arg (ap, GrB_Desc_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (value) ;
                (*value) = (desc == NULL) ? GxB_DEFAULT : desc->mask ;
            }
            break ;

        case GrB_INP0 : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value *value = va_arg (ap, GrB_Desc_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (value) ;
                (*value) = (desc == NULL) ? GxB_DEFAULT : desc->in0 ;
            }
            break ;

        case GrB_INP1 : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value *value = va_arg (ap, GrB_Desc_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (value) ;
                (*value) = (desc == NULL) ? GxB_DEFAULT : desc->in1 ;
            }
            break ;

        case GxB_DESCRIPTOR_NTHREADS :  // DEPRECATED

            {
                va_start (ap, field) ;
                int *nthreads = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (nthreads) ;
                int nth = (desc == NULL) ? GxB_DEFAULT : desc->nthreads_max ;
                (*nthreads) = nth ;
            }
            break ;

        case GxB_DESCRIPTOR_CHUNK :     // DEPRECATED

            {
                va_start (ap, field) ;
                double *chunk = va_arg (ap, double *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (chunk) ;
                (*chunk) = (desc == NULL) ? GxB_DEFAULT : desc->chunk ;
            }
            break ;

        case GxB_AxB_METHOD : 

            {
                va_start (ap, field) ;
                GrB_Desc_Value *value = va_arg (ap, GrB_Desc_Value *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (value) ;
                (*value) = (desc == NULL) ? GxB_DEFAULT : desc->axb ;
            }
            break ;

        case GxB_SORT : 

            {
                va_start (ap, field) ;
                int *do_sort = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (do_sort) ;
                int s = (desc == NULL) ? GxB_DEFAULT : desc->do_sort ;
                (*do_sort) = s ;
            }
            break ;

        case GxB_COMPRESSION : 

            {
                va_start (ap, field) ;
                int *compression = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (compression) ;
                int s = (desc == NULL) ? GxB_DEFAULT : desc->compression ;
                (*compression) = s ;
            }
            break ;

        case GxB_IMPORT : 

            {
                va_start (ap, field) ;
                int *method = va_arg (ap, int *) ;
                va_end (ap) ;
                GB_RETURN_IF_NULL (method) ;
                int s = (desc == NULL) ? GxB_DEFAULT : desc->import ;
                if (s != GxB_DEFAULT) s = GxB_SECURE_IMPORT ;
                (*method) = s ;
            }
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    #pragma omp flush
    return (GrB_SUCCESS) ;
}


//------------------------------------------------------------------------------
// GrB_error: return an error string describing the last error
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

//------------------------------------------------------------------------------
// status_code: convert GrB_Info enum into a string
//------------------------------------------------------------------------------

static const char *status_code ( )
{
    switch (GB_thread_local.info)
    {
        case GrB_SUCCESS             : return ("GrB_SUCCESS") ;
        case GrB_NO_VALUE            : return ("GrB_NO_VALUE") ;
        case GrB_UNINITIALIZED_OBJECT: return ("GrB_UNINITIALIZED_OBJECT") ;
        case GrB_INVALID_OBJECT      : return ("GrB_INVALID_OBJECT") ;
        case GrB_NULL_POINTER        : return ("GrB_NULL_POINTER") ;
        case GrB_INVALID_VALUE       : return ("GrB_INVALID_VALUE") ;
        case GrB_INVALID_INDEX       : return ("GrB_INVALID_INDEX") ;
        case GrB_DOMAIN_MISMATCH     : return ("GrB_DOMAIN_MISMATCH") ;
        case GrB_DIMENSION_MISMATCH  : return ("GrB_DIMENSION_MISMATCH") ;
        case GrB_OUTPUT_NOT_EMPTY    : return ("GrB_OUTPUT_NOT_EMPTY") ;
        case GrB_OUT_OF_MEMORY       : return ("GrB_OUT_OF_MEMORY") ;
        case GrB_INDEX_OUT_OF_BOUNDS : return ("GrB_INDEX_OUT_OF_BOUNDS") ;
        case GrB_PANIC               : return ("GrB_PANIC") ;
        default                      : return ("unknown!") ;
    }
}

//------------------------------------------------------------------------------
// GrB_error
//------------------------------------------------------------------------------

const char *GrB_error ( )       // return a string describing the last error
{

    //--------------------------------------------------------------------------
    // construct a string in thread local storage
    //--------------------------------------------------------------------------

    if (GB_thread_local.info == GrB_SUCCESS)
    {

        //----------------------------------------------------------------------
        // status is OK, print information about GraphBLAS
        //----------------------------------------------------------------------

        snprintf (GB_thread_local.report, GB_RLEN,
        "\n=================================================================\n"
        "%s"
        "SuiteSparse:GraphBLAS version: %d.%d.%d  Date: %s\n"
        "%s"
        "Conforms to GraphBLAS spec:    %d.%d.%d  Date: %s\n"
        "%s"
        "=================================================================\n"
        #ifndef NDEBUG
        "Debugging enabled; GraphBLAS will be very slow\n"
        #endif
        "GraphBLAS status: %s\n"
        "=================================================================\n",
        GXB_ABOUT,
        GXB_IMPLEMENTATION_MAJOR,
        GXB_IMPLEMENTATION_MINOR,
        GXB_IMPLEMENTATION_SUB,
        GXB_DATE,
        GXB_LICENSE,
        GXB_MAJOR,
        GXB_MINOR,
        GXB_SUB,
        GXB_SPEC_DATE,
        GXB_SPEC,
        status_code ( )) ;

    }
    else if (GB_thread_local.info == GrB_NO_VALUE)
    {

        //----------------------------------------------------------------------
        // 'no value' status
        //----------------------------------------------------------------------

        if (GB_thread_local.is_matrix)
        {

        snprintf (GB_thread_local.report, GB_RLEN,
        "\n=================================================================\n"
        "GraphBLAS status: %s\nGraphBLAS function: GrB_Matrix_extractElement\n"
        "No entry A(%" PRIu64 ",%" PRIu64 ") present in the matrix.\n"
        "=================================================================\n",
        status_code ( ), GB_thread_local.row, GB_thread_local.col) ;

        }

        else
        {

        snprintf (GB_thread_local.report, GB_RLEN,
        "\n=================================================================\n"
        "GraphBLAS status: %s\nGraphBLAS function: GrB_Vector_extractElement\n"
        "No entry v(%" PRIu64 ") present in the vector.\n"
        "=================================================================\n",
        status_code ( ), GB_thread_local.row) ;

        }

    }
    else
    {

        //----------------------------------------------------------------------
        // error status
        //----------------------------------------------------------------------

        snprintf (GB_thread_local.report, GB_RLEN,
        "\n=================================================================\n"
        "GraphBLAS error: %s\nfunction: %s\n%s\n"
        #ifdef DEVELOPER
        "Line: %d in file: %s\n"
        #endif
        "=================================================================\n",
        status_code ( ),
        GB_thread_local.where, GB_thread_local.details
        #ifdef DEVELOPER
        , GB_thread_local.line, GB_thread_local.file
        #endif
        ) ;

    }

    //--------------------------------------------------------------------------
    // return the string to the user
    //--------------------------------------------------------------------------

    return (GB_thread_local.report) ;
}


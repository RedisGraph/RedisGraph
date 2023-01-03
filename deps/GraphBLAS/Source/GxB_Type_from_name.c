//------------------------------------------------------------------------------
// GxB_Type_from_name: return a built-in GrB_Type from its name
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GxB_Type_from_name returns the built-in GrB_Type corresponding to the
// C name of the type as a string.  For user-defined types, type is returned
// as NULL.  This is not an error condition.  This allows the user to write
// code such as this:

/*
    typedef struct { double x ; char stuff [16] ; } myfirsttype ;
    typedef struct { float z [4][4] ; int color ; } myquaternion ;
    GrB_Type MyType1, MyQType ;
    GxB_Type_new (&MyType1, sizeof (myfirsttype), "myfirsttype",
        "typedef struct { double x ; char stuff [16] ; } myfirsttype ;") ;
    GxB_Type_new (&MyQType, sizeof (myquaternion), "myquaternion",
        "typedef struct { float z [4][4] ; int color ; } myquaternion ;") ;

    GrB_Matrix A ;
    // ... create a matrix A of some built-in or user-defined type

    // later on, to query the type of A:
    size_t typesize ;
    GxB_Type_size (&typesize, type) ;       // works for any type
    GrB_Type atype ;
    char atype_name [GxB_MAX_NAME_LEN] ;
    GxB_Matrix_type_name (atype_name, A) ;
    GxB_Type_from_name (&atype, atype_name) ;
    if (atype == NULL)
    {
        // This is not yet an error.  It means that A has a user-defined type.
        if ((strcmp (atype_name, "myfirsttype")) == 0) atype = MyType1 ;
        else if ((strcmp (atype_name, "myquaternion")) == 0) atype = MyQType ;
        else { ... this is now an error ... the type of A is unknown.  }
        }
    }
*/

// The alternative to this approach is a single function:
//
//      GxB_Matrix_type (&type, A)
//
// which returns the GrB_Type of a GrB_Matrix A.  This has several problems.
// The type, even for built-ins, is an ephemeral pointer.  It cannot be shared
// across two processes, say with an MPI message or by writing a serialized
// matrix to a file or a pipe and reading it in later.  A string (the
// type_name) can be safely passed between these processes but a pointer (the
// GrB_Type) cannot.  Once a receiving process has the type_name string,
// obtained from a file, or another process, it can safely reconstruct the
// corresponding GrB_Type with tests such as the ones below, or in the example
// above.  This cannot be safely done with GxB_Matrix_type.

// As a result, the GxB_Matrix_type function that appears in SuiteSparse
// GraphBLAS has been declared "historical" and its use is discouraged.  It
// won't be removed, to preserve backward compatibility, but it will eventually
// removed from the user guide.  Use the string-based type mechanism instead.

#include "GB.h"

GrB_Info GxB_Type_from_name     // return the GrB_Type from a name
(
    GrB_Type *type,             // built-in type, or NULL if not recognized
    const char *type_name       // array of size at least GxB_MAX_NAME_LEN
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL (type) ;
    GB_RETURN_IF_NULL (type_name) ;

    //--------------------------------------------------------------------------
    // determine the GrB_Type from its name
    //--------------------------------------------------------------------------

    #define MATCH(s) (strncmp (type_name, s, GxB_MAX_NAME_LEN) == 0)

    if      (MATCH ("bool"          )) (*type) = GrB_BOOL   ;
    else if (MATCH ("int8_t"        )) (*type) = GrB_INT8   ;
    else if (MATCH ("int16_t"       )) (*type) = GrB_INT16  ;
    else if (MATCH ("int32_t"       )) (*type) = GrB_INT32  ;
    else if (MATCH ("int64_t"       )) (*type) = GrB_INT64  ;
    else if (MATCH ("uint8_t"       )) (*type) = GrB_UINT8  ;
    else if (MATCH ("uint16_t"      )) (*type) = GrB_UINT16 ;
    else if (MATCH ("uint32_t"      )) (*type) = GrB_UINT32 ;
    else if (MATCH ("uint64_t"      )) (*type) = GrB_UINT64 ;
    else if (MATCH ("float"         )) (*type) = GrB_FP32   ;
    else if (MATCH ("double"        )) (*type) = GrB_FP64   ;
    else if (MATCH ("float complex" )) (*type) = GxB_FC32   ;
    else if (MATCH ("GxB_FC32_t"    )) (*type) = GxB_FC32   ;
    else if (MATCH ("double complex")) (*type) = GxB_FC64   ;
    else if (MATCH ("GxB_FC64_t"    )) (*type) = GxB_FC64   ;
    else
    {
        // This is not an error.  Returning type as NULL means that A has a
        // user-defined type.  GraphBLAS does not keep a registry of
        // user-defined types, so let the user application match the name to
        // the user-defined type (see example above).
        (*type) = NULL ;
    }

    return (GrB_SUCCESS) ;
}


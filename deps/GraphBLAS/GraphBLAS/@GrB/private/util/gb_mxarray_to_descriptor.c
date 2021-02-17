//------------------------------------------------------------------------------
// gb_mxarray_to_descriptor: get the contents of a GraphBLAS Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// get a GraphBLAS descriptor from a MATLAB struct.

#include "gb_matlab.h"

#define LEN 100

static void get_descriptor
(
    GrB_Descriptor desc,            // GraphBLAS descriptor to modify
    const mxArray *desc_matlab,     // MATLAB struct with d.out, etc
    const char *fieldname,          // fieldname to extract from desc_matlab
    const GrB_Desc_Field field      // field to set in desc
)
{

    // find the field in the MATLAB struct
    int fieldnumber = mxGetFieldNumber (desc_matlab, fieldname) ;
    if (fieldnumber >= 0)
    {

        // the field is present
        mxArray *value = mxGetFieldByNumber (desc_matlab, 0, fieldnumber) ;

        if (MATCH (fieldname, "nthreads"))
        { 

            // nthreads must be a numeric scalar
            CHECK_ERROR (!gb_mxarray_is_scalar (value),
                "d.nthreads must be a scalar") ;
            int nthreads_max = (int) mxGetScalar (value) ;
            OK (GxB_Desc_set (desc, GxB_NTHREADS, nthreads_max)) ;

        }
        else if (MATCH (fieldname, "chunk"))
        { 

            // chunk must be a numeric scalar
            CHECK_ERROR (!gb_mxarray_is_scalar (value),
                "d.chunk must be a scalar") ;
            double chunk = mxGetScalar (value) ;
            OK (GxB_Desc_set (desc, GxB_CHUNK, chunk)) ;

        }
        else
        {

            // get the string from the MATLAB field
            char s [LEN+2] ;
            gb_mxstring_to_string (s, LEN, value, "field") ;

            // convert the string to a Descriptor value, and set the value
            if (MATCH (s, "default"))
            { 
                OK (GxB_Desc_set (desc, field, GxB_DEFAULT)) ;
            }
            else if (MATCH (s, "transpose"))
            { 
                OK (GxB_Desc_set (desc, field, GrB_TRAN)) ;
            }
            else if (MATCH (s, "complement"))
            { 
                OK (GxB_Desc_set (desc, field, GrB_COMP)) ;
            }
            else if (MATCH (s, "structure") || MATCH (s, "structural"))
            { 
                OK (GxB_Desc_set (desc, field, GrB_STRUCTURE)) ;
            }
            else if (MATCH (s, "structural complement"))
            { 
                OK (GxB_Desc_set (desc, field, GrB_COMP+GrB_STRUCTURE)) ;
            }
            else if (MATCH (s, "replace"))
            { 
                OK (GxB_Desc_set (desc, field, GrB_REPLACE)) ;
            }
            else if (MATCH (s, "gustavson"))
            { 
                OK (GxB_Desc_set (desc, field, GxB_AxB_GUSTAVSON)) ;
            }
            else if (MATCH (s, "dot"))
            { 
                OK (GxB_Desc_set (desc, field, GxB_AxB_DOT)) ;
            }
            else if (MATCH (s, "saxpy"))
            { 
                OK (GxB_Desc_set (desc, field, GxB_AxB_SAXPY)) ;
            }
            else if (MATCH (s, "hash"))
            { 
                OK (GxB_Desc_set (desc, field, GxB_AxB_HASH)) ;
            }
            else
            { 
                // the string must be one of the strings listed above
                ERROR ("unrecognized descriptor value") ;
            }
        }
    }
}

//------------------------------------------------------------------------------
// gb_mxarray_to_descriptor
//------------------------------------------------------------------------------

GrB_Descriptor gb_mxarray_to_descriptor // new descriptor, or NULL if none
(
    const mxArray *desc_matlab, // MATLAB struct with possible descriptor
    kind_enum_t *kind,          // GrB, sparse, or full
    GxB_Format_Value *fmt,      // by row or by col
    int *sparsity,              // hypersparse/sparse/bitmap/full
    base_enum_t *base           // 0-based int, 1-based int, or 1-based double
)
{

    //--------------------------------------------------------------------------
    // check inputs and find the descriptor
    //--------------------------------------------------------------------------

    // set the defaults if no descriptor is present
    (*kind) = KIND_GRB ;
    (*fmt) = GxB_NO_FORMAT ;
    (*base) = BASE_DEFAULT ;
    (*sparsity) = 0 ;

    if (desc_matlab == NULL || !mxIsStruct (desc_matlab)
        || mxGetField (desc_matlab, 0, "GraphBLASv4") != NULL)
    {
        // If present, the descriptor is a struct whose first field is not
        // "desc.GraphBLASv4" (since that is a GrB matrix struct, not a
        // descriptor).  If not present, the GraphBLAS descriptor is NULL.
        // This is not an error.
        return (NULL) ;
    }

    //--------------------------------------------------------------------------
    // create the GraphBLAS descriptor
    //--------------------------------------------------------------------------

    GrB_Descriptor desc ;
    OK (GrB_Descriptor_new (&desc)) ;

    // get each component of the descriptor struct
    get_descriptor (desc, desc_matlab, "out"     , GrB_OUTP) ;
    get_descriptor (desc, desc_matlab, "in0"     , GrB_INP0) ;
    get_descriptor (desc, desc_matlab, "in1"     , GrB_INP1) ;
    get_descriptor (desc, desc_matlab, "mask"    , GrB_MASK) ;
    get_descriptor (desc, desc_matlab, "axb"     , GxB_AxB_METHOD) ;
    get_descriptor (desc, desc_matlab, "nthreads", GxB_NTHREADS) ;
    get_descriptor (desc, desc_matlab, "chunk"   , GxB_CHUNK) ;

    //--------------------------------------------------------------------------
    // get the desired kind of output
    //--------------------------------------------------------------------------

    mxArray *mxkind = mxGetField (desc_matlab, 0, "kind") ;
    if (mxkind != NULL)
    {
        // get the string from the MATLAB field
        char s [LEN+2] ;
        gb_mxstring_to_string (s, LEN, mxkind, "kind") ;
        if (MATCH (s, "grb") || MATCH (s, "default"))
        { 
            (*kind) = KIND_GRB ;        // @GrB matrix
        }
        else if (MATCH (s, "sparse"))
        { 
            (*kind) = KIND_SPARSE ;     // MATLAB sparse matrix
        }
        else if (MATCH (s, "full"))
        { 
            (*kind) = KIND_FULL ;       // MATLAB full matrix
        }
        else if (MATCH (s, "matlab"))
        {
            (*kind) = KIND_MATLAB ;     // MATLAB sparse or full matrix
        }
        else
        { 
            ERROR ("invalid descriptor.kind") ;
        }
    }

    //--------------------------------------------------------------------------
    // get the desired format of output, if any
    //--------------------------------------------------------------------------

    mxArray *mxfmt = mxGetField (desc_matlab, 0, "format") ;
    if (mxfmt != NULL)
    {
        bool ok = gb_mxstring_to_format (mxfmt, fmt, sparsity) ;
        if (!ok)
        { 
            ERROR ("unknown format") ;
        }
    }

    //--------------------------------------------------------------------------
    // get the desired base
    //--------------------------------------------------------------------------

    mxArray *mxbase = mxGetField (desc_matlab, 0, "base") ;
    if (mxbase != NULL)
    {
        // get the string from the MATLAB field
        char s [LEN+2] ;
        gb_mxstring_to_string (s, LEN, mxbase, "base") ;
        if (MATCH (s, "default"))
        { 
            // The indices are one-based by default.  The type is determined
            // automatically:  if I and J are outputs, then the type is double
            // (BASE_1_DOUBLE) unless the indices can exceed flintmax (in which
            // case BASE_1_INT64 is used)
            (*base) = BASE_DEFAULT ;
        }
        else if (MATCH (s, "zero-based"))
        { 
            // zero-based indices are always int64.  This is performance
            // purposes, internal to GrB methods.  The user may also use this
            // to speed up GrB.build, GrB.extract, GrB.assign. and
            // GrB.subassign.
            (*base) = BASE_0_INT64 ;
        }
        else if (MATCH (s, "one-based int"))
        { 
            // one-based indices, but in int64.  These are important for
            // indexing into matrices with dimension larger than flintmax.
            (*base) = BASE_1_INT64 ;
        }
        else if (MATCH (s, "one-based") || MATCH (s, "one-based double"))
        { 
            // for 'one-based', the caller may change this to BASE_1_INT64,
            // if I and J are inputs to the function are int64.  This is
            // the typical default.
            (*base) = BASE_1_DOUBLE ;
        }

        else
        { 
            ERROR ("invalid descriptor.base") ;
        }
    }

    //--------------------------------------------------------------------------
    // return the non-null Descriptor to the caller
    //--------------------------------------------------------------------------

    return (desc) ;
}


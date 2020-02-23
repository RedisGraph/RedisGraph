//------------------------------------------------------------------------------
// gb_mxarray_to_descriptor: get the contents of a GraphBLAS Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// get a GraphBLAS descriptor from a MATLAB struct.

#include "gb_matlab.h"

#define LEN 100

static void get_descriptor
(
    GrB_Descriptor D,               // GraphBLAS descriptor to modify
    const mxArray *D_matlab,        // MATLAB struct with d.out, etc
    const char *fieldname,          // fieldname to extract from D_matlab
    const GrB_Desc_Field field      // field to set in D
)
{

    // find the field in the MATLAB struct
    int fieldnumber = mxGetFieldNumber (D_matlab, fieldname) ;
    if (fieldnumber >= 0)
    {

        // the field is present
        mxArray *value = mxGetFieldByNumber (D_matlab, 0, fieldnumber) ;

        if (MATCH (fieldname, "nthreads"))
        { 

            // nthreads must be a numeric scalar
            CHECK_ERROR (!gb_mxarray_is_scalar (value),
                "d.nthreads must be a scalar") ;
            int nthreads_max = (int) mxGetScalar (value) ;
            OK (GxB_Desc_set (D, GxB_NTHREADS, nthreads_max)) ;

        }
        else if (MATCH (fieldname, "chunk"))
        { 

            // chunk must be a numeric scalar
            CHECK_ERROR (!gb_mxarray_is_scalar (value),
                "d.chunk must be a scalar") ;
            double chunk = mxGetScalar (value) ;
            OK (GxB_Desc_set (D, GxB_CHUNK, chunk)) ;

        }
        else
        {

            // get the string from the MATLAB field
            char s [LEN+2] ;
            gb_mxstring_to_string (s, LEN, value, "field") ;

            // convert the string to a Descriptor value, and set the value
            if (MATCH (s, "default"))
            { 
                OK (GxB_Desc_set (D, field, GxB_DEFAULT)) ;
            }
            else if (MATCH (s, "transpose"))
            { 
                OK (GxB_Desc_set (D, field, GrB_TRAN)) ;
            }
            else if (MATCH (s, "complement") || MATCH (s, "comp"))
            { 
                OK (GxB_Desc_set (D, field, GrB_COMP)) ;
            }
            else if (MATCH (s, "structure") || MATCH (s, "structural"))
            { 
                OK (GxB_Desc_set (D, field, GrB_STRUCTURE)) ;
            }
            else if (MATCH (s, "structural complement"))
            { 
                OK (GxB_Desc_set (D, field, GrB_COMP + GrB_STRUCTURE)) ;
            }
            else if (MATCH (s, "replace"))
            { 
                OK (GxB_Desc_set (D, field, GrB_REPLACE)) ;
            }
            else if (MATCH (s, "gustavson"))
            { 
                OK (GxB_Desc_set (D, field, GxB_AxB_GUSTAVSON)) ;
            }
            else if (MATCH (s, "dot"))
            { 
                OK (GxB_Desc_set (D, field, GxB_AxB_DOT)) ;
            }
            else if (MATCH (s, "saxpy"))
            { 
                OK (GxB_Desc_set (D, field, GxB_AxB_SAXPY)) ;
            }
            else if (MATCH (s, "heap"))
            { 
                OK (GxB_Desc_set (D, field, GxB_AxB_HEAP)) ;
            }
            else if (MATCH (s, "hash"))
            { 
                OK (GxB_Desc_set (D, field, GxB_AxB_HASH)) ;
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

GrB_Descriptor gb_mxarray_to_descriptor     // return a new descriptor
(
    const mxArray *D_matlab,    // MATLAB struct
    kind_enum_t *kind,          // GrB, sparse, or full
    GxB_Format_Value *fmt,      // by row or by col
    base_enum_t *base           // 0-based int, 1-based int, or 1-based double
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // By default, all mexFunctions return a GraphBLAS struct, to be wrapped in
    // a GrB object in GrB.m.
    (*kind) = KIND_GRB ;

    CHECK_ERROR (D_matlab == NULL || !mxIsStruct (D_matlab),
        "descriptor must be a struct") ;

    //--------------------------------------------------------------------------
    // create the GraphBLAS descriptor
    //--------------------------------------------------------------------------

    GrB_Descriptor D ;
    OK (GrB_Descriptor_new (&D)) ;

    // get each component of the descriptor struct
    get_descriptor (D, D_matlab, "out"     , GrB_OUTP) ;
    get_descriptor (D, D_matlab, "in0"     , GrB_INP0) ;
    get_descriptor (D, D_matlab, "in1"     , GrB_INP1) ;
    get_descriptor (D, D_matlab, "mask"    , GrB_MASK) ;
    get_descriptor (D, D_matlab, "axb"     , GxB_AxB_METHOD) ;
    get_descriptor (D, D_matlab, "nthreads", GxB_NTHREADS) ;
    get_descriptor (D, D_matlab, "chunk"   , GxB_CHUNK) ;

    //--------------------------------------------------------------------------
    // get the desired kind of output
    //--------------------------------------------------------------------------

    mxArray *mxkind = mxGetField (D_matlab, 0, "kind") ;
    if (mxkind != NULL)
    {
        // get the string from the MATLAB field
        char s [LEN+2] ;
        gb_mxstring_to_string (s, LEN, mxkind, "kind") ;
        if (MATCH (s, "grb") || MATCH (s, "default"))
        { 
            (*kind) = KIND_GRB ;
        }
        else if (MATCH (s, "sparse"))
        { 
            (*kind) = KIND_SPARSE ;
        }
        else if (MATCH (s, "full"))
        { 
            (*kind) = KIND_FULL ;
        }
        else
        { 
            ERROR ("invalid descriptor.kind") ;
        }
    }

    //--------------------------------------------------------------------------
    // get the desired format of output, if any
    //--------------------------------------------------------------------------

    (*fmt) = GxB_NO_FORMAT ;
    mxArray *mxfmt = mxGetField (D_matlab, 0, "format") ;
    if (mxfmt != NULL)
    {
        (*fmt) = gb_mxstring_to_format (mxfmt) ;
        if ((*fmt) == GxB_NO_FORMAT)
        { 
            ERROR ("unknown format") ;
        }
    }

    //--------------------------------------------------------------------------
    // get the desired base
    //--------------------------------------------------------------------------

    (*base) = BASE_DEFAULT ;
    mxArray *mxbase = mxGetField (D_matlab, 0, "base") ;
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

    return (D) ;
}


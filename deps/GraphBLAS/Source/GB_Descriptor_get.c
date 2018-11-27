//------------------------------------------------------------------------------
// GB_Descriptor_get: get the status of a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A descriptor modifies how the behavoir of a GraphBLAS operation.  In the
// current GraphBLAS spec, the following descriptor fields may be set.

//  Descriptor field:           Descriptor value:

//  desc->out                   GxB_DEFAULT or GrB_REPLACE

//      GrB_REPLACE means that the output matrix C is cleared just
//      prior to writing results back into it, via C<Mask> = results.  This
//      descriptor does not affect how C is used to compute the results.  If
//      GxB_DEFAULT, then C is not cleared before doing C<Mask>=results.

//  desc->mask                  GxB_DEFAULT or GrB_SCMP

//      An optional 'write mask' defines how the results are to be written back
//      into C.  The boolean Mask matrix has the same size as C (Mask is
//      typecasted to boolean if it has another type).  If the Mask input to
//      the GraphBLAS method is NULL, then implicitly Mask(i,j)=1 for all i and
//      j.  Let Z be the results to be written into C (the same dimension as
//      C).  If desc->mask is GxB_DEFAULT, and Mask(i,j)=1, then C(i,j) is
//      over-written with Z(i,j).  Otherwise, if Mask(i,j)=0 C(i,j) is left
//      unmodified (it remains an implicit zero if it is so, or its value is
//      unchanged if it has one).  If desc->mask is GrB_SCMP, then the use of
//      Mask is negated: Mask(i,j)=0 means that C(i,j) is overwritten with
//      Z(i,j), and Mask(i,j)=1 means that C(i,j) is left unchanged.

//      Writing results Z into C via the Mask is written as C<Mask>=Z in
//      GraphBLAS notation.

//      Note that it is the value of Mask(i,j) that determines how C(i,j) is
//      overwritten.  If the (i,j) entry is present in the Mask matrix data
//      structure but has a numerical value of zero, then it is the same as if
//      (i,j) is not present and thus implicitly zero.  Both mean 'Mask(i,j)=0'
//      in the description above of how the Mask works.

//  desc->in0 and desc->in1     GxB_DEFAULT or GrB_TRAN

//      A GrB_Matrix passed as an input parameter to GraphBLAS methods can
//      optionally transpose them prior to using them.  desc->in0 always refers
//      to the first input to the method, and desc->in1 always refers to the
//      second one.

//      If the value of this descriptor is GxB_DEFAULT, then the matrix is used
//      as-is.  Otherwise, it is transposed first.  That is, the results are
//      the same as if the transpose of the matrix was passed to the method.

//  desc->axb                   see GraphBLAS.h; can be:

//      GrB_DEFAULT         automatic selection
//      GxB_AxB_GUSTAVSON   gather-scatter saxpy method
//      GxB_AxB_HEAP        heap-based saxpy method
//      GxB_AxB_DOT         dot product

#include "GB.h"

GrB_Info GB_Descriptor_get      // get the contents of a descriptor
(
    const GrB_Descriptor desc,  // descriptor to query, may be NULL
    bool *C_replace,            // if true replace C before C<Mask>=Z
    bool *Mask_comp,            // if true use logical negation of Mask
    bool *In0_transpose,        // if true transpose first input
    bool *In1_transpose,        // if true transpose second input
    GrB_Desc_Value *AxB_method, // method for C=A*B
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // desc may be null, but if not NULL it must be initialized
    GB_RETURN_IF_FAULTY (desc) ;

    //--------------------------------------------------------------------------
    // get the contents of the descriptor
    //--------------------------------------------------------------------------

    // default values if descriptor is NULL
    GrB_Desc_Value C_desc    = GxB_DEFAULT ;
    GrB_Desc_Value Mask_desc = GxB_DEFAULT ;
    GrB_Desc_Value In0_desc  = GxB_DEFAULT ;
    GrB_Desc_Value In1_desc  = GxB_DEFAULT ;
    GrB_Desc_Value AxB_desc  = GxB_DEFAULT ;

    // non-defaults descriptors
    if (desc != NULL)
    { 
        // get the contents
        C_desc    = desc->out ;   // DEFAULT or REPLACE
        Mask_desc = desc->mask ;  // DEFAULT or SCMP
        In0_desc  = desc->in0 ;   // DEFAULT or TRAN
        In1_desc  = desc->in1 ;   // DEFAULT or TRAN
        AxB_desc  = desc->axb ;   // DEFAULT, GUSTAVSON, HEAP, or DOT
    }

    // check for valid values of each descriptor field
    if (!(C_desc    == GxB_DEFAULT || C_desc    == GrB_REPLACE) ||
        !(Mask_desc == GxB_DEFAULT || Mask_desc == GrB_SCMP) ||
        !(In0_desc  == GxB_DEFAULT || In0_desc  == GrB_TRAN) ||
        !(In1_desc  == GxB_DEFAULT || In1_desc  == GrB_TRAN) ||
        !(AxB_desc  == GxB_DEFAULT || AxB_desc  == GxB_AxB_GUSTAVSON ||
          AxB_desc  == GxB_AxB_DOT || AxB_desc  == GxB_AxB_HEAP))
    { 
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG, "Descriptor invalid"))) ;
    }

    if (C_replace != NULL)
    { 
        *C_replace = (C_desc == GrB_REPLACE) ;
    }
    if (Mask_comp != NULL)
    { 
        *Mask_comp = (Mask_desc == GrB_SCMP) ;
    }
    if (In0_transpose != NULL)
    { 
        *In0_transpose = (In0_desc == GrB_TRAN) ;
    }
    if (In1_transpose != NULL)
    { 
        *In1_transpose = (In1_desc == GrB_TRAN) ;
    }
    if (AxB_method != NULL)
    { 
        *AxB_method = AxB_desc ;
    }

    return (GrB_SUCCESS) ;
}


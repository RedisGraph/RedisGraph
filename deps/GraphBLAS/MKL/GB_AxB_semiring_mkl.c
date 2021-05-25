//------------------------------------------------------------------------------
// GB_AxB_semiring_mkl: map a GraphBLAS semiring to an Intel MKL semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This function maps the 1,438 semrings available in GraphBLAS to the 13
// semirings that currently appear in the Intel MKL_graph library.

#include "GB_mkl.h"

#if GB_HAS_MKL_GRAPH

int GB_AxB_semiring_mkl         // return the MKL semiring, or -1 if none.
(
    GB_Opcode add_opcode,       // additive monoid
    GB_Opcode mult_opcode,      // multiply operator
    GB_Opcode xycode            // type of x for z = mult (x,y), except for
                                // z = SECOND(x,y) = y, where xycode is the
                                // type of y
)
{

    //--------------------------------------------------------------------------
    // determine the MKL_graph semiring
    //--------------------------------------------------------------------------

    int no = -1 ;

    switch (add_opcode)
    {

        //----------------------------------------------------------------------
        case GB_PLUS_opcode:    // PLUS monoid
        //----------------------------------------------------------------------

            // 218 semirings with the PLUS monoid.

            switch (mult_opcode)
            {

#if 0

                case GB_FIRST_opcode:       // z = x

                    switch (xycode)         // PLUS_FIRST semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }
#endif

                case GB_SECOND_opcode:      // z = y

                    switch (xycode)         // PLUS_SECOND semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code:  return (no) ;
                        case GB_FP32_code:    return (GB_MKL_GRAPH_SEMIRING_PLUS_SECOND_FP32) ;   // added in 2021.1.beta6
                        case GB_FP64_code:    
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

#if 0

                case GB_PAIR_opcode:        // z = 1

                    switch (xycode)         // PLUS_PAIR semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_MIN_opcode:         // z = min(x,y)

                    switch (xycode)         // PLUS_MIN semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MAX_opcode:         // z = max(x,y)

                    switch (xycode)         // PLUS_MAX semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_PLUS_opcode:        // z = x + y

                    switch (xycode)         // PLUS_PLUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }
#endif

                case GB_TIMES_opcode:       // z = x * y

                    switch (xycode)         // PLUS_TIMES semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:   return (no) ;
                        case GB_INT32_code:   return (MKL_GRAPH_SEMIRING_PLUS_TIMES_INT32) ;
                        case GB_INT64_code:   return (MKL_GRAPH_SEMIRING_PLUS_TIMES_INT64) ;
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code:  return (no) ;
                        case GB_FP32_code:    return (MKL_GRAPH_SEMIRING_PLUS_TIMES_FP32) ;
                        case GB_FP64_code:    return (MKL_GRAPH_SEMIRING_PLUS_TIMES_FP64) ;
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

#if 0

                case GB_MINUS_opcode:       // z = x - y

                    switch (xycode)         // PLUS_MINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_RMINUS_opcode:      // z = y - x (reverse minus)

                    switch (xycode)         // PLUS_RMINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_DIV_opcode:         // z = x / y

                    switch (xycode)         // PLUS_DIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_RDIV_opcode:        // z = y / x (reverse division)

                    switch (xycode)         // PLUS_RDIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_ISEQ_opcode:        // z = (x == y)

                    switch (xycode)         // PLUS_ISEQ semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISNE_opcode:        // z = (x != y)

                    switch (xycode)         // PLUS_ISEQ semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGT_opcode:        // z = (x >  y)

                    switch (xycode)         // PLUS_ISGT semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLT_opcode:        // z = (x <  y)

                    switch (xycode)         // PLUS_ISLT semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGE_opcode:        // z = (x >= y)

                    switch (xycode)         // PLUS_ISGE semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLE_opcode:        // z = (x <= y)

                    switch (xycode)         // PLUS_ISLE semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // z = x || y

                    switch (xycode)         // PLUS_LOR semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // z = x && y

                    switch (xycode)         // PLUS_LAND semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // z = x != y

                    switch (xycode)         // PLUS_LXOR semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }
#endif

                default:                    return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_TIMES_opcode:   // TIMES monoid
        //----------------------------------------------------------------------

            // 206 semirings with the TIMES monoid.
            // same switch table as PLUS, except no PAIR multiplier

#if 0
            switch (mult_opcode)
            {
                case GB_FIRST_opcode:       // z = x

                    switch (xycode)         // TIMES_FIRST semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // z = y

                    switch (xycode)         // TIMES_SECOND semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_MIN_opcode:         // z = min(x,y)

                    switch (xycode)         // TIMES_MIN semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MAX_opcode:         // z = max(x,y)

                    switch (xycode)         // TIMES_MAX semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_PLUS_opcode:        // z = x + y

                    switch (xycode)         // TIMES_PLUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_TIMES_opcode:       // z = x * y

                    switch (xycode)         // TIMES_TIMES semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_MINUS_opcode:       // z = x - y

                    switch (xycode)         // TIMES_MINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_RMINUS_opcode:      // z = y - x (reverse minus)

                    switch (xycode)         // TIMES_RMINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_DIV_opcode:         // z = x / y

                    switch (xycode)         // TIMES_DIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_RDIV_opcode:        // z = y / x (reverse division)

                    switch (xycode)         // TIMES_RDIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_ISEQ_opcode:        // z = (x == y)

                    switch (xycode)         // TIMES_ISEQ semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISNE_opcode:        // z = (x != y)

                    switch (xycode)         // TIMES_ISEQ semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGT_opcode:        // z = (x >  y)

                    switch (xycode)         // TIMES_ISGT semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLT_opcode:        // z = (x <  y)

                    switch (xycode)         // TIMES_ISLT semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGE_opcode:        // z = (x >= y)

                    switch (xycode)         // TIMES_ISGE semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLE_opcode:        // z = (x <= y)

                    switch (xycode)         // TIMES_ISLE semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // z = x || y

                    switch (xycode)         // TIMES_LOR semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // z = x && y

                    switch (xycode)         // TIMES_LAND semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // z = x != y

                    switch (xycode)         // TIMES_LXOR semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                default:                    return (no) ;
            }
#endif

            return (no) ;

        //----------------------------------------------------------------------
        case GB_MIN_opcode:     // MIN monoid
        //----------------------------------------------------------------------

            // 190 semirings with the MIN monoid.
            // same table as TIMES, except no complex cases.

            switch (mult_opcode)
            {

#if 0
                case GB_FIRST_opcode:       // z = x

                    switch (xycode)         // MIN_FIRST semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // z = y

                    switch (xycode)         // MIN_SECOND semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MIN_opcode:         // z = min(x,y)

                    switch (xycode)         // MIN_MIN semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MAX_opcode:         // z = max(x,y)

                    switch (xycode)         // MIN_MAX semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }
#endif

                case GB_PLUS_opcode:        // z = x + y

                    switch (xycode)         // MIN_PLUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:   return (no) ;
                        case GB_INT32_code:   return (MKL_GRAPH_SEMIRING_MIN_PLUS_INT32) ;
                        case GB_INT64_code:   return (MKL_GRAPH_SEMIRING_MIN_PLUS_INT64) ;
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code:  return (no) ;
                        case GB_FP32_code:    return (MKL_GRAPH_SEMIRING_MIN_PLUS_FP32) ;
                        case GB_FP64_code:    return (MKL_GRAPH_SEMIRING_MIN_PLUS_FP64) ;
                        default:              return (no) ;
                    }

#if 0
                case GB_TIMES_opcode:       // z = x * y

                    switch (xycode)         // MIN_TIMES semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MINUS_opcode:       // z = x - y

                    switch (xycode)         // MIN_MINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_RMINUS_opcode:      // z = y - x (reverse minus)

                    switch (xycode)         // MIN_RMINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_DIV_opcode:         // z = x / y

                    switch (xycode)         // MIN_DIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_RDIV_opcode:        // z = y / x (reverse division)

                    switch (xycode)         // MIN_RDIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISEQ_opcode:        // z = (x == y)

                    switch (xycode)         // MIN_ISEQ semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISNE_opcode:        // z = (x != y)

                    switch (xycode)         // MIN_ISNE semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGT_opcode:        // z = (x >  y)

                    switch (xycode)         // MIN_ISGT semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLT_opcode:        // z = (x <  y)

                    switch (xycode)         // MIN_ISLT semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGE_opcode:        // z = (x >= y)

                    switch (xycode)         // MIN_ISGE semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLE_opcode:        // z = (x <= y)

                    switch (xycode)         // MIN_ISGE semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // z = x || y

                    switch (xycode)         // MIN_LOR semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // z = x && y

                    switch (xycode)         // MIN_LAND semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // z = x != y

                    switch (xycode)         // MIN_LXOR semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }
#endif

                default:                    return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_MAX_opcode:     // MAX monoid
        //----------------------------------------------------------------------

            // 190 semirings with the MAX monoid.  same table as MIN.

            switch (mult_opcode)
            {

                case GB_FIRST_opcode:       // z = x

                    switch (xycode)         // MAX_FIRST semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:   return (no) ;
                        case GB_INT32_code:   return (MKL_GRAPH_SEMIRING_MAX_FIRST_INT32) ;
                        case GB_INT64_code:   return (MKL_GRAPH_SEMIRING_MAX_FIRST_INT64) ;
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code:  return (no) ;
                        case GB_FP32_code:    return (MKL_GRAPH_SEMIRING_MAX_FIRST_FP32) ;
                        case GB_FP64_code:    return (MKL_GRAPH_SEMIRING_MAX_FIRST_FP64) ;
                        default:              return (no) ;
                    }
#if 0

                case GB_SECOND_opcode:      // z = y

                    switch (xycode)         // MAX_SECOND semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MIN_opcode:         // z = min(x,y)

                    switch (xycode)         // MAX_MIN semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MAX_opcode:         // z = max(x,y)

                    switch (xycode)         // MAX_MAX semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_PLUS_opcode:        // z = x + y

                    switch (xycode)         // MAX_PLUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_TIMES_opcode:       // z = x * y

                    switch (xycode)         // MAX_TIMES semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MINUS_opcode:       // z = x - y

                    switch (xycode)         // MAX_MINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_RMINUS_opcode:      // z = y - x (reverse minus)

                    switch (xycode)         // MAX_RMINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_DIV_opcode:         // z = x / y

                    switch (xycode)         // MAX_DIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_RDIV_opcode:        // z = y / x (reverse division)

                    switch (xycode)         // MAX_RDIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISEQ_opcode:        // z = (x == y)

                    switch (xycode)         // MAX_ISEQ semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISNE_opcode:        // z = (x != y)

                    switch (xycode)         // MAX_ISNE semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGT_opcode:        // z = (x >  y)

                    switch (xycode)         // MAX_ISGT semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLT_opcode:        // z = (x <  y)

                    switch (xycode)         // MAX_ISLT semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGE_opcode:        // z = (x >= y)

                    switch (xycode)         // MAX_ISGE semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLE_opcode:        // z = (x <= y)

                    switch (xycode)         // MAX_ISGE semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // z = x || y

                    switch (xycode)         // MAX_LOR semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // z = x && y

                    switch (xycode)         // MAX_LAND semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // z = x != y

                    switch (xycode)         // MAX_LXOR semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }
#endif

                default:                    return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_LOR_opcode:     // OR monoid
        //----------------------------------------------------------------------

            // 70 semirings with the LOR monoid

            switch (mult_opcode)
            {

#if 0
                case GB_FIRST_opcode:       // LOR_FIRST_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // LOR_SECOND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // LOR_LOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }
#endif

                case GB_LAND_opcode:        // LOR_LAND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:    return (MKL_GRAPH_SEMIRING_LOR_LAND_BOOL ) ;
                        default:              return (no) ;
                    }

#if 0
                case GB_LXOR_opcode:        // LOR_LXOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_EQ_opcode:          // z = (x == y)

                    switch (xycode)         // LOR_EQ semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_NE_opcode:          // z = (x != y)

                    switch (xycode)         // LOR_NE semirings
                    {
                        // LOR_NE_BOOL is just LOR_LXOR_BOOL
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GT_opcode:          // z = (x >  y)

                    switch (xycode)         // LOR_GT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LT_opcode:          // z = (x <  y)

                    switch (xycode)         // LOR_LT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GE_opcode:          // z = (x >= y)

                    switch (xycode)         // LOR_GE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LE_opcode:          // z = (x <= y)

                    switch (xycode)         // LOR_LE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }
#endif

                default:                    return (no) ;
            }

#if 0

        //----------------------------------------------------------------------
        case GB_LAND_opcode:    // AND monoid
        //----------------------------------------------------------------------

            // 70 semirings with the LAND monoid.  Same table as LOR

            switch (mult_opcode)
            {

                case GB_FIRST_opcode:       // LAND_FIRST_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // LAND_SECOND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // LAND_LOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // LAND_LAND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // LAND_LXOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_EQ_opcode:          // z = (x == y)

                    switch (xycode)         // LAND_EQ semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_NE_opcode:          // z = (x != y)

                    switch (xycode)         // LAND_NE semirings
                    {
                        // LAND_NE_BOOL is just LAND_LXOR_BOOL
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GT_opcode:          // z = (x >  y)

                    switch (xycode)         // LAND_GT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LT_opcode:          // z = (x <  y)

                    switch (xycode)         // LAND_LT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GE_opcode:          // z = (x >= y)

                    switch (xycode)         // LAND_GE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LE_opcode:          // z = (x <= y)

                    switch (xycode)         // LAND_LE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                default:                    return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_EQ_opcode:      // EQ, XNOR monoid
        //----------------------------------------------------------------------

            // 70 semirings with the EQ/XNOR monoid.  Same table as LOR

            switch (mult_opcode)
            {

                case GB_FIRST_opcode:       // EQ_FIRST_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // EQ_SECOND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // EQ_LOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // EQ_LAND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // EQ_LXOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_EQ_opcode:          // z = (x == y)

                    switch (xycode)         // EQ_EQ semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_NE_opcode:          // z = (x != y)

                    switch (xycode)         // EQ_NE semirings
                    {
                        // EQ_NE_BOOL is just EQ_LXOR_BOOL
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GT_opcode:          // z = (x >  y)

                    switch (xycode)         // EQ_GT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LT_opcode:          // z = (x <  y)

                    switch (xycode)         // EQ_LT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GE_opcode:          // z = (x >= y)

                    switch (xycode)         // EQ_GE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LE_opcode:          // z = (x <= y)

                    switch (xycode)         // EQ_LE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                default:                    return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_LXOR_opcode:    // XOR monoid
        //----------------------------------------------------------------------

            // 71 semirings with the LXOR monoid.  Same table as LOR,
            // but with the LXOR_PAIR_BOOL semiring as well.

            switch (mult_opcode)
            {

                case GB_FIRST_opcode:       // LXOR_FIRST_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // LXOR_SECOND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_PAIR_opcode:        // LXOR_PAIR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // LXOR_LOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // LXOR_LAND_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // LXOR_LXOR_BOOL semiring

                    switch (xycode)
                    {
                        case GB_BOOL_code:   
                        default:              return (no) ;
                    }

                case GB_EQ_opcode:          // z = (x == y)

                    switch (xycode)         // LXOR_EQ semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_NE_opcode:          // z = (x != y)

                    switch (xycode)         // LXOR_NE semirings
                    {
                        // LXOR_NE_BOOL is just LXOR_LXOR_BOOL
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GT_opcode:          // z = (x >  y)

                    switch (xycode)         // LXOR_GT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LT_opcode:          // z = (x <  y)

                    switch (xycode)         // LXOR_LT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GE_opcode:          // z = (x >= y)

                    switch (xycode)         // LXOR_GE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LE_opcode:          // z = (x <= y)

                    switch (xycode)         // LXOR_LE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                default:                    return (no) ;
            }
            return (no) ;

        //----------------------------------------------------------------------
        case GB_ANY_opcode:     // ANY monoid
        //----------------------------------------------------------------------

            // 289 semirings with the ANY monoid.

            switch (mult_opcode)
            {

                case GB_FIRST_opcode:       // z = x

                    switch (xycode)         // ANY_FIRST semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_SECOND_opcode:      // z = y

                    switch (xycode)         // ANY_SECOND semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_PAIR_opcode:        // z = 1

                    switch (xycode)         // ANY_PAIR semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_MIN_opcode:         // z = min(x,y)

                    switch (xycode)         // ANY_MIN semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_MAX_opcode:         // z = max(x,y)

                    switch (xycode)         // ANY_MAX semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_PLUS_opcode:        // z = x + y

                    switch (xycode)         // ANY_PLUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_TIMES_opcode:       // z = x * y

                    switch (xycode)         // ANY_TIMES semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_MINUS_opcode:       // z = x - y

                    switch (xycode)         // ANY_MINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_RMINUS_opcode:      // z = y - x (reverse minus)

                    switch (xycode)         // ANY_RMINUS semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_DIV_opcode:         // z = x / y

                    switch (xycode)         // ANY_DIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_RDIV_opcode:        // z = y / x (reverse division)

                    switch (xycode)         // ANY_RDIV semirings
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        case GB_FC32_code:   
                        case GB_FC64_code:   
                        default:              return (no) ;
                    }

                case GB_ISEQ_opcode:        // z = (x == y)

                    switch (xycode)         // ANY_ISEQ semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISNE_opcode:        // z = (x != y)

                    switch (xycode)         // ANY_ISEQ semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGT_opcode:        // z = (x >  y)

                    switch (xycode)         // ANY_ISGT semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLT_opcode:        // z = (x <  y)

                    switch (xycode)         // ANY_ISLT semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISGE_opcode:        // z = (x >= y)

                    switch (xycode)         // ANY_ISGE semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_ISLE_opcode:        // z = (x <= y)

                    switch (xycode)         // ANY_ISLE semirings (no complex)
                    {
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_EQ_opcode:          // z = (x == y)

                    switch (xycode)         // ANY_EQ semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_NE_opcode:          // z = (x != y)

                    switch (xycode)         // ANY_NE semirings
                    {
                        // ANY_NE_BOOL is just ANY_LXOR_BOOL, below
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GT_opcode:          // z = (x >  y)

                    switch (xycode)         // ANY_GT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LT_opcode:          // z = (x <  y)

                    switch (xycode)         // ANY_LT semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_GE_opcode:          // z = (x >= y)

                    switch (xycode)         // ANY_GE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LE_opcode:          // z = (x <= y)

                    switch (xycode)         // ANY_LE semirings
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LOR_opcode:         // z = x || y

                    switch (xycode)         // ANY_LOR semirings (no complex)
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LAND_opcode:        // z = x && y

                    switch (xycode)         // ANY_LAND semirings (no complex)
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                case GB_LXOR_opcode:        // z = x != y

                    switch (xycode)         // ANY_LXOR semirings (no complex)
                    {
                        case GB_BOOL_code:   
                        case GB_INT8_code:   
                        case GB_INT16_code:  
                        case GB_INT32_code:  
                        case GB_INT64_code:  
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        case GB_FP32_code:   
                        case GB_FP64_code:   
                        default:              return (no) ;
                    }

                default:                    return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_BOR_opcode:     // BOR monoid
        //----------------------------------------------------------------------

            // 16 semirings with the BOR monoid.
            switch (mult_opcode)
            {
                case GB_BOR_opcode:         // z = (x | y), bitwise or

                    switch (xycode)         // BOR_BOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BAND_opcode:        // z = (x & y), bitwise and

                    switch (xycode)         // BOR_BAND semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXOR_opcode:        // z = (x ^ y), bitwise xor

                    switch (xycode)         // BOR_BXOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXNOR_opcode:       // z = ~(x ^ y), bitwise xnor

                    switch (xycode)         // BOR_BXNOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                default:                 return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_BAND_opcode:    // BAND monoid
        //----------------------------------------------------------------------

            // 16 semirings with the BAND monoid.
            switch (mult_opcode)
            {
                case GB_BOR_opcode:         // z = (x | y), bitwise or

                    switch (xycode)         // BAND_BOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BAND_opcode:        // z = (x & y), bitwise and

                    switch (xycode)         // BAND_BAND semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXOR_opcode:        // z = (x ^ y), bitwise xor

                    switch (xycode)         // BAND_BXOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXNOR_opcode:       // z = ~(x ^ y), bitwise xnor

                    switch (xycode)         // BAND_BXNOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                default:                 return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_BXOR_opcode:    // BXOR monoid
        //----------------------------------------------------------------------

            // 16 semirings with the BXOR monoid.
            switch (mult_opcode)
            {
                case GB_BOR_opcode:         // z = (x | y), bitwise or

                    switch (xycode)         // BXOR_BOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BAND_opcode:        // z = (x & y), bitwise and

                    switch (xycode)         // BXOR_BAND semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXOR_opcode:        // z = (x ^ y), bitwise xor

                    switch (xycode)         // BXOR_BXOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXNOR_opcode:       // z = ~(x ^ y), bitwise xnor

                    switch (xycode)         // BXOR_BXNOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                default:                 return (no) ;
            }

        //----------------------------------------------------------------------
        case GB_BXNOR_opcode:   // BXNOR monoid
        //----------------------------------------------------------------------

            // 16 semirings with the BXNOR monoid.
            switch (mult_opcode)
            {
                case GB_BOR_opcode:         // z = (x | y), bitwise or

                    switch (xycode)         // BXNOR_BOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BAND_opcode:        // z = (x & y), bitwise and

                    switch (xycode)         // BXNOR_BAND semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXOR_opcode:        // z = (x ^ y), bitwise xor

                    switch (xycode)         // BXNOR_BXOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                case GB_BXNOR_opcode:       // z = ~(x ^ y), bitwise xnor

                    switch (xycode)         // BXNOR_BXNOR semirings
                    {
                        case GB_UINT8_code:  
                        case GB_UINT16_code: 
                        case GB_UINT32_code: 
                        case GB_UINT64_code: 
                        default:              return (no) ;
                    }

                default:                 return (no) ;
            }
#endif

        //----------------------------------------------------------------------
        default:               return (no) ;
        //----------------------------------------------------------------------

    }
}

#endif


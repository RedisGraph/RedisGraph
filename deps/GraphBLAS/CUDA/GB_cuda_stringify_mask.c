//SPDX-License-Identifier: Apache-2.0

//#include "GB_cuda.h"
#include "GB.h"
#include "GB_cuda_stringify.h"

const char *GB_cuda_stringify_mask
(
    const GB_Type_code M_type_code,
    bool mask_is_structural
)
{

    if (mask_is_structural)
    {
        return (
            "#define GB_MTYPE void\n"
            "#define MX(i) true") ;
    }
    else
    {
        switch (M_type_code)
        {
            case GB_BOOL_code:
            case GB_INT8_code:
            case GB_UINT8_code:
                return (
                    "#define GB_MTYPE uint8_t\n"
                    "#define MX(i) Mx [i]") ;

            case GB_INT16_code:
            case GB_UINT16_code:
                return (
                    "#define GB_MTYPE uint16_t\n"
                    "#define MX(i) Mx [i]") ;

            case GB_INT32_code:
            case GB_UINT32_code:
//          case GB_FC32_code:
            case GB_FP32_code:
                return (
                    "#define GB_MTYPE uint32_t\n"
                    "#define MX(i) Mx [i]") ;

            case GB_INT64_code:
            case GB_UINT64_code:
//          case GB_FC64_code:
            case GB_FP64_code:
                return (
                    "#define GB_MTYPE uint64_t\n"
                    "#define MX(i) Mx [i]") ;

//          case GB_FC64_code:
//              return (
//                  "#define GB_MTYPE double complex\n"
//                  "#define MX(i) Mx [i]") ;

            default: ;
        }
    }
    
    // unrecognized type
    return (NULL) ;
}


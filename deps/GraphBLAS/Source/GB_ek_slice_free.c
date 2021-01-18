//------------------------------------------------------------------------------
// GB_ek_slice_free: free workspace created by GB_ek_slice
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_ek_slice.h"

void GB_ek_slice_free
(
    int64_t *GB_RESTRICT *pstart_slice_handle,
    int64_t *GB_RESTRICT *kfirst_slice_handle,
    int64_t *GB_RESTRICT *klast_slice_handle
)
{ 
    GB_FREE (*pstart_slice_handle) ;
    GB_FREE (*kfirst_slice_handle) ;
    GB_FREE (*klast_slice_handle) ;
}


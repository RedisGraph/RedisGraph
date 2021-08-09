//------------------------------------------------------------------------------
// GB_cuda_global.cpp: accessor functions for global GraphBLAS/CUDA variables
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS/CUDA, (c) NVIDIA Corp. 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_cuda_stringifier.hpp"

//Global definition required here, sorry
GB_cuda_stringifier *SR_callback_ptr;   // thunk

std::istream* callback_wrapper
(
    std::string file_name,      // string with the requested "file" name
    std::iostream& file_stream  // the I/O stream for the "file" contents
)
{
    return SR_callback_ptr->callback (file_name, file_stream) ;
}



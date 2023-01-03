// Class to manage both stringify functions from mxm, ops and monoids to char buffers
// Also provides a iostream callback to deliver the buffer to jitify as if read from a file

// (c) Nvidia Corp. 2020 All rights reserved
// SPDX-License-Identifier: Apache-2.0

// Implementations of string callbacks
#pragma once

#include <iostream>
#include <cstdint>
#include "GB_jit_cache.h"

extern "C"
{
#include "GB.h"
#include "GB_stringify.h"
}

// FIXME: delegate problem generation to data factory
class GB_cuda_reduce_factory: public jit::File_Desc {

public:
    uint64_t rcode;
    GrB_Monoid reduce ;
    GrB_Type atype ;

    // file ptr
    FILE *fp;

    void open( const char *path_and_file, const char *mode)
    {
        fp = fopen( path_and_file, mode);
    }

    void close( )
    {
        fclose( fp );
    }


    void reduce_factory(GrB_Monoid reduce, GrB_Matrix A)
    {
        uint64_t rcode;
        GB_enumify_reduce (
                // output:
                &rcode,         // unique encoding of entire monoid
                // input:
                reduce,
                A
        ) ;

        this->rcode = rcode;
        this->reduce = reduce ;
        this->atype = A->type ;

        // FIXME: use GB_namify_problem
        std::stringstream ss;
        ss << "GB_reduce_" << this->rcode << ".h";

        std::string new_filename = ss.str();
        filename.resize(new_filename.size());
        strcpy(filename.data(), new_filename.data());
    }

//------------------------------------------------------------------------------
// Macrofy takes a code and creates the corresponding string macros for
// operators, datatypes, sparsity formats and produces a character buffer.
//------------------------------------------------------------------------------

    void macrofy ( ) override
    {
        GB_macrofy_reduce (
                // output to file :
                fp,
                // input:
                this->rcode,
                this->reduce,
                this->atype
        ) ;
    }


}; // GB_cuda_reduce_factory


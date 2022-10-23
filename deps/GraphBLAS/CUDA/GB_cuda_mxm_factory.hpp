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
    #include "GB_binop.h"
    #include "GB_stringify.h"
}

// Define function pointer we will use later
//std::istream* (*file_callback)(std::string, std::iostream&);

// Define a factory class for building any mxm text definitions

// FIXME: delegate problem generation to data factory
class GB_cuda_mxm_factory: public jit::File_Desc {

    public:
        uint64_t sr_code;
        GrB_Semiring semiring ;
        GrB_Type ctype, atype, btype ;

        // file ptr
        FILE *fp;

    void open( const char *path_and_file, const char *mode)
    {
//        std::cout<< "opening "<< path_and_file<<" for write"<< std::endl;
        fp = fopen( path_and_file, mode);
    }

    void close( )
    {
        fclose( fp );
    }

    //--------------------------------------------------------------------------
    // mxm_factory takes a set of inputs describing and operation (semiring,
    // mask, datatypes, sparsity formats, etc) and produces a numerical unique
    // value for those This allows rapid lookups to see if we have handled this
    // case before, and avoids the need to generate and manage strings at this
    // stage.
    //--------------------------------------------------------------------------

    // FIXME: pass in user's C_in matrix, in case C_in<M>+=A*B can be done
    //        in-place 
    // FIXME: handle hypersparse case in dot3

    void mxm_factory
    (  
        // C matrix:
        bool C_iso,             // true if C is iso-valued
        int C_sparsity,         // sparsity structure of C
        GrB_Type ctype,         // the type of C
        // M matrix:
        GrB_Matrix M,           // may be NULL
        bool Mask_struct,       // mask is structural
        bool Mask_comp,         // mask is complemented
        // semiring:
        GrB_Semiring semiring,  // the semiring to enumify
        bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
        // A and B:
        GrB_Matrix A,
        GrB_Matrix B
    )
    {
//       std::cout<<" calling stringify semiring: " << std::endl;
//     GxB_Semiring_fprint (semiring, "stringfiy the smiering", GxB_COMPLETE, stdout) ;
//       std::cout<<" Mask_struct: " << Mask_struct << std::endl;
       uint64_t scode; 
       GB_enumify_mxm (
	    // output:
	    &scode,         // unique encoding of the entire semiring
	    // input:
            C_iso,          // true if C is iso-valued
	    C_sparsity,     // sparsity structure of C
	    ctype,          // the type of C
            // M matrix:
            M,
	    Mask_struct,    // mask is structural
	    Mask_comp,      // mask is complemented
            // semiring:
	    semiring,      // the semiring to enumify
	    flipxy,        // multiplier is: mult(a,b) or mult(b,a)
            // A and B:
            A,
            B
       ) ;

//       printf("scode=%lu\n", scode);
//       std::cout << "done stringify mxm" << std::endl;
       this->sr_code = scode;

       this->semiring = semiring ;
       this->atype = A->type ;
       this->btype = B->type ;
       this->ctype = ctype ;

       std::stringstream ss;
       // FIXME: use GB_namify_problem here:
       ss << "GB_mxm_" << this->sr_code << ".h";

       std::string new_filename = ss.str();
       filename.resize(new_filename.size());
       strcpy(filename.data(), new_filename.data());

//       std::cout<<" returned from  stringify mxm"<< std::endl;
    }

//------------------------------------------------------------------------------
// Macrofy takes a code and creates the corresponding string macros for 
// operators, datatypes, sparsity formats and produces a character buffer. 
//------------------------------------------------------------------------------

    void macrofy ( ) override
    {
//       std::cout<<" calling macrofy mxm. sr_code="<< this->sr_code << std::endl;
       GB_macrofy_mxm (
	    // output to file :
	    fp, 
	    // input:
	    this->sr_code,
	    this->semiring,
	    this->ctype,
	    this->atype,
	    this->btype
       ) ;
//       std::cout<<" returned from  macrofy mxm"<< std::endl;

    }


}; // GB_cuda_mxm_factory


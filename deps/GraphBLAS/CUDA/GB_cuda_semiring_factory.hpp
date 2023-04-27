// Class to manage both stringify functions from semiring, ops and monoids to char buffers
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

// Define a factory class for building any semiring text definitions
class GB_cuda_semiring_factory: public jit::File_Desc {

    public:

        uint64_t sr_code;

        // file ptr
        FILE *fp;

    void open( const char *path_and_file, const char *mode)
    {
        std::cout<< "opening "<< path_and_file<<" for write"<< std::endl;  
        fp = fopen( path_and_file, mode);
    }

    void close( )
    {
        fclose( fp );
    }

    //--------------------------------------------------------------------------
    //semiring_factor takes a set of inputs describing and operation (semiring,
    //mask, datatypes, sparsity formats) and produces a numerical unique value
    //for those This allows rapid lookups to see if we have handled this case
    //before, and avoids the need to generate and manage strings at this stage.
    //--------------------------------------------------------------------------

    void semiring_factory 
    (  
        // input:
        GrB_Semiring semiring,  // the semiring to enumify
        bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
        GrB_Type ctype,         // the type of C
        GrB_Type mtype,         // the type of M, or NULL if no mask
        GrB_Type atype,         // the type of A
        GrB_Type btype,         // the type of B
        bool Mask_struct,       // mask is structural
        bool Mask_comp,         // mask is complemented
        int C_sparsity,         // sparsity structure of C
        int M_sparsity,         // sparsity structure of M
        int A_sparsity,         // sparsity structure of A
        int B_sparsity          // sparsity structure of B
    )
    {
       std::cout<<" calling stringify semiring: " << semiring << std::endl;
       uint64_t scode; 
       GB_enumify_semiring (
	    // output:
	    &scode,         // unique encoding of the entire semiring
	    // input:
	    semiring,      // the semiring to enumify
	    flipxy,        // multiplier is: mult(a,b) or mult(b,a)
	    ctype,         // the type of C
	    mtype,         // the type of M, or NULL if no mask
	    atype,         // the type of A
	    btype,         // the type of B
	    Mask_struct,   // mask is structural
	    Mask_comp,     // mask is complemented
	    C_sparsity,    // sparsity structure of C
	    M_sparsity,    // sparsity structure of M
	    A_sparsity,    // sparsity structure of A
	    B_sparsity     // sparsity structure of B
       ) ;

       printf("scode=%uld\n", scode);
       std::cout << "done stringify semiring" << std::endl;
       this->sr_code = scode;

       std::stringstream ss;
       ss << "GB_semiring_" << this->sr_code << ".h";

       std::string new_filename = ss.str();
       filename.resize(new_filename.size());
       strcpy(filename.data(), new_filename.data());

       std::cout<<" returned from  stringify semiring"<< std::endl;
    }

//------------------------------------------------------------------------------
// Macrofy takes a code and creates the corresponding string macros for 
// operators, datatypes, sparsity formats and produces a character buffer. 
//------------------------------------------------------------------------------

    void macrofy ( ) override
    {
       std::cout<<" calling macrofy semiring. sr_code="<< this->sr_code << std::endl;
       GB_macrofy_semiring (
	    // output to file :
	    fp, 
	    // input:
	    this->sr_code  
       ) ;
       std::cout<<" returned from  macrofy semiring"<< std::endl; 

    }


}; // GB_cuda_semiring_factory


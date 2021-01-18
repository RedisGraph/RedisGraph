// SPDX-License-Identifier: Apache-2.0

// Implementations of string callbacks
#include <iostream>
#pragma once
// Define function pointer we will use later
//std::istream* (*file_callback)(std::string, std::iostream&);

// Define a factory class for building any buffer of text
class GB_callback {
  char *callback_string;
  const char *include_filename;
  public:
     void load_string(const char *fname, char *input){
        callback_string = input; 
        include_filename =  fname;
     }
     std::istream* callback( std::string filename, std::iostream& tmp_stream) {
        if ( filename == std::string(this->include_filename) )
        {
           tmp_stream << this->callback_string; 
           return &tmp_stream;
        }
        else 
        {
           return nullptr;
        }
     }
};


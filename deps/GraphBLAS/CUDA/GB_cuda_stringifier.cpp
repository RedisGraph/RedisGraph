// Class to manage both stringify functions from semiring, ops and monoids to char buffers
// Also provides a iostream callback to deliver the buffer to jitify as if read from a file

// (c) Nvidia Corp. 2020 All rights reserved 
// SPDX-License-Identifier: Apache-2.0

// Implementations of string callbacks
#pragma once
#include <iostream>
#include "GB.h"
#include "GB_cuda_stringify.h"

// Define function pointer we will use later
//std::istream* (*file_callback)(std::string, std::iostream&);

// Define a factory class for building any buffer of text
class GB_cuda_stringifier {
  char callback_buffer[2048];
  char *callback_string;
  const char *include_filename;

  public:

//------------------------------------------------------------------------------
// load string: set string and file name to mimic
//------------------------------------------------------------------------------
    void load_string(const char *fname, char *input)
    {
        callback_string = input; 
        include_filename =  fname;
    }

//------------------------------------------------------------------------------
// callback: return string as if it was read from a file 
//------------------------------------------------------------------------------

    std::istream* callback( std::string filename, std::iostream& tmp_stream) 
    {
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

//------------------------------------------------------------------------------
// stringify_identity: return string for identity value
//------------------------------------------------------------------------------
#define  ID( x)  IDENT = (x)
    void stringify_identity 
    (
        // output:
        char *code_string,  // string with the #define macro
        // input:
        GB_Opcode opcode,     // must be a built-in binary operator from a monoid
        GB_Type_code zcode    // type code used in the opcode we want
    )
    {
        const char *IDENT;
        switch (opcode)
        {
            case GB_MIN_opcode :

                switch (zcode)
                {
                    case GB_BOOL_code   : ID ("true") ;     // boolean AND
                    case GB_INT8_code   : ID ("INT8_MAX") ;
                    case GB_INT16_code  : ID ("INT16_MAX") ;
                    case GB_INT32_code  : ID ("INT32_MAX") ;
                    case GB_INT64_code  : ID ("INT64_MAX") ;
                    case GB_UINT8_code  : ID ("UINT8_MAX") ;
                    case GB_UINT16_code : ID ("UINT16_MAX") ;
                    case GB_UINT32_code : ID ("UINT32_MAX") ;
                    case GB_UINT64_code : ID ("UINT64_MAX") ;
                    default             : ID ("INFINITY") ;
                }
                break ;

            case GB_MAX_opcode :

                switch (zcode)
                {
                    case GB_BOOL_code   : ID ("false") ;    // boolean OR
                    case GB_INT8_code   : ID ("INT8_MIN") ;
                    case GB_INT16_code  : ID ("INT16_MIN") ;
                    case GB_INT32_code  : ID ("INT32_MIN") ;
                    case GB_INT64_code  : ID ("INT64_MIN") ;
                    case GB_UINT8_code  : ID ("0") ;
                    case GB_UINT16_code : ID ("0") ;
                    case GB_UINT32_code : ID ("0") ;
                    case GB_UINT64_code : ID ("0") ;
                    default             : ID ("(-INFINITY)") ;
                }
                break ;

            case GB_PLUS_opcode     : ID ("0") ;
            case GB_TIMES_opcode    : ID ("1") ;
            case GB_LOR_opcode      : ID ("false") ;
            case GB_LAND_opcode     : ID ("true") ;
            case GB_LXOR_opcode     : ID ("false") ;
            // case GB_LXNOR_opcode :
            case GB_EQ_opcode       : ID ("true") ;
            // case GB_ANY_opcode   :
            default                 : ID ("0") ;
        }
        snprintf (code_string, GB_CUDA_STRLEN, "#define GB_IDENTITY (%s)", IDENT) ;

    }

    
    const char *GB_cuda_stringify_opcode
    (
    GB_Opcode opcode    // opcode of GraphBLAS operator
    )
    {
        switch (opcode)
        {
            case GB_FIRST_opcode :  return ("1st") ;
            // case GB_ANY_opcode : return ("any") ;
            case GB_SECOND_opcode : return ("2nd") ;
            case GB_MIN_opcode :    return ("min") ;
            case GB_MAX_opcode :    return ("max") ;
            case GB_PLUS_opcode :   return ("plus") ;
            case GB_MINUS_opcode :  return ("minus") ;
            case GB_RMINUS_opcode : return ("rminus") ;
            case GB_TIMES_opcode :  return ("times") ;
            case GB_DIV_opcode :    return ("div") ;
            case GB_RDIV_opcode :   return ("rdiv") ;
            case GB_EQ_opcode :     return ("eq") ;
            case GB_ISEQ_opcode :   return ("iseq") ;
            case GB_NE_opcode :     return ("ne") ;
            case GB_ISNE_opcode :   return ("isne") ;
            case GB_GT_opcode :     return ("gt") ;
            case GB_ISGT_opcode :   return ("isgt") ;
            case GB_LT_opcode :     return ("lt") ;
            case GB_ISLT_opcode :   return ("islt") ;
            case GB_GE_opcode :     return ("ge") ;
            case GB_ISGE_opcode :   return ("isge") ;
            case GB_LE_opcode :     return ("le") ;
            case GB_ISLE_opcode :   return ("isle") ;
            case GB_LOR_opcode :    return ("lor") ;
            case GB_LAND_opcode :   return ("land") ;
            case GB_LXOR_opcode :   return ("lxor") ;
            // case GB_BOR_opcode : ... bitwise ops
            // x | y, etc
            // case GB_PAIR_opcode :
            default :  ;
        }

        return ("") ;
    }
   
    void stringify_binop 
    (
        // output:
        char *code_string,  // string with the #define macro
        // input:
        const char *macro_name,   // name of macro to construct
        GB_Opcode opcode,   // opcode of GraphBLAS operator to convert into a macro
        GB_Type_code zcode  // op->ztype->code of the operator
    )
    {

    // The binop macro generates an expression, not a full statement (there
    // is no semicolon).

    // for example:
    // #define GB_MULT(x,y) ((x) * (y))

        const char *f ;

        switch (opcode)
        {

            case GB_FIRST_opcode :    //  7: z = x

                f = "(x)" ;
                break ;

            // case GB_ANY_opcode :
            case GB_SECOND_opcode :   //  8: z = y

                f = "(y)" ;
                break ;

            case GB_MIN_opcode :      //  9: z = min(x,y)

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) && (y)" ;
                    case GB_FP32_code    : f = "fminf (x,y)" ;
                    case GB_FP64_code    : f = "fmin (x,y)" ;
                    default              : f = "GB_IMIN (x,y)" ;
                }
                break ;

            case GB_MAX_opcode :      // 10: z = max(x,y)

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) || (y)" ;
                    case GB_FP32_code    : f = "fmaxf (x,y)" ;
                    case GB_FP64_code    : f = "fmax (x,y)" ;
                    default              : f = "GB_IMAX (x,y)" ;
                }
                break ;

            case GB_PLUS_opcode :     // 11: z = x + y

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) || (y)" ;
                    default              : f = "(x) + (y)" ;
                }
                break ;

            case GB_MINUS_opcode :    // 12: z = x - y

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) != (y)" ;
                    default              : f = "(x) - (y)" ;
                }
                break ;

            case GB_RMINUS_opcode :   // 13: z = y - x

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) != (y)" ;
                    default              : f = "(y) - (x)" ;
                }
                break ;

            case GB_TIMES_opcode :    // 14: z = x * y

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) && (y)" ;
                    default              : f = "(x) * (y)" ;
                }
                break ;

            case GB_DIV_opcode :      // 15: z = x / y ;

                switch (zcode)
                {
                    case GB_BOOL_code   : f = "(x)" ;
                    case GB_INT8_code   : f = "GB_IDIV_SIGNED (x,y,8)" ;
                    case GB_INT16_code  : f = "GB_IDIV_SIGNED (x,y,16)" ;
                    case GB_INT32_code  : f = "GB_IDIV_SIGNED (x,y,32)" ;
                    case GB_INT64_code  : f = "GB_IDIV_SIGNED (x,y,64)" ;
                    case GB_UINT8_code  : f = "GB_IDIV_UNSIGNED (x,y,8)" ;
                    case GB_UINT16_code : f = "GB_IDIV_UNSIGNED (x,y,16)" ;
                    case GB_UINT32_code : f = "GB_IDIV_UNSIGNED (x,y,32)" ;
                    case GB_UINT64_code : f = "GB_IDIV_UNSIGNED (x,y,64)" ;
                    default             : f = "(x) / (y)" ;
                }
                break ;

            case GB_RDIV_opcode :      // z = y / x ;

                switch (zcode)
                {
                    case GB_BOOL_code   : f = "(x)" ;
                    case GB_INT8_code   : f = "GB_IDIV_SIGNED (y,x,8)" ;
                    case GB_INT16_code  : f = "GB_IDIV_SIGNED (y,x,16)" ;
                    case GB_INT32_code  : f = "GB_IDIV_SIGNED (y,x,32)" ;
                    case GB_INT64_code  : f = "GB_IDIV_SIGNED (y,x,64)" ;
                    case GB_UINT8_code  : f = "GB_IDIV_UNSIGNED (y,x,8)" ;
                    case GB_UINT16_code : f = "GB_IDIV_UNSIGNED (y,x,16)" ;
                    case GB_UINT32_code : f = "GB_IDIV_UNSIGNED (y,x,32)" ;
                    case GB_UINT64_code : f = "GB_IDIV_UNSIGNED (y,x,64)" ;
                    default             : f = "(y) / (x)" ;
                }
                break ;

            case GB_EQ_opcode :
            case GB_ISEQ_opcode :     // 17: z = (x == y)

                f = "(x) == (y)" ;
                break ;

            case GB_NE_opcode :
            case GB_ISNE_opcode :     // 18: z = (x != y)

                f = "(x) != (y)" ;
                break ;

            case GB_GT_opcode :
            case GB_ISGT_opcode :     // 19: z = (x >  y)

                f = "(x) > (y)" ;
                break ;

            case GB_LT_opcode :
            case GB_ISLT_opcode :     // 20: z = (x <  y)

                f = "(x) < (y)" ;
                break ;

            case GB_GE_opcode :
            case GB_ISGE_opcode :     // 21: z = (x >= y)

                f = "(x) >= (y)" ;
                break ;

            case GB_LE_opcode :
            case GB_ISLE_opcode :     // 22: z = (x <= y)

                f = "(x) <= (y)" ;
                break ;

            case GB_LOR_opcode :      // 23: z = (x != 0) || (y != 0)

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) || (y)" ;
                    default              : f = "((x) != 0) || ((y) != 0)" ;
                }
                break ;

            case GB_LAND_opcode :     // 23: z = (x != 0) && (y != 0)

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) && (y)" ;
                    default              : f = "((x) != 0) && ((y) != 0)" ;
                }
                break ;

            case GB_LXOR_opcode :     // 25: z = (x != 0) != (y != 0)

                switch (zcode)
                {
                    case GB_BOOL_code    : f = "(x) != (y)" ;
                    default              : f = "((x) != 0) != ((y) != 0)" ;
                }
                break ;

            // case GB_BOR_opcode : ... bitwise ops
            // x | y, etc

            // case GB_PAIR_opcode :
            default :
                
                f = "1" ;
                break ;
        }

        snprintf (code_string, GB_CUDA_STRLEN,
            "#define %s(x,y) (%s)", macro_name, f) ;
    }


    void stringify_terminal 
    (
        // outputs:
        bool *is_monoid_terminal,
        char *terminal_condition,
        char *terminal_statement,
        // inputs:
        const char *macro_condition_name,
        const char *macro_statement_name,
        GB_Opcode opcode,    // must be a built-in binary operator from a monoid
        GB_Type_code zcode   // op->ztype->code
    )
    {
    //------------------------------------------------------------------------------
    // GB_cuda_stringify_terminal: string to check terminal condition
    //------------------------------------------------------------------------------

    // The macro_condition_name(cij) should return true if the value of cij has
    // reached its terminal value, or false otherwise.  If the monoid is not
    // terminal, then the macro should always return false.  The ANY monoid
    // should always return true.

    // The macro_statement_name is a macro containing a full statement.  If the
    // monoid is never terminal, it becomes the empty statement (";").  Otherwise,
    // it checks the terminal condition and does a "break" if true.


    //--------------------------------------------------------------------------
    // determine if the monoid is terminal, and find its terminal value
    //--------------------------------------------------------------------------

        bool is_terminal = false ;
        const char *f = NULL ;

        switch (opcode)
        {

            #if 0
            case GB_ANY_opcode :
                f = NULL ;
                is_terminal = true ;
                break ;
            #endif

            case GB_MIN_opcode :

                is_terminal = true ;
                switch (zcode)
                {
                    case GB_BOOL_code   : f = "false" ;         break ;
                    case GB_INT8_code   : f = "INT8_MIN" ;      break ;
                    case GB_INT16_code  : f = "INT16_MIN" ;     break ;
                    case GB_INT32_code  : f = "INT32_MIN" ;     break ;
                    case GB_INT64_code  : f = "INT64_MIN" ;     break ;
                    case GB_UINT8_code  : f = "0" ;             break ;
                    case GB_UINT16_code : f = "0" ;             break ;
                    case GB_UINT32_code : f = "0" ;             break ;
                    case GB_UINT64_code : f = "0" ;             break ;
                    default             : f = "(-INFINITY)" ;   break ;
                }
                break ;

            case GB_MAX_opcode :

                is_terminal = true ;
                switch (zcode)
                {
                    case GB_BOOL_code   : f = "true" ;          break ;
                    case GB_INT8_code   : f = "INT8_MAX" ;      break ;
                    case GB_INT16_code  : f = "INT16_MAX" ;     break ;
                    case GB_INT32_code  : f = "INT32_MAX" ;     break ;
                    case GB_INT64_code  : f = "INT64_MAX" ;     break ;
                    case GB_UINT8_code  : f = "UINT8_MAX" ;     break ;
                    case GB_UINT16_code : f = "UINT16_MAX" ;    break ;
                    case GB_UINT32_code : f = "UINT32_MAX" ;    break ;
                    case GB_UINT64_code : f = "UINT64_MAX" ;    break ;
                    default             : f = "INFINITY" ;      break ;
                }
                break ;

            case GB_PLUS_opcode :

                if (zcode == GB_BOOL_code)
                {
                    f = "true" ;      // boolean OR
                    is_terminal = true ;
                }
                else
                {
                    f = NULL ;
                    is_terminal = false ;
                }
                break ;

            case GB_TIMES_opcode :

                switch (zcode)
                {
                    case GB_BOOL_code   :   // boolean AND
                    case GB_INT8_code   :
                    case GB_INT16_code  :
                    case GB_INT32_code  :
                    case GB_INT64_code  :
                    case GB_UINT8_code  :
                    case GB_UINT16_code :
                    case GB_UINT32_code :
                    case GB_UINT64_code :
                        f = "0" ;
                        is_terminal = true ;
                        break ;
                    default             :
                        f = NULL ;
                        is_terminal = false ;
                        break ;
                }
                break ;

            case GB_LOR_opcode      : f = "true"  ; is_terminal = true  ; break ;
            case GB_LAND_opcode     : f = "false" ; is_terminal = true  ; break ; 

            case GB_LXOR_opcode     :
            // case GB_LXNOR_opcode :
            case GB_EQ_opcode       :
            default                 :
                // the monoid is not terminal
                f = NULL ;
                is_terminal = false ;
                break ;
        }

        //--------------------------------------------------------------------------
        // construct the macro to test the terminal condition
        //--------------------------------------------------------------------------

        if (is_terminal)
        {
            // the monoid is terminal
            if (f == NULL)
            {
                // ANY monoid
                snprintf (terminal_condition, GB_CUDA_STRLEN,
                    "#define %s(cij) true", macro_condition_name) ;
                snprintf (terminal_statement, GB_CUDA_STRLEN,
                    "#define %s break", macro_statement_name) ;
            }
            else
            {
                // typical terminal monoids: check if C(i,j) has reached its
                // terminal value
                snprintf (terminal_condition, GB_CUDA_STRLEN,
                    "#define %s(cij) ((cij) == %s)", macro_condition_name, f) ;
                snprintf (terminal_statement, GB_CUDA_STRLEN,
                    "#define %s if (%s (cij)) break",
                    macro_statement_name, macro_condition_name) ;
            }
        }
        else
        {
            // the monoid is not terminal: the condition is always false
            snprintf (terminal_condition, GB_CUDA_STRLEN, "#define %s(cij) false",
                macro_condition_name) ;
            snprintf (terminal_statement, GB_CUDA_STRLEN, "#define %s",
                macro_statement_name) ;
        }

        (*is_monoid_terminal) = is_terminal ;
    }


    //--------------------------------------------------------------------------
    //  Handle mask type and structural vs not 
    //--------------------------------------------------------------------------
    const char *stringify_mask
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

// Construct a macro to load and typecast.  For example:
//  
//  #define GB_GETA(blob) blob
//
// then use as:
//      GB_GETA (double aij = Ax [p]) ;
//      GB_GETA (double *Ax = A->x) ;
//      GB_GETA (T_A *restrict Ax = A->x) ;
//
// which become
//      double aij = Ax [p] ;
//      double *Ax = A->x ;
//      T_A *Ax = A->x ;
//
// or, if is_pattern is true, the macro becomes the empty string.

    void stringify_load {}
    (
        // output:
        char *result,
        // input:
        const char *macro_name,       // name of macro to construct
        bool is_pattern         // if true, load/cast does nothing
    )
    {

        if (is_pattern)
        {
            snprintf (result, GB_CUDA_STRLEN, "#define %s(blob)", macro_name) ;
        }
        else
        {
            snprintf (result, GB_CUDA_STRLEN, "#define %s(blob) blob", macro_name) ;
        }
    }

    void stringify_semiring {}

    // Construct a string defining a semiring.
    // User-defined types are not handled.
    // build a semiring (name and code)
    (
        // input:
        GrB_Semiring semiring,  // the semiring to stringify
        bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
        GrB_Type ctype,         // the type of C
        GrB_Type atype,         // the type of A
        GrB_Type btype,         // the type of B
        GrB_Type mtype,         // the type of M, or NULL if no mask
        bool Mask_struct,       // mask is structural
        bool mask_in_semiring_name, // if true, then the semiring_name includes
                                    // the mask_name.  If false, then semiring_name
                                    // is independent of the mask_name
        // output: (all of size at least GB_CUDA_LEN+1)
        char *semiring_name,    // name of the semiring
        char *semiring_code,    // List of types and macro defs
        char *mask_name         // definition of mask data load
    )
    {

        // check inputs
        ASSERT (semiring->object_kind == GB_BUILTIN) ;

        // get the semiring
        GrB_Monoid add = semiring->add ;
        GrB_BinaryOp mult = semiring->multiply ;
        GrB_BinaryOp addop = add->op ;
        GrB_Type xtype = mult->xtype ;
        GrB_Type ytype = mult->ytype ;
        GrB_Type ztype = mult->ztype ;
        GB_Opcode mult_opcode = mult->opcode ;
        GB_Opcode add_opcode  = addop->opcode ;
        GB_Type_code xcode = xtype->code ;
        GB_Type_code ycode = ytype->code ;
        GB_Type_code zcode = ztype->code ;

        // these must always be true for any semiring:
        ASSERT (mult->ztype == addop->ztype) ;
        ASSERT (addop->xtype == addop->ztype && addop->ytype == addop->ztype) ;

        // for now, this is true for all built-in binops:
        ASSERT (xcode == ycode) ;

        //--------------------------------------------------------------------------
        // rename redundant boolean operators
        //--------------------------------------------------------------------------

        // consider z = op(x,y) where both x and y are boolean:
        // DIV becomes FIRST
        // RDIV becomes SECOND
        // MIN and TIMES become LAND
        // MAX and PLUS become LOR
        // NE, ISNE, RMINUS, and MINUS become LXOR
        // ISEQ becomes EQ
        // ISGT becomes GT
        // ISLT becomes LT
        // ISGE becomes GE
        // ISLE becomes LE

        if (zcode == GB_BOOL_code)
        {
            // rename the monoid
            add_opcode = GB_boolean_rename (add_opcode) ;
        }

        if (xcode == GB_BOOL_code)  // && (ycode == GB_BOOL_code)
        { 
            // rename the multiplicative operator
            mult_opcode = GB_boolean_rename (mult_opcode) ;
        }

        //--------------------------------------------------------------------------
        // handle the flip
        //--------------------------------------------------------------------------

        if (flipxy)
        { 
            // z = fmult (b,a) will be computed: handle this by renaming the
            // multiplicative operator

            // handle the flip
            mult_opcode = GB_binop_flip (mult_opcode) ;

            // the flip is now handled completely.  This assumes xtype and ytype
            // are the same for all built-in operators.  If this changes, the
            // types will have to be flipped too.
            flipxy = false ;
        }

        //--------------------------------------------------------------------------
        // determine if A and/or B are value-agnostic
        //--------------------------------------------------------------------------

        bool op_is_first  = (mult_opcode == GB_FIRST_opcode ) ;
        bool op_is_second = (mult_opcode == GB_SECOND_opcode) ;
        bool op_is_pair   = false ; // (mult_opcode == GB_PAIR_opcode) ;
        bool A_is_pattern = op_is_second || op_is_pair ;
        bool B_is_pattern = op_is_first  || op_is_pair ;

        //--------------------------------------------------------------------------
        // construct macros to load scalars from A and B (and typecast) them
        //--------------------------------------------------------------------------

        char acast [GB_CUDA_STRLEN+1] ;
        char bcast [GB_CUDA_STRLEN+1] ;
        GB_cuda_stringify_load (acast, "GB_GETA", A_is_pattern) ;
        GB_cuda_stringify_load (bcast, "GB_GETB", B_is_pattern) ;

        //--------------------------------------------------------------------------
        // construct macros for the multiply
        //--------------------------------------------------------------------------

        char mult_function [GB_CUDA_STRLEN+1] ;
        GB_cuda_stringify_binop (mult_function, "GB_MULT", mult_opcode, zcode) ;

        //--------------------------------------------------------------------------
        // construct the monoid macros
        //--------------------------------------------------------------------------

        char add_function [GB_CUDA_STRLEN+1] ;
        GB_cuda_stringify_binop (add_function, "GB_ADD", add_opcode, zcode) ;

        char identity_definition [GB_CUDA_STRLEN+1] ;
        GB_cuda_stringify_identity ( identity_definition, add_opcode, zcode) ;

        bool is_terminal ;
        char terminal_condition [GB_CUDA_STRLEN+1] ;
        char terminal_statement [GB_CUDA_STRLEN+1] ;

        GB_cuda_stringify_terminal (
            &is_terminal, terminal_condition, terminal_statement,
            "GB_TERMINAL_CONDITION", "GB_IF_TERMINAL_BREAK", add_opcode, zcode) ;

        //--------------------------------------------------------------------------
        // macro to typecast the result back into C
        //--------------------------------------------------------------------------

        // for the ANY_PAIR semiring, "c_is_one" will be true, and Cx [0..cnz] will
        // be filled with all 1's later.
        bool c_is_one = false ;
        // TODO:
        // (add_opcode == GB_ANY_opcode && mult_opcode == GB_PAIR_opcode) ;
        char ccast [GB_CUDA_STRLEN+1] ;
        GB_cuda_stringify_load (ccast, "GB_PUTC", c_is_one) ;

        //--------------------------------------------------------------------------
        // construct the macros to access the mask (if any), and its name
        //--------------------------------------------------------------------------

        const char *mask_string = "" ;
        const char *mask_type_name = "" ;
        const char *struct_str = "struct";
        if (mtype != NULL)
        {
            mask_string = GB_cuda_stringify_mask (mtype->code, Mask_struct) ;
            mask_type_name = mtype->name ;
        }
        else
        {
            mask_type_name = struct_str;
        }

        snprintf (mask_name, GB_CUDA_STRLEN, "mask_%s", mask_type_name) ;

        //--------------------------------------------------------------------------
        // build the final semiring code
        //--------------------------------------------------------------------------

        snprintf (semiring_code, GB_CUDA_STRLEN,
            "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
            acast, bcast, mult_function, add_function, identity_definition,
            terminal_condition, terminal_statement, ccast, mask_string) ;

        //--------------------------------------------------------------------------
        // build the final semiring name
        //--------------------------------------------------------------------------

        // the semiring_name depends on:
        // add_opcode
        // mult_opcode
        // ztype->name
        // xtype->name (currently, always == ytype->name, but will change (TODO))
        // ytype->name
        // ctype->name
        // mask_type_name    (but only if mask_in_semiring_name is true)
        // atype->name
        // btype->name

        const char *add_name;
        const char *mult_name;

        add_name  = GB_cuda_stringify_opcode (add_opcode) ;
        mult_name = GB_cuda_stringify_opcode (mult_opcode) ;

    //  these are not needed: they are template parameters to the CUDA kernel:
    //  ztype->name, xtype->name, ytype->name,
    //  ctype->name, atype->name, btype->name

    //  ztype->name is required, since the kernel needs it for the identity
    //  value.  xtype->name is not strictly required.  However, the GraphBLAS
    //  naming scheme is add_mult_xtype, so it is included here.  The ytype
    //  and ztype need not be xtype.

        if (mask_in_semiring_name)
        {

            // the format of the semiring name is:
            //
            //  semiring_add_mult_xtype_M_mtype_Z_ztype

            snprintf (semiring_name, GB_CUDA_STRLEN,
                "semiring_%s_%s_%s_M_%s_Z_%s",
                // The first part is akin to GxB_PLUS_TIMES_FP64 (for example),
                // but here this example is semiring_plus_times_double instead:
                add_name, mult_name, xtype->name,
                // these are not in the GrB* or GxB* name, but are needed by CUDA:
                // mask_type_name is (say) 'int64' or 'bool'.
                // ztype is the name of the monoid type.
                mask_type_name, ztype->name) ;

        }
        else
        {

            // the format of the semiring name is:
            //
            //  semiring_add_mult_xtype_Z_ztype

            snprintf (semiring_name, GB_CUDA_STRLEN,
                "semiring_%s_%s_%s_Z_%s",
                // The first part is akin to GxB_PLUS_TIMES_FP64 (for example),
                // but here this example is semiring_plus_times_double instead:
                add_name, mult_name, xtype->name,
                // this is not in the GrB* or GxB* name, but is needed by CUDA:
                // ztype is the name of the monoid type.
                ztype->name) ;

        }

        printf ("semiring_name:\n%s\n", semiring_name) ;
        //printf ("semiring_code:\n%s\n", semiring_code) ;
        //printf ("mask_name:    \n%s\n", mask_name) ;
    }


};


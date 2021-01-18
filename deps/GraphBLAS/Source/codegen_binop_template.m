function codegen_binop_template (binop, bfunc, ifunc, ffunc, dfunc, fcfunc, dcfunc)
%CODEGEN_BINOP_TEMPLATE create binop functions
%
% Generate functions for a binary operator, for all types.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\n%-9s', binop) ;

% boolean operators
if (~isempty (bfunc))
    codegen_binop_method (binop, bfunc, 'bool') ;
end

% integer operators
if (~isempty (ifunc))
    codegen_binop_method (binop, ifunc, 'int8_t'  ) ;
    codegen_binop_method (binop, ifunc, 'int16_t' ) ;
    codegen_binop_method (binop, ifunc, 'int32_t' ) ;
    codegen_binop_method (binop, ifunc, 'int64_t' ) ;
    codegen_binop_method (binop, ifunc, 'uint8_t' ) ;
    codegen_binop_method (binop, ifunc, 'uint16_t') ;
    codegen_binop_method (binop, ifunc, 'uint32_t') ;
    codegen_binop_method (binop, ifunc, 'uint64_t') ;
end

% floating-point operators
if (~isempty (ffunc))
    codegen_binop_method (binop, ffunc, 'float'   ) ;
end
if (~isempty (dfunc))
    codegen_binop_method (binop, dfunc, 'double'  ) ;
end
if (~isempty (fcfunc))
    codegen_binop_method (binop, fcfunc, 'GxB_FC32_t') ;
end
if (~isempty (dcfunc))
    codegen_binop_method (binop, dcfunc, 'GxB_FC64_t') ;
end


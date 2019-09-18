function codegen_binop_template (binop, iscompare, bfunc, ifunc, ffunc, dfunc)
%CODEGEN_BINOP_TEMPLATE create binop functions
%
% Generate functions for a binary operator, for all types.

fprintf ('\n%-7s', binop) ;

if (nargin < 5)
    ffunc = [ ] ;
end

if (nargin < 6)
    dfunc = [ ] ;
end

if (isempty (ffunc))
    ffunc = ifunc ;
end

if (isempty (dfunc))
    dfunc = ffunc ;
end

% integer and floating-point operators
codegen_binop_method (binop, ifunc, iscompare, 'int8_t'  ) ;
codegen_binop_method (binop, ifunc, iscompare, 'int16_t' ) ;
codegen_binop_method (binop, ifunc, iscompare, 'int32_t' ) ;
codegen_binop_method (binop, ifunc, iscompare, 'int64_t' ) ;
codegen_binop_method (binop, ifunc, iscompare, 'uint8_t' ) ;
codegen_binop_method (binop, ifunc, iscompare, 'uint16_t') ;
codegen_binop_method (binop, ifunc, iscompare, 'uint32_t') ;
codegen_binop_method (binop, ifunc, iscompare, 'uint64_t') ;
codegen_binop_method (binop, ffunc, iscompare, 'float'   ) ;
codegen_binop_method (binop, dfunc, iscompare, 'double'  ) ;

% boolean operators
if (~isempty (bfunc))
    codegen_binop_method (binop, bfunc, iscompare, 'bool') ;
end


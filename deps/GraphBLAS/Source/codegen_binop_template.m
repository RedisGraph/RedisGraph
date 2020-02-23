function codegen_binop_template (binop, iscompare, bfunc, ifunc, ffunc, dfunc, is_binop_subset)
%CODEGEN_BINOP_TEMPLATE create binop functions
%
% Generate functions for a binary operator, for all types.

fprintf ('\n%-7s', binop) ;

assert (~isequal (binop, 'any'))

if (nargin < 5)
    ffunc = [ ] ;
end

if (nargin < 6)
    dfunc = [ ] ;
end

if (nargin < 7)
    is_binop_subset = false ;
end

if (isempty (ffunc))
    ffunc = ifunc ;
end

if (isempty (dfunc))
    dfunc = ffunc ;
end

% integer and floating-point operators
codegen_binop_method (binop, ifunc, iscompare, 'int8_t'  , is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'int16_t' , is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'int32_t' , is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'int64_t' , is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'uint8_t' , is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'uint16_t', is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'uint32_t', is_binop_subset) ;
codegen_binop_method (binop, ifunc, iscompare, 'uint64_t', is_binop_subset) ;
codegen_binop_method (binop, ffunc, iscompare, 'float'   , is_binop_subset) ;
codegen_binop_method (binop, dfunc, iscompare, 'double'  , is_binop_subset) ;

% boolean operators
if (~isempty (bfunc))
    codegen_binop_method (binop, bfunc, iscompare, 'bool', is_binop_subset) ;
end


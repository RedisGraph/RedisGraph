function codegen_axb_compare_template (multop, bmult, mult)
%CODEGEN_AXB_COMPARE_TEMPLATE create a function for a semiring with a TxT -> bool multiplier

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

% For gcc and icc: lor, land, and lxor monoids can be done with OpenMP atomics.
% For Microsoft Visual Studio: no boolean monoids work with OpenMP atomics.

fprintf ('\n%-7s', multop) ;

% lor monoid
add = 'w |= t' ;        % TODO: should this be w ||= t ?
addfunc = 'w | t' ;     % TODO: should this be w || t ?
codegen_axb_method ('lor', multop, add, addfunc, bmult, 'bool', 'bool'    , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'int8_t'  , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'uint8_t' , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'int16_t' , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'uint16_t', 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'int32_t' , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'uint32_t', 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'int64_t' , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'uint64_t', 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'float'   , 'false', 'true', 1, 0) ;
codegen_axb_method ('lor', multop, add, addfunc,  mult, 'bool', 'double'  , 'false', 'true', 1, 0) ;

% any monoid
add = 'w = t' ;
addfunc = 't' ;
codegen_axb_method ('any', multop, add, addfunc, bmult, 'bool', 'bool'    , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'int8_t'  , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'uint8_t' , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'int16_t' , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'uint16_t', 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'int32_t' , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'uint32_t', 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'int64_t' , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'uint64_t', 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'float'   , 'false', '(any value)', 0, 0) ;
codegen_axb_method ('any', multop, add, addfunc,  mult, 'bool', 'double'  , 'false', '(any value)', 0, 0) ;

% land monoid
add = 'w &= t' ;        % TODO: should this be w &&= t ?
addfunc = 'w & t' ;     % TODO: should this be w && t ?
codegen_axb_method ('land', multop, add, addfunc, bmult, 'bool', 'bool'    , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'int8_t'  , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'uint8_t' , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'int16_t' , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'uint16_t', 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'int32_t' , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'uint32_t', 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'int64_t' , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'uint64_t', 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'float'   , 'true', 'false', 1, 0) ;
codegen_axb_method ('land', multop, add, addfunc,  mult, 'bool', 'double'  , 'true', 'false', 1, 0) ;

% lxor monoid
add = 'w ^= t' ;
addfunc = 'w ^ t' ;
codegen_axb_method ('lxor', multop, add, addfunc, bmult, 'bool', 'bool'    , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'int8_t'  , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'uint8_t' , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'int16_t' , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'uint16_t', 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'int32_t' , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'uint32_t', 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'int64_t' , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'uint64_t', 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'float'   , 'false', [ ], 1, 0) ;
codegen_axb_method ('lxor', multop, add, addfunc,  mult, 'bool', 'double'  , 'false', [ ], 1, 0) ;

% eq (lxnor) monoid.  Cannot be done with OpenMP atomic update
add = 'w = (w == t)' ;
addfunc = 'w == t' ;
codegen_axb_method ('eq', multop, add, addfunc, bmult, 'bool', 'bool'    , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'int8_t'  , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'uint8_t' , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'int16_t' , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'uint16_t', 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'int32_t' , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'uint32_t', 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'int64_t' , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'uint64_t', 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'float'   , 'true', [ ], 0, 0) ;
codegen_axb_method ('eq', multop, add, addfunc,  mult, 'bool', 'double'  , 'true', [ ], 0, 0) ;


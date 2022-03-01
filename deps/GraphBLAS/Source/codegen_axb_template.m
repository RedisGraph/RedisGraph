function codegen_axb_template (multop, bmult, imult, fmult, dmult, fcmult, dcmult, no_min_max_any_times_monoids)
%CODEGEN_AXB_TEMPLATE create a function for a semiring with a TxT->T multiplier

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\n%-7s', multop) ;
if (nargin < 8)
    no_min_max_any_times_monoids = false ;
end

is_pair = isequal (multop, 'pair') ;
% the any_pair_iso semiring
if (is_pair)
    codegen_axb_method ('any', 'pair') ;
end

plusinf32 = 'INFINITY' ;
neginf32  = '(-INFINITY)' ;
plusinf64 = '((double) INFINITY)' ;
neginf64  = '((double) -INFINITY)' ;

% MIN monoid: integer types are terminal, float and double are not.  None can be done with OpenMP atomic update
if (~no_min_max_any_times_monoids)
    add = 'w = GB_IMIN (w, t)' ;
    addfunc = 'GB_IMIN (w, t)' ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'int8_t'  , 'int8_t'  , 'INT8_MAX'  , 'INT8_MIN'  , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'int16_t' , 'int16_t' , 'INT16_MAX' , 'INT16_MIN' , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'int32_t' , 'int32_t' , 'INT32_MAX' , 'INT32_MIN' , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'int64_t' , 'int64_t' , 'INT64_MAX' , 'INT64_MIN' , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'uint8_t' , 'uint8_t' , 'UINT8_MAX' , '0'         , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'uint16_t', 'uint16_t', 'UINT16_MAX', '0'         , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'uint32_t', 'uint32_t', 'UINT32_MAX', '0'         , 0, 0) ;
    codegen_axb_method ('min', multop, add, addfunc, imult, 'uint64_t', 'uint64_t', 'UINT64_MAX', '0'         , 0, 0) ;
    add = 'w = fminf (w, t)' ;
    addfunc = 'fminf (w, t)' ;
    codegen_axb_method ('min', multop, add, addfunc, fmult, 'float'   , 'float'   , plusinf32   , [ ]         , 0, 0) ;
    add = 'w = fmin (w, t)' ;
    addfunc = 'fmin (w, t)' ;
    codegen_axb_method ('min', multop, add, addfunc, dmult, 'double'  , 'double'  , plusinf64   , [ ]         , 0, 0) ;
end

% MAX monoid: integer types are terminal, float and double are not.  None can be done with OpenMP atomic update
if (~no_min_max_any_times_monoids)
    add = 'w = GB_IMAX (w, t)' ;
    addfunc = 'GB_IMAX (w, t)' ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'int8_t'  , 'int8_t'  , 'INT8_MIN'  , 'INT8_MAX'  , 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'int16_t' , 'int16_t' , 'INT16_MIN' , 'INT16_MAX' , 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'int32_t' , 'int32_t' , 'INT32_MIN' , 'INT32_MAX' , 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'int64_t' , 'int64_t' , 'INT64_MIN' , 'INT64_MAX' , 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'uint8_t' , 'uint8_t' , '0'         , 'UINT8_MAX' , 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'uint16_t', 'uint16_t', '0'         , 'UINT16_MAX', 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'uint32_t', 'uint32_t', '0'         , 'UINT32_MAX', 0, 0) ;
    codegen_axb_method ('max', multop, add, addfunc, imult, 'uint64_t', 'uint64_t', '0'         , 'UINT64_MAX', 0, 0) ;
    % floating-point MAX must use unsigned integer puns for compare-and-swap
    add = 'w = fmaxf (w, t)' ;
    addfunc = 'fmaxf (w, t)' ;
    codegen_axb_method ('max', multop, add, addfunc, fmult, 'float'   , 'float'   , neginf32    , [ ]         , 0, 0) ;
    add = 'w = fmax (w, t)' ;
    addfunc = 'fmax (w, t)' ;
    codegen_axb_method ('max', multop, add, addfunc, dmult, 'double'  , 'double'  , neginf64    , [ ]         , 0, 0) ;
end

% ANY monoid: all are terminal.
if (~no_min_max_any_times_monoids && ~is_pair)
    add = 'w = t' ;
    addfunc = 't' ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'int8_t'  , 'int8_t'  , '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'int16_t' , 'int16_t' , '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'int32_t' , 'int32_t' , '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'int64_t' , 'int64_t' , '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'uint8_t' , 'uint8_t' , '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'uint16_t', 'uint16_t', '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'uint32_t', 'uint32_t', '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, imult, 'uint64_t', 'uint64_t', '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, fmult, 'float'   , 'float'   , '0' , '(any value)', 0, 0) ;
    codegen_axb_method ('any', multop, add, addfunc, dmult, 'double'  , 'double'  , '0' , '(any value)', 0, 0) ;
    % complex case:
    id = 'GxB_CMPLXF(0,0)' ;
    codegen_axb_method ('any', multop, add, addfunc, fcmult, 'GxB_FC32_t', 'GxB_FC32_t', id, '(any value)', 0, 0) ;
    id = 'GxB_CMPLX(0,0)' ;
    codegen_axb_method ('any', multop, add, addfunc, dcmult, 'GxB_FC64_t', 'GxB_FC64_t', id, '(any value)', 0, 0) ;
end

% PLUS monoid: none are terminal.  All reals can be done with OpenMP atomic update
add = 'w += t' ;
addfunc = 'w + t' ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'int8_t'  , 'int8_t'  , '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'uint8_t' , 'uint8_t' , '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'int16_t' , 'int16_t' , '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'uint16_t', 'uint16_t', '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'int32_t' , 'int32_t' , '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'uint32_t', 'uint32_t', '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'int64_t' , 'int64_t' , '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, imult, 'uint64_t', 'uint64_t', '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, fmult, 'float'   , 'float'   , '0', [ ], 1, 1) ;
codegen_axb_method ('plus', multop, add, addfunc, dmult, 'double'  , 'double'  , '0', [ ], 1, 1) ;
% complex types done with two OpenMP atomic updates:
add = 'w = GB_FC32_add (w, t)' ;
addfunc = 'GB_FC32_add (w, t)' ;
id = 'GxB_CMPLXF(0,0)' ;
codegen_axb_method ('plus', multop, add, addfunc, fcmult, 'GxB_FC32_t', 'GxB_FC32_t', id, [ ], 1, 1) ;
add = 'w = GB_FC64_add (w, t)' ;
addfunc = 'GB_FC64_add (w, t)' ;
id = 'GxB_CMPLX(0,0)' ;
codegen_axb_method ('plus', multop, add, addfunc, dcmult, 'GxB_FC64_t', 'GxB_FC64_t', id, [ ], 1, 1) ;

% TIMES monoid: integers are terminal, float and double are not.
if (~no_min_max_any_times_monoids)
    % All real types can be done with OpenMP atomic update
    add = 'w *= t' ;
    addfunc = 'w * t' ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'int8_t'  , 'int8_t'  , '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'uint8_t' , 'uint8_t' , '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'int16_t' , 'int16_t' , '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'uint16_t', 'uint16_t', '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'int32_t' , 'int32_t' , '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'uint32_t', 'uint32_t', '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'int64_t' , 'int64_t' , '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, imult, 'uint64_t', 'uint64_t', '1', '0', 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, fmult, 'float'   , 'float'   , '1', [ ], 1, 1) ;
    codegen_axb_method ('times', multop, add, addfunc, dmult, 'double'  , 'double'  , '1', [ ], 1, 1) ;
    % complex types cannot be done with OpenMP atomic update:
    add = 'w = GB_FC32_mul (w, t)' ;
    addfunc = 'GB_FC32_mul (w, t)' ;
    id = 'GxB_CMPLXF(1,0)' ;
    codegen_axb_method ('times', multop, add, addfunc, fcmult, 'GxB_FC32_t', 'GxB_FC32_t', id, [ ], 0, 0) ;
    add = 'w = GB_FC64_mul (w, t)' ;
    addfunc = 'GB_FC64_mul (w, t)' ;
    id = 'GxB_CMPLX(1,0)' ;
    codegen_axb_method ('times', multop, add, addfunc, dcmult, 'GxB_FC64_t', 'GxB_FC64_t', id, [ ], 0, 0) ;
end

% boolean monoids: LOR, LAND are terminal; LXOR, EQ are not.
% For gcc and icc: LOR, LAND, and LXOR can be done as OpenMP atomic updates; EQ cannot.
% For MS Visual Studio: none can be done with OpenMP atomic updates.
codegen_axb_method ('lor',  multop, 'w |= t', 'w | t', bmult, 'bool', 'bool', 'false', 'true' , 1, 0) ;
codegen_axb_method ('land', multop, 'w &= t', 'w & t', bmult, 'bool', 'bool', 'true' , 'false', 1, 0) ;
codegen_axb_method ('lxor', multop, 'w ^= t', 'w ^ t', bmult, 'bool', 'bool', 'false', [ ]    , 1, 0) ;
if (~is_pair)
    codegen_axb_method ('any', multop, 'w = t' , 't' , bmult, 'bool', 'bool', '0'    , '(any value)', 0, 0) ;
end
add = 'w = (w == t)' ;
addfunc = 'w == t' ;
codegen_axb_method ('eq',   multop, add,      addfunc, bmult, 'bool', 'bool', 'true' , [ ]    , 0, 0) ;


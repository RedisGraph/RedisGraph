function codegen_binop
%CODEGEN_BINOP create functions for all binary operators
%
% This function creates all files of the form GB_binop__*.[ch], including 260
% functions (GB_binop__*.c) and one include file, GB_binop__include.h.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('\nbinary operators:\n') ;

f = fopen ('Generated2/GB_binop__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_binop__include.h: definitions for GB_binop__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.\n') ;
fprintf (f, '// SPDX-License-Identifier: Apache-2.0\n\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_binop.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

% The ANY operator is not used as a binary operator in the generated functions.
% It can be used as the binary op in eWiseAdd, eWiseMult, etc, but has been
% renamed to SECOND before calling the generated function.

codegen_binop_template ('first',        ...
    'xarg',                             ... % bool
    'xarg',                             ... % int, uint
    'xarg',                             ... % float
    'xarg',                             ... % double
    'xarg',                             ... % GxB_FC32_t
    'xarg') ;                           ... % GxB_FC64_t

codegen_binop_template ('second',       ...
    'yarg',                             ... % bool
    'yarg',                             ... % int, uint
    'yarg',                             ... % float
    'yarg',                             ... % double
    'yarg',                             ... % GxB_FC32_t
    'yarg') ;                           ... % GxB_FC64_t

codegen_binop_template ('pair',         ...
    '1',                                ... % bool
    '1',                                ... % int, uint
    '1',                                ... % float
    '1',                                ... % double
    'GxB_CMPLXF(1,0)',                  ... % GxB_FC32_t
    'GxB_CMPLX(1,0)') ;                 ... % GxB_FC64_t

codegen_binop_template ('min',          ...
    [ ],                                ... % bool
    'GB_IMIN (xarg, yarg)',             ... % int, uint
    'fminf (xarg, yarg)',               ... % float
    'fmin (xarg, yarg)',                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('max',          ...
    [ ],                                ... % bool
    'GB_IMAX (xarg, yarg)',             ... % int, uint
    'fmaxf (xarg, yarg)',               ... % float
    'fmax (xarg, yarg)',                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('plus',         ...
    [ ],                                ... % bool
    '(xarg + yarg)',                    ... % int, uint
    '(xarg + yarg)',                    ... % float
    '(xarg + yarg)',                    ... % double
    'GB_FC32_add (xarg, yarg)',         ... % GxB_FC32_t
    'GB_FC64_add (xarg, yarg)') ;       ... % GxB_FC64_t

codegen_binop_template ('minus',        ...
    [ ],                                ... % bool
    '(xarg - yarg)',                    ... % int, uint
    '(xarg - yarg)',                    ... % float
    '(xarg - yarg)',                    ... % double
    'GB_FC32_minus (xarg, yarg)',       ... % GxB_FC32_t
    'GB_FC64_minus (xarg, yarg)') ;     ... % GxB_FC64_t

codegen_binop_template ('rminus',       ...
    [ ],                                ... % bool
    '(yarg - xarg)',                    ... % int, uint
    '(yarg - xarg)',                    ... % float
    '(yarg - xarg)',                    ... % double
    'GB_FC32_minus (yarg, xarg)',       ... % GxB_FC32_t
    'GB_FC64_minus (yarg, xarg)') ;     ... % GxB_FC64_t

codegen_binop_template ('times',        ...
    [ ],                                ... % bool
    '(xarg * yarg)',                    ... % int, uint
    '(xarg * yarg)',                    ... % float
    '(xarg * yarg)',                    ... % double
    'GB_FC32_mul (xarg, yarg)',         ... % GxB_FC32_t
    'GB_FC64_mul (xarg, yarg)') ;       ... % GxB_FC64_t

codegen_binop_template ('div',          ...
    [ ],                                ... % bool
    'GB_idiv (xarg, yarg)',             ... % int, uint
    '(xarg / yarg)',                    ... % float
    '(xarg / yarg)',                    ... % double
    'GB_FC32_div (xarg, yarg)',         ... % GxB_FC32_t
    'GB_FC64_div (xarg, yarg)') ;       ... % GxB_FC64_t

codegen_binop_template ('rdiv',         ...
    [ ],                                ... % bool
    'GB_idiv (yarg, xarg)',             ... % int, uint
    '(yarg / xarg)',                    ... % float
    '(yarg / xarg)',                    ... % double
    'GB_FC32_div (yarg, xarg)',         ... % GxB_FC32_t
    'GB_FC64_div (yarg, xarg)') ;       ... % GxB_FC64_t

codegen_binop_template ('iseq',         ...
    [ ],                                ... % bool
    '(xarg == yarg)',                   ... % int, uint
    '(xarg == yarg)',                   ... % float
    '(xarg == yarg)',                   ... % double
    'GB_FC32_iseq (xarg, yarg)',        ... % GxB_FC32_t
    'GB_FC64_iseq (xarg, yarg)') ;      ... % GxB_FC64_t

codegen_binop_template ('isne',         ...
    [ ],                                ... % bool
    '(xarg != yarg)',                   ... % int, uint
    '(xarg != yarg)',                   ... % float
    '(xarg != yarg)',                   ... % double
    'GB_FC32_isne (xarg, yarg)',        ... % GxB_FC32_t
    'GB_FC64_isne (xarg, yarg)') ;      ... % GxB_FC64_t

codegen_binop_template ('isgt',         ...
    [ ],                                ... % bool
    '(xarg > yarg)',                    ... % int, uint
    '(xarg > yarg)',                    ... % float
    '(xarg > yarg)',                    ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('islt',         ...
    [ ],                                ... % bool
    '(xarg < yarg)',                    ... % int, uint
    '(xarg < yarg)',                    ... % float
    '(xarg < yarg)',                    ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('isge',         ...
    [ ],                                ... % bool
    '(xarg >= yarg)',                   ... % int, uint
    '(xarg >= yarg)',                   ... % float
    '(xarg >= yarg)',                   ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('isle',         ...
    [ ],                                ... % bool
    '(xarg <= yarg)',                   ... % int, uint
    '(xarg <= yarg)',                   ... % float
    '(xarg <= yarg)',                   ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('eq',           ...
    '(xarg == yarg)',                   ... % bool
    '(xarg == yarg)',                   ... % int, uint
    '(xarg == yarg)',                   ... % float
    '(xarg == yarg)',                   ... % double
    'GB_FC32_eq (xarg, yarg)',          ... % GxB_FC32_t
    'GB_FC64_eq (xarg, yarg)') ;        ... % GxB_FC64_t

codegen_binop_template ('ne',           ...
    [ ],                                ... % bool
    '(xarg != yarg)',                   ... % int, uint
    '(xarg != yarg)',                   ... % float
    '(xarg != yarg)',                   ... % double
    'GB_FC32_ne (xarg, yarg)',          ... % GxB_FC32_t
    'GB_FC64_ne (xarg, yarg)') ;        ... % GxB_FC64_t

codegen_binop_template ('gt',           ...
    '(xarg > yarg)',                    ... % bool
    '(xarg > yarg)',                    ... % int, uint
    '(xarg > yarg)',                    ... % float
    '(xarg > yarg)',                    ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('lt',           ...
    '(xarg < yarg)',                    ... % bool
    '(xarg < yarg)',                    ... % int, uint
    '(xarg < yarg)',                    ... % float
    '(xarg < yarg)',                    ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('ge',           ...
    '(xarg >= yarg)',                   ... % bool
    '(xarg >= yarg)',                   ... % int, uint
    '(xarg >= yarg)',                   ... % float
    '(xarg >= yarg)',                   ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('le',           ...
    '(xarg <= yarg)',                   ... % bool
    '(xarg <= yarg)',                   ... % int, uint
    '(xarg <= yarg)',                   ... % float
    '(xarg <= yarg)',                   ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('lor',          ...
    '(xarg || yarg)',                   ... % bool
    '((xarg != 0) || (yarg != 0))',     ... % int, uint
    '((xarg != 0) || (yarg != 0))',     ... % float
    '((xarg != 0) || (yarg != 0))',     ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('land',         ...
    '(xarg && yarg)',                   ... % bool
    '((xarg != 0) && (yarg != 0))',     ... % int, uint
    '((xarg != 0) && (yarg != 0))',     ... % float
    '((xarg != 0) && (yarg != 0))',     ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('lxor',         ...
    '(xarg != yarg)',                   ... % bool
    '((xarg != 0) != (yarg != 0))',     ... % int, uint
    '((xarg != 0) != (yarg != 0))',     ... % float
    '((xarg != 0) != (yarg != 0))',     ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('atan2',        ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'atan2f (xarg, yarg)',              ... % float
    'atan2 (xarg, yarg)',               ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('hypot',        ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'hypotf (xarg, yarg)',              ... % float
    'hypot (xarg, yarg)',               ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('fmod',         ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'fmodf (xarg, yarg)',               ... % float
    'fmod (xarg, yarg)',                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('remainder',    ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'remainderf (xarg, yarg)',          ... % float
    'remainder (xarg, yarg)',           ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('copysign',     ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'copysignf (xarg, yarg)',           ... % float
    'copysign (xarg, yarg)',            ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('ldexp',        ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'ldexpf (xarg, yarg)',              ... % float
    'ldexp (xarg, yarg)',               ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('cmplx',        ...
    [ ],                                ... % bool
    [ ],                                ... % int, uint
    'GxB_CMPLXF (xarg, yarg)',          ... % float  (z is GxB_FC32_t)
    'GxB_CMPLX (xarg, yarg)',           ... % double (z is GxB_FC64_t)
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('bor',          ...
    [ ],                                ... % bool
    '(xarg) | (yarg)',                  ... % int, uint
    [ ],                                ... % float
    [ ],                                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('band',         ...
    [ ],                                ... % bool
    '(xarg) & (yarg)',                  ... % int, uint
    [ ],                                ... % float
    [ ],                                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('bxor',         ...
    [ ],                                ... % bool
    '(xarg) ^ (yarg)',                  ... % int, uint
    [ ],                                ... % float
    [ ],                                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

codegen_binop_template ('bxnor',        ...
    [ ],                                ... % bool
    '~((xarg) ^ (yarg))',               ... % int, uint
    [ ],                                ... % float
    [ ],                                ... % double
    [ ],                                ... % GxB_FC32_t
    [ ]) ;                              ... % GxB_FC64_t

% bget
fprintf ('\nbget     ') ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, int8_t, 8)'   , 'int8_t'  ) ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, int16_t, 16)' , 'int16_t' ) ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, int32_t, 32)' , 'int32_t' ) ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, int64_t, 64)' , 'int64_t' ) ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, uint8_t, 8)'  , 'uint8_t' ) ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, uint16_t, 16)', 'uint16_t') ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, uint32_t, 32)', 'uint32_t') ;
codegen_binop_method ('bget', 'GB_BITGET (xarg, yarg, uint64_t, 64)', 'uint64_t') ;

% bset
fprintf ('\nbset     ') ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, int8_t, 8)'   , 'int8_t'  ) ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, int16_t, 16)' , 'int16_t' ) ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, int32_t, 32)' , 'int32_t' ) ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, int64_t, 64)' , 'int64_t' ) ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, uint8_t, 8)'  , 'uint8_t' ) ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, uint16_t, 16)', 'uint16_t') ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, uint32_t, 32)', 'uint32_t') ;
codegen_binop_method ('bset', 'GB_BITSET (xarg, yarg, uint64_t, 64)', 'uint64_t') ;

% bclr
fprintf ('\nbclr     ') ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, int8_t, 8)'   , 'int8_t'  ) ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, int16_t, 16)' , 'int16_t' ) ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, int32_t, 32)' , 'int32_t' ) ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, int64_t, 64)' , 'int64_t' ) ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, uint8_t, 8)'  , 'uint8_t' ) ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, uint16_t, 16)', 'uint16_t') ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, uint32_t, 32)', 'uint32_t') ;
codegen_binop_method ('bclr', 'GB_BITCLR (xarg, yarg, uint64_t, 64)', 'uint64_t') ;

% bshift
fprintf ('\nbshift   ') ;
codegen_binop_method ('bshift', 'GB_bitshift_int8 (xarg, yarg)'  , 'int8_t'  ) ;
codegen_binop_method ('bshift', 'GB_bitshift_int16 (xarg, yarg)' , 'int16_t' ) ;
codegen_binop_method ('bshift', 'GB_bitshift_int32 (xarg, yarg)' , 'int32_t' ) ;
codegen_binop_method ('bshift', 'GB_bitshift_int64 (xarg, yarg)' , 'int64_t' ) ;
codegen_binop_method ('bshift', 'GB_bitshift_uint8 (xarg, yarg)' , 'uint8_t' ) ;
codegen_binop_method ('bshift', 'GB_bitshift_uint16 (xarg, yarg)', 'uint16_t') ;
codegen_binop_method ('bshift', 'GB_bitshift_uint32 (xarg, yarg)', 'uint32_t') ;
codegen_binop_method ('bshift', 'GB_bitshift_uint64 (xarg, yarg)', 'uint64_t') ;

% pow
fprintf ('\npow      ') ;
codegen_binop_method ('pow', 'GB_pow_int8 (xarg, yarg)'  , 'int8_t'    ) ;
codegen_binop_method ('pow', 'GB_pow_int16 (xarg, yarg)' , 'int16_t'   ) ;
codegen_binop_method ('pow', 'GB_pow_int32 (xarg, yarg)' , 'int32_t'   ) ;
codegen_binop_method ('pow', 'GB_pow_int64 (xarg, yarg)' , 'int64_t'   ) ;
codegen_binop_method ('pow', 'GB_pow_uint8 (xarg, yarg)' , 'uint8_t'   ) ;
codegen_binop_method ('pow', 'GB_pow_uint16 (xarg, yarg)', 'uint16_t'  ) ;
codegen_binop_method ('pow', 'GB_pow_uint32 (xarg, yarg)', 'uint32_t'  ) ;
codegen_binop_method ('pow', 'GB_pow_uint64 (xarg, yarg)', 'uint64_t'  ) ;
codegen_binop_method ('pow', 'GB_powf (xarg, yarg)'      , 'float'     ) ;
codegen_binop_method ('pow', 'GB_pow (xarg, yarg)'       , 'double'    ) ;
codegen_binop_method ('pow', 'GB_cpowf (xarg, yarg)'     , 'GxB_FC32_t') ;
codegen_binop_method ('pow', 'GB_cpow (xarg, yarg)'      , 'GxB_FC64_t') ;

fprintf ('\n') ;


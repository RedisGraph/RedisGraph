function codegen_red
%CODEGEN_RED create functions for all reduction operators
%
% This function creates all files of the form GB_red__*.c,
% and the include file GB_red__include.h.

fprintf ('\nreduction operators:\n') ;

f = fopen ('Generated/GB_red__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_red__include.h: definitions for GB_red__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.\n') ;
fprintf (f, '// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.\n') ;
fprintf (f, '\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_red.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

%-------------------------------------------------------------------------------
% the monoid: MIN, MAX, PLUS, TIMES, ANY, OR, AND, XOR, EQ
%-------------------------------------------------------------------------------

% Note that the min and max monoids are carefully written to obtain the correct
% NaN behavior for float and double.  Comparisons with NaN are always false.
% zarg is the accumulator.  If zarg is not NaN and the comparison is false,
% zarg is not modified and the value of yarg is properly ignored.  Thus if zarg
% is not NaN but yarg is NaN, then yarg is ignored.  If zarg is NaN, the
% condition becomes true and zarg is replaced with yarg.

% The panel size is selected so that the Panel array in GB_reduce_panel.c is
% always 128 bytes in size (16 doubles = 16*8 = 128 bytes, 32 floats, etc).
% This is 1024 bits, which can be computed with two 512-bit Intel vector
% instructions.  Reducing the panel to 64 bytes (512 bits), or increasing
% the panel size, is slightly slower.

% Panel sizes are optimal for gcc 8.3, on a MacBook.  They are probably fine
% for other architectures and compilers, too, but they haven't been tuned
% except for gcc 8.3 on a Mac.

% MIN: 10 monoids:  name      op   type        identity      terminal   panel
fprintf ('\nmin    ') ;
op = 'if (yarg < zarg) zarg = yarg' ;
codegen_red_method ('min',    op, 'int8_t'  , 'INT8_MAX'  , 'INT8_MIN'  , 16) ;
codegen_red_method ('min',    op, 'int16_t' , 'INT16_MAX' , 'INT16_MIN' , 16) ;
codegen_red_method ('min',    op, 'int32_t' , 'INT32_MAX' , 'INT32_MIN' , 16) ;
codegen_red_method ('min',    op, 'int64_t' , 'INT64_MAX' , 'INT64_MIN' , 16) ;
codegen_red_method ('min',    op, 'uint8_t' , 'UINT8_MAX' , '0'         , 16) ;
codegen_red_method ('min',    op, 'uint16_t', 'UINT16_MAX', '0'         , 16) ;
codegen_red_method ('min',    op, 'uint32_t', 'UINT32_MAX', '0'         , 16) ;
codegen_red_method ('min',    op, 'uint64_t', 'UINT64_MAX', '0'         , 16) ;
op = 'if ((yarg < zarg) || (zarg != zarg)) zarg = yarg' ;
codegen_red_method ('min',    op, 'float'   , 'INFINITY' , '(-INFINITY)', 16) ;
codegen_red_method ('min',    op, 'double'  , ...
    '((double) INFINITY)'  , '((double) -INFINITY)' , 16) ;

% MAX: 10 monoids
fprintf ('\nmax    ') ;
op = 'if (yarg > zarg) zarg = yarg' ;
codegen_red_method ('max',    op, 'int8_t'  , 'INT8_MIN'  , 'INT8_MAX'  , 16) ;
codegen_red_method ('max',    op, 'int16_t' , 'INT16_MIN' , 'INT16_MAX' , 16) ;
codegen_red_method ('max',    op, 'int32_t' , 'INT32_MIN' , 'INT32_MAX' , 16) ;
codegen_red_method ('max',    op, 'int64_t' , 'INT64_MIN' , 'INT64_MAX' , 16) ;
codegen_red_method ('max',    op, 'uint8_t' , '0'         , 'UINT8_MAX' , 16) ;
codegen_red_method ('max',    op, 'uint16_t', '0'         , 'UINT16_MAX', 16) ;
codegen_red_method ('max',    op, 'uint32_t', '0'         , 'UINT32_MAX', 16) ;
codegen_red_method ('max',    op, 'uint64_t', '0'         , 'UINT64_MAX', 16) ;
op = 'if ((yarg > zarg) || (zarg != zarg)) zarg = yarg' ;
codegen_red_method ('max',    op, 'float'   , '(-INFINITY)', 'INFINITY' , 16) ;
codegen_red_method ('max',    op, 'double'  , ...
    '((double) INFINITY)'  , '((double) -INFINITY)' , 16) ;

% ANY: 11 monoids (including boolean)
fprintf ('\nany    ') ;
op = 'zarg = yarg' ;
codegen_red_method ('any',    op, 'int8_t'  , '0') ;
codegen_red_method ('any',    op, 'int16_t' , '0') ;
codegen_red_method ('any',    op, 'int32_t' , '0') ;
codegen_red_method ('any',    op, 'int64_t' , '0') ;
codegen_red_method ('any',    op, 'uint8_t' , '0') ;
codegen_red_method ('any',    op, 'uint16_t', '0') ;
codegen_red_method ('any',    op, 'uint32_t', '0') ;
codegen_red_method ('any',    op, 'uint64_t', '0') ;
codegen_red_method ('any',    op, 'float'   , '0') ;
codegen_red_method ('any',    op, 'double'  , '0') ;
codegen_red_method ('any' ,   op, 'bool'    , '0') ;

% PLUS: 10 monoids
fprintf ('\nplus   ') ;
op = 'zarg += yarg' ;
codegen_red_method ('plus',   op, 'int8_t'  , '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'int16_t' , '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'int32_t' , '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'int64_t' , '0'         , [ ]         , 32) ;
codegen_red_method ('plus',   op, 'uint8_t' , '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'uint16_t', '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'uint32_t', '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'uint64_t', '0'         , [ ]         , 32) ;
codegen_red_method ('plus',   op, 'float'   , '0'         , [ ]         , 64) ;
codegen_red_method ('plus',   op, 'double'  , '0'         , [ ]         , 32) ;

% TIMES: 10 monoids
fprintf ('\ntimes  ') ;
op = 'zarg *= yarg' ;
codegen_red_method ('times',  op, 'int8_t'  , '1'         , '0'         , 64) ;
codegen_red_method ('times',  op, 'int16_t' , '1'         , '0'         , 64) ;
codegen_red_method ('times',  op, 'int32_t' , '1'         , '0'         , 64) ;
codegen_red_method ('times',  op, 'int64_t' , '1'         , '0'         , 16) ;
codegen_red_method ('times',  op, 'uint8_t' , '1'         , '0'         , 64) ;
codegen_red_method ('times',  op, 'uint16_t', '1'         , '0'         , 64) ;
codegen_red_method ('times',  op, 'uint32_t', '1'         , '0'         , 64) ;
codegen_red_method ('times',  op, 'uint64_t', '1'         , '0'         , 16) ;
codegen_red_method ('times',  op, 'float'   , '1'         , [ ]         , 64) ;
codegen_red_method ('times',  op, 'double'  , '1'         , [ ]         , 32) ;

% 4 boolean monoids
fprintf ('\nlor    ') ;
codegen_red_method ('lor' , 'zarg = (zarg || yarg)', 'bool','false', 'true' ,8);
fprintf ('\nland   ') ;
codegen_red_method ('land', 'zarg = (zarg && yarg)', 'bool','true' , 'false',8);
fprintf ('\nlxor   ') ;
codegen_red_method ('lxor', 'zarg = (zarg != yarg)', 'bool','false', [ ]    ,8);
fprintf ('\neq     ') ;
codegen_red_method ('eq'  , 'zarg = (zarg == yarg)', 'bool','true' , [ ]    ,8);
fprintf ('\nany    ') ;
codegen_red_method ('any' , 'zarg = (yarg)'        , 'bool','false') ;

%-------------------------------------------------------------------------------
% FIRST and SECOND (not monoids; used for GB_red_build__[first,second]_[type])
%-------------------------------------------------------------------------------

% FIRST: 11 ops:    name      op           type        identity terminal panel
fprintf ('\nfirst  ') ;
codegen_red_method ('first',  ';'          , 'bool'    , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'int8_t'  , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'int16_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'int32_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'int64_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'uint8_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'uint16_t', [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'uint32_t', [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'uint64_t', [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'float'   , [ ]  , [ ],     1  ) ;
codegen_red_method ('first',  ';'          , 'double'  , [ ]  , [ ],     1  ) ;

% SECOND: 11 ops    name      op           type        identity terminal panel
fprintf ('\nsecond ') ;
codegen_red_method ('second', 'zarg = yarg', 'bool'    , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'int8_t'  , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'int16_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'int32_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'int64_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'uint8_t' , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'uint16_t', [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'uint32_t', [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'uint64_t', [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'float'   , [ ]  , [ ],     1  ) ;
codegen_red_method ('second', 'zarg = yarg', 'double'  , [ ]  , [ ],     1  ) ;

fprintf ('\n') ;


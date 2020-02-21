function codegen_binop
%CODEGEN_BINOP create functions for all binary operators
%
% This function creates all files of the form GB_binop__*.[ch], including 260
% functions (GB_binop__*.c) and one include file, GB_binop__include.h.

fprintf ('\nbinary operators:\n') ;

f = fopen ('Generated/GB_binop__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_binop__include.h: definitions for GB_binop__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.\n') ;
fprintf (f, '// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.\n') ;
fprintf (f, '\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_binop.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

codegen_binop_template ('first',  0, 'xarg', 'xarg') ;
codegen_binop_template ('second', 0, 'yarg', 'yarg') ;
codegen_binop_template ('pair',   0, '1', '1') ;

% The ANY operator is not used as a binary operator in the generated functions.
% It can be used as the binary op in eWiseAdd, eWiseMult, etc, but has been
% renamed to SECOND before calling the generated function.

codegen_binop_template ('min',    0, [ ], 'GB_IMIN (xarg, yarg)', 'fminf (xarg, yarg)', 'fmin (xarg, yarg)', 1) ;
codegen_binop_template ('max',    0, [ ], 'GB_IMAX (xarg, yarg)', 'fmaxf (xarg, yarg)', 'fmax (xarg, yarg)', 1) ;
codegen_binop_template ('plus',   0, [ ], '(xarg + yarg)'       , [ ]                 , [ ]                , 1) ;
codegen_binop_template ('minus',  0, [ ], '(xarg - yarg)'       , [ ]                 , [ ]                , 1) ;
codegen_binop_template ('rminus', 0, [ ], '(yarg - xarg)'       , [ ]                 , [ ]                , 1) ;
codegen_binop_template ('times',  0, [ ], '(xarg * yarg)'       , [ ]                 , [ ]                , 1) ;
codegen_binop_template ('div',    0, [ ], 'GB_IDIV (xarg, yarg)', '(xarg / yarg)'     , [ ]                , 1) ;
codegen_binop_template ('rdiv',   0, [ ], 'GB_IDIV (yarg, xarg)', '(yarg / xarg)'     , [ ]                , 1) ;

codegen_binop_template ('iseq',   0, [ ], '(xarg == yarg)') ;
codegen_binop_template ('isne',   0, [ ], '(xarg != yarg)') ;
codegen_binop_template ('isgt',   0, [ ], '(xarg > yarg)' ) ;
codegen_binop_template ('islt',   0, [ ], '(xarg < yarg)' ) ;
codegen_binop_template ('isge',   0, [ ], '(xarg >= yarg)') ;
codegen_binop_template ('isle',   0, [ ], '(xarg <= yarg)') ;

codegen_binop_template ('eq',     1, '(xarg == yarg)', '(xarg == yarg)') ;
codegen_binop_template ('ne',     1, [ ],              '(xarg != yarg)') ;
codegen_binop_template ('gt',     1, '(xarg > yarg)',  '(xarg > yarg)' ) ;
codegen_binop_template ('lt',     1, '(xarg < yarg)',  '(xarg < yarg)' ) ;
codegen_binop_template ('ge',     1, '(xarg >= yarg)', '(xarg >= yarg)') ;
codegen_binop_template ('le',     1, '(xarg <= yarg)', '(xarg <= yarg)') ;

codegen_binop_template ('lor',    0, '(xarg || yarg)', '((xarg != 0) || (yarg != 0))') ;
codegen_binop_template ('land',   0, '(xarg && yarg)', '((xarg != 0) && (yarg != 0))') ;
codegen_binop_template ('lxor',   0, '(xarg != yarg)', '((xarg != 0) != (yarg != 0))') ;

fprintf ('\n') ;


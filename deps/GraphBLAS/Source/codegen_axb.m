function codegen_axb
%CODEGEN_AXB create all C=A*B functions for all semirings
%
% This function creates all files of the form GB_AxB__*.[ch], including all
% built-in semirings (GB_AxB__*.c) and one include file, GB_AxB__include.h.

fprintf ('\nsemirings:\n') ;

f = fopen ('Generated/GB_AxB__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_AxB__include.h: definitions for GB_AxB__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.\n') ;
fprintf (f, '// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.\n') ;
fprintf (f, '\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_AxB.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

codegen_axb_template ('first',  'xarg', 'xarg') ;
codegen_axb_template ('second', 'yarg', 'yarg') ;
codegen_axb_template ('pair',   '1', '1') ;

% The ANY operator is not used as a multiplicative operator in the generated
% functions.  It can be used as the multiplicative op in a semiring, but is
% renamed to SECOND before calling the generated function.

codegen_axb_template ('min',    [ ], 'GB_IMIN (xarg, yarg)', 'fminf (xarg, yarg)', 'fmin (xarg, yarg)') ;
codegen_axb_template ('max',    [ ], 'GB_IMAX (xarg, yarg)', 'fmaxf (xarg, yarg)', 'fmax (xarg, yarg)') ;
codegen_axb_template ('plus',   [ ], '(xarg + yarg)') ;
codegen_axb_template ('minus',  [ ], '(xarg - yarg)') ;
codegen_axb_template ('rminus', [ ], '(yarg - xarg)') ;
codegen_axb_template ('times',  [ ], '(xarg * yarg)') ;
codegen_axb_template ('div',    [ ], 'GB_IDIV (xarg, yarg)', '(xarg / yarg)') ;
codegen_axb_template ('rdiv',   [ ], 'GB_IDIV (yarg, xarg)', '(yarg / xarg)') ;

codegen_axb_template ('iseq',   [ ], '(xarg == yarg)') ;
codegen_axb_template ('isne',   [ ], '(xarg != yarg)') ;
codegen_axb_template ('isgt',   [ ], '(xarg > yarg)' ) ;
codegen_axb_template ('islt',   [ ], '(xarg < yarg)' ) ;
codegen_axb_template ('isge',   [ ], '(xarg >= yarg)') ;
codegen_axb_template ('isle',   [ ], '(xarg <= yarg)') ;

codegen_axb_compare_template ('eq',     '(xarg == yarg)', '(xarg == yarg)') ;
codegen_axb_compare_template ('ne',     [ ],              '(xarg != yarg)') ;
codegen_axb_compare_template ('gt',     '(xarg > yarg)',  '(xarg > yarg)' ) ;
codegen_axb_compare_template ('lt',     '(xarg < yarg)',  '(xarg < yarg)' ) ;
codegen_axb_compare_template ('ge',     '(xarg >= yarg)', '(xarg >= yarg)') ;
codegen_axb_compare_template ('le',     '(xarg <= yarg)', '(xarg <= yarg)') ;

codegen_axb_template ('lor',  '(xarg || yarg)', '((xarg != 0) || (yarg != 0))') ;
codegen_axb_template ('land', '(xarg && yarg)', '((xarg != 0) && (yarg != 0))') ;
codegen_axb_template ('lxor', '(xarg != yarg)', '((xarg != 0) != (yarg != 0))') ;

fprintf ('\n') ;


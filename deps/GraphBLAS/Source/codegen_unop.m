function codegen_unop
%CODEGEN_UNOP create functions for all unary operators
%
% This function creates all files of the form GB_unaryop__*.[ch],
% and the include file GB_unaryop__include.h.

fprintf ('\nunary operators:\n') ;

f = fopen ('Generated/GB_unaryop__include.h', 'w') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '// GB_unaryop__include.h: definitions for GB_unaryop__*.c\n') ;
fprintf (f, '//------------------------------------------------------------------------------\n') ;
fprintf (f, '\n') ;
fprintf (f, '// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.\n') ;
fprintf (f, '// http://suitesparse.com   See GraphBLAS/Doc/License.txargt for license.\n') ;
fprintf (f, '\n') ;
fprintf (f, '// This file has been automatically generated from Generator/GB_unaryop.h') ;
fprintf (f, '\n\n') ;
fclose (f) ;

codegen_unop_template ('one',  ...
'true',  '1',                '1',              '1',             '1') ;
codegen_unop_template ('identity', ...
'xarg',  'xarg',            'xarg',            'xarg',         'xarg' ) ;
codegen_unop_template ('ainv', ...
'xarg',  '-xarg',           '-xarg',           '-xarg',        '-xarg' ) ;
codegen_unop_template ('abs', ...
'xarg',  'GB_IABS (xarg)',  'xarg',            'fabsf (xarg)', 'fabs (xarg)' ) ;
codegen_unop_template ('minv', ...
'true',  'GB_IMINV (xarg)', 'GB_IMINV (xarg)', '(1.0F)/xarg', '1./xarg' ) ;
codegen_unop_template ('lnot',  ...
'!xarg', '!(xarg != 0)',    '!(xarg != 0)',    '!(xarg != 0)', '!(xarg != 0)') ;

fprintf ('\n') ;


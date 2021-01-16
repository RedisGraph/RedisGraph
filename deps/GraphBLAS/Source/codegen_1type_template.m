function codegen_1type_method (xtype)
%CODEGEN_1TYPE_METHOD create a function to compute over a given type
%
% codegen_1type_method (xtype)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

f = fopen ('control.m4', 'w') ;

[fname, unsigned, bits] = codegen_type (xtype) ;
fprintf ('%-11s:  suffix: %-7s  unsigned: %d bits: %d\n', xtype, fname, unsigned, bits) ;

% function names
fprintf (f, 'define(`GB_Cdense_05d'', `GB_Cdense_05d__%s'')\n', fname) ;
fprintf (f, 'define(`GB_Cdense_06d'', `GB_Cdense_06d__%s'')\n', fname) ;
fprintf (f, 'define(`GB_Cdense_25'', `GB_Cdense_25__%s'')\n', fname) ;
fprintf (f, 'define(`GB_convert_s2b'', `GB_convert_s2b__%s'')\n', fname) ;

fprintf (f, 'define(`GB_ctype'', `%s'')\n', xtype) ;

% mask macro
if (isequal (xtype, 'GxB_FC32_t') || isequal (xtype, 'GxB_FC64_t'))
    asize = sprintf ('sizeof (%s)', xtype) ;
    fprintf (f, 'define(`GB_ax_mask'', `GB_mcast ((GB_void *) $1, $2, %s)'')\n', asize) ;
else
    fprintf (f, 'define(`GB_ax_mask'', `($1 [$2] != 0)'')\n') ;
end

% create the disable flag
disable = sprintf ('GxB_NO_%s', upper (fname)) ;
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;
fclose (f) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_type.c | m4 | tail -n +8 > Generated/GB_type__%s.c', ...
fname) ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_type.h | m4 | tail -n +8 >> Generated/GB_type__include.h') ;
system (cmd) ;

delete ('control.m4') ;


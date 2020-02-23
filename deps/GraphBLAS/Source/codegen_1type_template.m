function codegen_1type_method (xtype)
%CODEGEN_1TYPE_METHOD create a function to compute over a given type
%
% codegen_1type_method (xtype)

f = fopen ('control.m4', 'w') ;

[fname, unsigned, bits] = codegen_type (xtype) ;

% function names
fprintf (f, 'define(`GB_Cdense_05d'', `GB_Cdense_05d__%s'')\n', fname) ;
fprintf (f, 'define(`GB_Cdense_06d'', `GB_Cdense_06d__%s'')\n', fname) ;
fprintf (f, 'define(`GB_Cdense_25'', `GB_Cdense_25__%s'')\n', fname) ;

fprintf (f, 'define(`GB_ctype'', `%s'')\n', xtype) ;

% create the disable flag
disable = sprintf ('GxB_NO_%s', upper (fname)) ;
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;
fclose (f) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_type.c | m4 | tail -n +6 > Generated/GB_type__%s.c', ...
fname) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_type.h | m4 | tail -n +6 >> Generated/GB_type__include.h') ;
system (cmd) ;

delete ('control.m4') ;


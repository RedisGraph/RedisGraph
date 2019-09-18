function codegen_unop_method (unop, op, fcast, ztype, xtype)
%CODEGEN_UNOP_METHOD create a function to compute C=unop(cast(A))
%
% codegen_unop_method (unop, op, ztype, xtype)

f = fopen ('control.m4', 'w') ;

[zname, zunsigned, zbits] = codegen_type (ztype) ;
[xname, xunsigned, xbits] = codegen_type (xtype) ;

name = sprintf ('%s_%s_%s', unop, zname, xname) ;

% function names
fprintf (f, 'define(`GB_unop'', `GB_unop__%s'')\n', name) ;
fprintf (f, 'define(`GB_tran'', `GB_tran__%s'')\n', name) ;

% type of C and A
fprintf (f, 'define(`GB_ctype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`GB_atype'', `%s'')\n', xtype) ;

A_is_pattern = (isempty (strfind (op, 'xarg'))) ;

% to get an entry from A
if (A_is_pattern)
    % A(i,j) is not needed
    fprintf (f, 'define(`GB_geta'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_geta'', `%s $1 = $2 [$3]'')\n', xtype) ;
end

% type-specific IMINV
if (~isempty (strfind (op, 'IMINV')))
    if (zunsigned)
        op = strrep (op, 'IMINV', 'IMINV_UNSIGNED') ;
    else
        op = strrep (op, 'IMINV', 'IMINV_SIGNED') ;
    end
    op = strrep (op, ')', sprintf (', %d)', zbits)) ;
end

% create the unary operator
op = strrep (op, 'xarg', '`$2''') ;
fprintf (f, 'define(`GB_UNARYOP'', `$1 = %s'')\n', op) ;

% create the cast operator
if (A_is_pattern)
    % cast (A(i,j)) is not needed
    fprintf (f, 'define(`GB_CAST'', `;'')\n') ;
else
    fcast = strrep (fcast, 'zarg', '`$1''') ;
    fcast = strrep (fcast, 'xarg', '`$2''') ;
    fprintf (f, 'define(`GB_CAST'', `%s'')\n', fcast) ;
end

% create the disable flag
disable  = sprintf ('GxB_NO_%s', upper (unop)) ;
disable = [disable (sprintf (' || GxB_NO_%s', upper (zname)))] ;
if (~isequal (zname, xname))
    disable = [disable (sprintf (' || GxB_NO_%s', upper (xname)))] ;
end
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;
fclose (f) ;

% ff = fopen ('temp.h', 'a') ;
% fprintf (ff, '// #define GxB_NO_%s\n', upper (unop)) ;
% fclose (ff) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_unaryop.c | m4 | tail -n +9 > Generated/GB_unaryop__%s.c', ...
name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_unaryop.h | m4 | tail -n +9 >> Generated/GB_unaryop__include.h') ;
system (cmd) ;

delete ('control.m4') ;


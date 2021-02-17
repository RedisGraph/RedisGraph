function codegen_unop_method (unop, op, fcast, ztype, xtype)
%CODEGEN_UNOP_METHOD create a function to compute C=unop(cast(A))
%
% codegen_unop_method (unop, op, fcast, ztype, xtype)
%
%   unop: the name of the operator
%   op: a string defining the computation
%   ztype: the type of z for z=f(x)
%   xtype: the type of x for z=f(x)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

f = fopen ('control.m4', 'w') ;

[zname, zunsigned, zbits] = codegen_type (ztype) ;
[xname, xunsigned, xbits] = codegen_type (xtype) ;

name = sprintf ('%s_%s_%s', unop, zname, xname) ;

% determine if the op is identity with no typecast
is_identity = isequal (unop, 'identity') ;
no_typecast = isequal (ztype, xtype) ;
if (is_identity && no_typecast)
    fprintf (f, 'define(`GB_op_is_identity_with_no_typecast'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_op_is_identity_with_no_typecast'', `0'')\n') ;
end

% function names
fprintf (f, 'define(`GB_unop_apply'', `GB_unop_apply__%s'')\n', name) ;
fprintf (f, 'define(`GB_unop_tran'', `GB_unop_tran__%s'')\n', name) ;

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
fprintf (f, 'define(`GB_unaryop'', `$1 = %s'')\n', op) ;

% create the cast operator
if (A_is_pattern)
    % cast (A(i,j)) is not needed
    fprintf (f, 'define(`GB_cast'', `;'')\n') ;
else
    fcast = strrep (fcast, 'zarg', '`$1''') ;
    fcast = strrep (fcast, 'xarg', '`$2''') ;
    fprintf (f, 'define(`GB_cast'', `%s'')\n', fcast) ;
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
'cat control.m4 Generator/GB_unop.c | m4 | tail -n +10 > Generated/GB_unop__%s.c', ...
name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_unop.h | m4 | tail -n +10 >> Generated/GB_unop__include.h') ;
system (cmd) ;

delete ('control.m4') ;


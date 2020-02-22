function codegen_red_method (opname, func, atype, identity, terminal, panel)
%CODEGEN_RED_METHOD create a reduction function, C = reduce (A)
%
% codegen_red_method (opname, func, atype, identity, terminal)

f = fopen ('control.m4', 'w') ;

[aname, unsigned, bits] = codegen_type (atype) ;

name = sprintf ('%s_%s', opname, aname) ;
is_any = isequal (opname, 'any') ;

% function names
fprintf (f, 'define(`GB_red_build'', `GB_red_build__%s'')\n', name) ;

% the type of A and C (no typecasting)
fprintf (f, 'define(`GB_atype'', `%s'')\n', atype) ;
fprintf (f, 'define(`GB_ctype'', `%s'')\n', atype) ;

if (~isempty (identity))
    fprintf (f, 'define(`GB_red_scalar'',    `GB_red_scalar__%s'')\n',    name);
    fprintf (f, 'define(`GB_red_eachvec'',   `GB_red_eachvec__%s'')\n',   name);
    fprintf (f, 'define(`GB_red_eachindex'', `GB_red_eachindex__%s'')\n', name);
    % identity and terminal values for the monoid
    fprintf (f, 'define(`GB_identity'', `%s'')\n', identity) ;
    fprintf (f, 'define(`if_is_monoid'', `'')\n') ;
    fprintf (f, 'define(`endif_is_monoid'', `'')\n') ;
else
    fprintf (f, 'define(`GB_red_scalar'',    `GB_red_scalar__(none)'')\n') ;
    fprintf (f, 'define(`GB_red_eachvec'',   `GB_red_eachvec__(none)'')\n') ;
    fprintf (f, 'define(`GB_red_eachindex'', `GB_red_eachindex__(none)'')\n') ;
    % first and second operators are not monoids (GB_red_build only)
    fprintf (f, 'define(`GB_identity'', `(none)'')\n') ;
    fprintf (f, 'define(`if_is_monoid'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_is_monoid'', `#endif'')\n') ;
end

if (is_any)
    fprintf (f, 'define(`GB_is_any_monoid'', `1'')\n') ;
    fprintf (f, 'define(`GB_has_terminal'', `1'')\n') ;
    fprintf (f, 'define(`GB_terminal_value'', `(any value)'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `break ;'')\n') ;
elseif (~isempty (terminal))
    fprintf (f, 'define(`GB_is_any_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_has_terminal'', `1'')\n') ;
    fprintf (f, 'define(`GB_terminal_value'', `%s'')\n', terminal) ;
    fprintf (f, 'define(`GB_terminal'', `if (s == %s) break ;'')\n', terminal) ;
else
    fprintf (f, 'define(`GB_is_any_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_has_terminal'', `0'')\n') ;
    fprintf (f, 'define(`GB_terminal_value'', `(none)'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `;'')\n') ;
end

if (is_any)
    fprintf (f, 'define(`GB_panel'', `(no panel)'')\n') ;
else
    fprintf (f, 'define(`GB_panel'', `%d'')\n', panel) ;
end

% create the operator
func = strrep (func, 'zarg', '`$1''') ;
func = strrep (func, 'yarg', '`$2''') ;
fprintf (f, 'define(`GB_REDUCE_OP'', `%s'')\n', func) ;

% create the disable flag
disable  = sprintf ('GxB_NO_%s', upper (opname)) ;
disable = [disable (sprintf (' || GxB_NO_%s', upper (aname)))] ;
disable = [disable (sprintf (' || GxB_NO_%s_%s', upper (opname), upper (aname)))] ;
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;

fclose (f) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_red.c | m4 | tail -n +17 > Generated/GB_red__%s.c', ...
name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_red.h | m4 | tail -n +17 >> Generated/GB_red__include.h') ;
system (cmd) ;

delete ('control.m4') ;


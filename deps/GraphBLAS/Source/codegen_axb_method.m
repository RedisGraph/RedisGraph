function codegen_axb_method (addop, multop, add, addfunc, mult, ztype, xytype, identity, terminal, omp_atomic)
%CODEGEN_AXB_METHOD create a function to compute C=A*B over a semiring
%
% codegen_axb_method (addop, multop, add, addfunc, mult, ztype, xytype, identity, terminal, omp_atomic)

f = fopen ('control.m4', 'w') ;

is_first    = isequal (multop, 'first') ;
is_second   = isequal (multop, 'second') ;
is_pair     = isequal (multop, 'pair') ;
is_any      = isequal (addop, 'any') ;
is_eq       = isequal (addop, 'eq') ;
is_any_pair = is_any && isequal (multop, 'pair') ;
is_real     = isequal (ztype, 'float') || isequal (ztype, 'double') ;

% special cases for the PAIR multiplier
switch (ztype)
    case { 'bool' }
        bits = '0x1L' ;
    case { 'int8_t', 'uint8_t' }
        bits = '0xffL' ;
    case { 'int16_t', 'uint16_t' }
        bits = '0xffffL' ;
    case { 'int32_t', 'uint32_t' }
        bits = '0xffffffffL' ;
    case { 'int64_t', 'uint64_t' }
        bits = '0' ;
    case { 'float', 'double' }
        bits = '0' ;
    otherwise
        error ('unknown type') ;
end
fprintf (f, 'define(`GB_ctype_bits'', `%s'')\n', bits) ;

if isequal (addop, 'plus') && isequal (multop, 'times') && isequal (ztype, 'float')
    % plus_times_fp32 semiring
end

if (is_pair)
    % these semirings are renamed to any_pair, and not thus created
    if (isequal (addop, 'land') || isequal (addop, 'eq'   ) || ...
        isequal (addop, 'lor' ) || isequal (addop, 'max'  ) || ...
        isequal (addop, 'min' ) || isequal (addop, 'times'))
        return
    end
end

[fname, unsigned, bits] = codegen_type (xytype) ;
[zname, ~, ~] = codegen_type (ztype) ;

name = sprintf ('%s_%s_%s', addop, multop, fname) ;

% function names
fprintf (f, 'define(`GB_AgusB'', `GB_AgusB__%s'')\n', name) ;
fprintf (f, 'define(`GB_Adot2B'', `GB_Adot2B__%s'')\n', name) ;
fprintf (f, 'define(`GB_Adot3B'', `GB_Adot3B__%s'')\n', name) ;
fprintf (f, 'define(`GB_Adot4B'', `GB_Adot4B__%s'')\n', name) ;
fprintf (f, 'define(`GB_AheapB'', `GB_AheapB__%s'')\n', name) ;
fprintf (f, 'define(`GB_Asaxpy3B'', `GB_Asaxpy3B__%s'')\n', name) ;

% type of C, A, and B
fprintf (f, 'define(`GB_ctype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`GB_atype'', `%s'')\n', xytype) ;
fprintf (f, 'define(`GB_btype'', `%s'')\n', xytype) ;

% identity and terminal values for the monoid
fprintf (f, 'define(`GB_identity'', `%s'')\n', identity) ;

if (is_any_pair)
    fprintf (f, 'define(`GB_is_any_pair_semiring'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_any_pair_semiring'', `0'')\n') ;
end

if (is_pair)
    fprintf (f, 'define(`GB_is_pair_multiplier'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_pair_multiplier'', `0'')\n') ;
end

if (is_eq)
    fprintf (f, 'define(`GB_is_eq_monoid'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_eq_monoid'', `0'')\n') ;
end

if (is_any)
    % the ANY monoid terminates on the first entry seen
    fprintf (f, 'define(`GB_is_any_monoid'', `1'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `break ;'')\n') ;
    fprintf (f, 'define(`GB_dot_simd_vectorize'', `;'')\n') ;
elseif (~isempty (terminal))
    fprintf (f, 'define(`GB_is_any_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `if (cij == %s) break ;'')\n', terminal) ;
    fprintf (f, 'define(`GB_dot_simd_vectorize'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_is_any_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `;'')\n') ;
    fprintf (f, 'define(`GB_dot_simd_vectorize'', `GB_PRAGMA_SIMD'')\n') ;
end

% all built-in monoids are atomic
fprintf (f, 'define(`GB_has_atomic'', `1'')\n') ;

% only PLUS, TIMES, LOR, LAND, and LXOR can be done with OpenMP atomics
fprintf (f, 'define(`GB_has_omp_atomic'', `%d'')\n', omp_atomic) ;

% MIN and MAX for floating-point types need unsigned integer puns
% pun for compare-and-swap of ztype
if (isequal (ztype, 'float'))
    pun = 'uint32_t' ;
elseif (isequal (ztype, 'double'))
    pun = 'uint64_t' ;
else
    % no type punning needed for compare-and-swap
    pun = ztype ;
end
fprintf (f, 'define(`GB_ctype_pun'', `%s'')\n', pun) ;

% to get an entry from A
if (is_second || is_pair)
    % value of A is ignored for the SECOND and PAIR operators
    fprintf (f, 'define(`GB_geta'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_geta'', `%s $1 = $2 [$3]'')\n', xytype) ;
end

% to get an entry from B
if (is_first || is_pair)
    % value of B is ignored for the FIRST and PAIR operators
    fprintf (f, 'define(`GB_getb'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_getb'', `%s $1 = $2 [$3]'')\n', xytype) ;
end

% type-specific IDIV
if (~isempty (strfind (mult, 'IDIV')))
    if (unsigned)
        mult = strrep (mult, 'IDIV', 'IDIV_UNSIGNED') ;
    else
        mult = strrep (mult, 'IDIV', 'IDIV_SIGNED') ;
    end
    mult = strrep (mult, ')', sprintf (', %d)', bits)) ;
end

% create the multiply operator
mult2 = strrep (mult,  'xarg', '`$2''') ;
mult2 = strrep (mult2, 'yarg', '`$3''') ;
fprintf (f, 'define(`GB_MULTIPLY'', `$1 = %s'')\n', mult2) ;

% create the add operator, of the form w += t
add2 = strrep (add,  'w', '`$1''') ;
add2 = strrep (add2, 't', '`$2''') ;
fprintf (f, 'define(`GB_add_update'', `%s'')\n', add2) ;

% create the add function, of the form w + t
add2 = strrep (addfunc,  'w', '`$1''') ;
add2 = strrep (add2,     't', '`$2''') ;
fprintf (f, 'define(`GB_add_function'', `%s'')\n', add2) ;

% create the multiply-add operator
if (isequal (ztype, 'float') || isequal (ztype, 'double') || ...
    isequal (ztype, 'bool') || is_first || is_second || is_pair || ...
    isequal (multop (1:2), 'is') || isequal (multop, 'any'))
    % float and double do not get promoted.
    % bool is OK since promotion of the result (0 or 1) to int is safe.
    % first and second are OK since no promotion occurs.
    % is* operators are OK too.
    multadd = strrep (add, 't',  mult) ;
    multadd = strrep (multadd, 'w', '`$1''') ;
    multadd = strrep (multadd, 'xarg', '`$2''') ;
    multadd = strrep (multadd, 'yarg', '`$3''') ;
    fprintf (f, 'define(`GB_multiply_add'', `%s'')\n', multadd) ;
else
    % use explicit typecasting to avoid ANSI C integer promotion.
    add2 = strrep (add,  'w', '`$1''') ;
    add2 = strrep (add2, 't', 'x_op_y') ;
    fprintf (f, 'define(`GB_multiply_add'', `%s x_op_y = %s ; %s'')\n', ...
        ztype, mult2, add2) ;
end

% create the disable flag
disable  = sprintf ('GxB_NO_%s', upper (addop)) ;
if (~isequal (addop, multop))
    disable = [disable (sprintf (' || GxB_NO_%s', upper (multop)))] ;
end
disable = [disable (sprintf (' || GxB_NO_%s', upper (fname)))] ;
disable = [disable (sprintf (' || GxB_NO_%s_%s', upper (addop), upper (zname)))] ;
if (~ (isequal (addop, multop) && isequal (zname, fname)))
    disable = [disable (sprintf (' || GxB_NO_%s_%s', upper (multop), upper (fname)))] ;
end
disable = [disable (sprintf (' || GxB_NO_%s_%s_%s', ...
    upper (addop), upper (multop), upper (fname))) ] ;
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;
fclose (f) ;

% To create GB_control.h
% ff = fopen ('temp.h', 'a') ;
% fprintf (ff, '// #define GxB_NO_%s\n', upper (addop)) ;
% fprintf (ff, '// #define GxB_NO_%s\n', upper (multop)) ;
% fprintf (ff, '//  #define GxB_NO_%s\n', upper (fname)) ;
% fprintf (ff, '//   #define GxB_NO_%s_%s_%s\n', upper (addop), upper (multop), upper (fname)) ;
% fclose (ff) ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_AxB.c | m4 | tail -n +28 > Generated/GB_AxB__%s.c', ...
name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_AxB.h | m4 | tail -n +28 >> Generated/GB_AxB__include.h') ;
system (cmd) ;

delete ('control.m4') ;


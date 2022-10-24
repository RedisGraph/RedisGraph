function codegen_binop_method (binop, op, xtype)
%CODEGEN_BINOP_METHOD create a function to compute C=binop(A,B)
%
% codegen_binop_method (binop, op, xtype)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

f = fopen ('control.m4', 'w') ;

% no code is generated for the ANY operator (SECOND is used in its place)
assert (~isequal (binop, 'any')) ;

[fname, unsigned, bits] = codegen_type (xtype) ;

name = sprintf ('%s_%s', binop, fname) ;

% function names
fprintf (f, 'define(`_AaddB'', `_AaddB__%s'')\n', name) ;

fprintf (f, 'define(`_Cdense_accumB'', `_Cdense_accumB__%s'')\n', name) ;
fprintf (f, 'define(`_Cdense_accumb'', `_Cdense_accumb__%s'')\n', name) ;
fprintf (f, 'define(`_Cdense_ewise3_noaccum'', `_Cdense_ewise3_noaccum__%s'')\n', name) ;

% subset of operators for GB_dense_ewise3_accum
switch (binop)
    case { 'min', 'max', 'plus', 'minus', 'rminus', 'times', 'div', 'rdiv' }
        % these operators are used in ewise3_accum
        fprintf (f, 'define(`_Cdense_ewise3_accum'', `_Cdense_ewise3_accum__%s'')\n', name) ;
        fprintf (f, 'define(`if_is_binop_subset'', `'')\n') ;
        fprintf (f, 'define(`endif_is_binop_subset'', `'')\n') ;
    otherwise
        fprintf (f, 'define(`_Cdense_ewise3_accum'', `(none)'')\n') ;
        fprintf (f, 'define(`if_is_binop_subset'', `#if 0'')\n') ;
        fprintf (f, 'define(`endif_is_binop_subset'', `#endif'')\n') ;
end

% subset of operators for GB_AxB_rowscale and GB_AxB_colscale
switch (binop)
    case { 'min', 'max', 'plus', 'minus', 'rminus', 'times', 'div', 'rdiv', ...
        'first', 'second', 'isgt', 'islt', 'isge', 'isle', ...
        'gt', 'lt', 'ge', 'le', 'lor', 'land', 'lxor' }
        % these operators are used in GB_AxB_*scale
        binop_is_semiring_multiplier = true ;
    case { 'eq', 'iseq', 'ne', 'isne' }
        % these do not appear in complex semirings
        binop_is_semiring_multiplier = (~codegen_contains (xtype, 'FC')) ;
    case { 'bor', 'band', 'bxor', 'bxnor' }
        % these operators are used in GB_AxB_*scale for uint* only
        binop_is_semiring_multiplier = codegen_contains (xtype, 'uint') ;
    otherwise
        % these operators are not used in GB_AxB_*scale by any builtin semiring
        binop_is_semiring_multiplier = false ;
end
if (binop_is_semiring_multiplier)
    fprintf (f, 'define(`_AxD'', `_AxD__%s'')\n', name) ;
    fprintf (f, 'define(`_DxB'', `_DxB__%s'')\n', name) ;
    fprintf (f, 'define(`if_binop_is_semiring_multiplier'', `'')\n') ;
    fprintf (f, 'define(`endif_binop_is_semiring_multiplier'', `'')\n') ;
else
    fprintf (f, 'define(`_AxD'', `(none)'')\n') ;
    fprintf (f, 'define(`_DxB'', `(none)'')\n') ;
    fprintf (f, 'define(`if_binop_is_semiring_multiplier'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_binop_is_semiring_multiplier'', `#endif'')\n') ;
end

% subset of operators for GB_apply
switch (binop)
    case { 'first', 'second', 'any', 'pair' }
        % no bind1st or bind2nd for these operators
        fprintf (f, 'define(`_bind1st'', `(none)'')\n', name) ;
        fprintf (f, 'define(`_bind1st_tran'', `(none)'')\n', name) ;
        fprintf (f, 'define(`_bind2nd'', `(none)'')\n', name) ;
        fprintf (f, 'define(`_bind2nd_tran'', `(none)'')\n', name) ;
        fprintf (f, 'define(`if_binop_bind_is_enabled'', `#if 0'')\n') ;
        fprintf (f, 'define(`endif_binop_bind_is_enabled'', `#endif'')\n') ;
    otherwise
        fprintf (f, 'define(`_bind1st'', `_bind1st__%s'')\n', name) ;
        fprintf (f, 'define(`_bind1st_tran'', `_bind1st_tran__%s'')\n', name) ;
        fprintf (f, 'define(`_bind2nd'', `_bind2nd__%s'')\n', name) ;
        fprintf (f, 'define(`_bind2nd_tran'', `_bind2nd_tran__%s'')\n', name) ;
        fprintf (f, 'define(`if_binop_bind_is_enabled'', `'')\n') ;
        fprintf (f, 'define(`endif_binop_bind_is_enabled'', `'')\n') ;
end

% subset of operators for GB_emult
switch (binop)
    case { 'pair' }
        % no emult for these operators
        fprintf (f, 'define(`_AemultB_08'', `(none)'')\n') ;
        fprintf (f, 'define(`_AemultB_02'', `(none)'')\n') ;
        fprintf (f, 'define(`_AemultB_04'', `(none)'')\n') ;
        fprintf (f, 'define(`_AemultB_bitmap'', `(none)'')\n') ;
        fprintf (f, 'define(`if_binop_emult_is_enabled'', `#if 0'')\n') ;
        fprintf (f, 'define(`endif_binop_emult_is_enabled'', `#endif'')\n') ;
    otherwise
        fprintf (f, 'define(`_AemultB_08'', `_AemultB_08__%s'')\n', name) ;
        fprintf (f, 'define(`_AemultB_02'', `_AemultB_02__%s'')\n', name) ;
        fprintf (f, 'define(`_AemultB_04'', `_AemultB_04__%s'')\n', name) ;
        fprintf (f, 'define(`_AemultB_bitmap'', `_AemultB_bitmap__%s'')\n', name) ;
        fprintf (f, 'define(`if_binop_emult_is_enabled'', `'')\n') ;
        fprintf (f, 'define(`endif_binop_emult_is_enabled'', `'')\n') ;
end

if (isequal (binop, 'second'))
    fprintf (f, 'define(`GB_op_is_second'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_op_is_second'', `0'')\n') ;
end

% determine type of z, x, and y from xtype and binop
switch (binop)
    case { 'eq', 'ne', 'gt', 'lt', 'ge', 'le' }
        % GrB_LT_* and related operators are TxT -> bool
        ztype = 'bool' ;
        ytype = xtype ;
    case { 'cmplx' }
        % GxB_CMPLX_* are TxT -> (complex T)
        if (isequal (xtype, 'float'))
            ztype = 'GxB_FC32_t' ;
        else
            ztype = 'GxB_FC64_t' ;
        end
        ytype = xtype ;
    case { 'bshift' }
        % z = bitshift (x,y): y is always int8
        ztype = xtype ;
        ytype = 'int8_t' ;
    otherwise
        % all other operators: z, x, and y have the same type
        ztype = xtype ;
        ytype = xtype ;
end

fprintf (f, 'define(`GB_ctype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`GB_atype'', `%s'')\n', xtype) ;
fprintf (f, 'define(`GB_btype'', `%s'')\n', ytype) ;

fprintf (f, 'define(`GB_atype_is_btype'', `%d'')\n', isequal (xtype, ytype)) ;
fprintf (f, 'define(`GB_ctype_is_atype'', `%d'')\n', isequal (ztype, xtype)) ;
fprintf (f, 'define(`GB_ctype_is_btype'', `%d'')\n', isequal (ztype, ytype)) ;

% C_dense_update: operators z=f(x,y) where ztype and xtype match, and op is not 'first'
if (isequal (xtype, ztype) && ~isequal (binop, 'first'))
    fprintf (f, 'define(`GB_C_dense_update'', `1'')\n') ;
    fprintf (f, 'define(`if_C_dense_update'', `'')\n') ;
    fprintf (f, 'define(`endif_C_dense_update'', `'')\n') ;
else
    fprintf (f, 'define(`GB_C_dense_update'', `0'')\n') ;
    fprintf (f, 'define(`if_C_dense_update'', `#if 0'')\n') ;
    fprintf (f, 'define(`endif_C_dense_update'', `#endif'')\n') ;
end

% to get an entry from A
if (isequal (binop, 'second') || isequal (binop, 'pair'))
    % the value of A is ignored
    fprintf (f, 'define(`GB_geta'', `;'')\n') ;
    fprintf (f, 'define(`GB_a_is_pattern'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_geta'', `%s $1 = GBX ($2, $3, $4)'')\n', xtype) ;
    fprintf (f, 'define(`GB_a_is_pattern'', `0'')\n') ;
end

% to get an entry from B
if (isequal (binop, 'first') || isequal (binop, 'pair'))
    % the value of B is ignored
    fprintf (f, 'define(`GB_getb'', `;'')\n') ;
    fprintf (f, 'define(`GB_b_is_pattern'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_getb'', `%s $1 = GBX ($2, $3, $4)'')\n', ytype) ;
    fprintf (f, 'define(`GB_b_is_pattern'', `0'')\n') ;
end

% to copy an entry from A to C
if (isequal (xtype, 'GxB_FC32_t') && isequal (ztype, 'bool'))
    fprintf (f, 'define(`GB_copy_a_to_c'', `$1 = (crealf (GBX ($2, $3, $4)) != 0) || (cimagf (GBX ($2, $3, $4)) != 0)'')\n') ;
elseif (isequal (xtype, 'GxB_FC64_t') && isequal (ztype, 'bool'))
    fprintf (f, 'define(`GB_copy_a_to_c'', `$1 = (creal (GBX ($2, $3, $4)) != 0) || (cimag (GBX ($2, $3, $4)) != 0)'')\n') ;
elseif (isequal (xtype, 'float') && isequal (ztype, 'GxB_FC32_t'))
    fprintf (f, 'define(`GB_copy_a_to_c'', `$1 = GxB_CMPLXF (GBX ($2, $3, $4), 0)'')\n') ;
elseif (isequal (xtype, 'double') && isequal (ztype, 'GxB_FC64_t'))
    fprintf (f, 'define(`GB_copy_a_to_c'', `$1 = GxB_CMPLX (GBX ($2, $3, $4), 0)'')\n') ;
else
    fprintf (f, 'define(`GB_copy_a_to_c'', `$1 = GBX ($2, $3, $4)'')\n') ;
end

% to copy an entry from B to C
if (isequal (ytype, 'GxB_FC32_t') && isequal (ztype, 'bool'))
    fprintf (f, 'define(`GB_copy_b_to_c'', `$1 = (crealf (GBX ($2, $3, $4)) != 0) || (cimagf (GBX ($2, $3, $4)) != 0)'')\n') ;
elseif (isequal (ytype, 'GxB_FC64_t') && isequal (ztype, 'bool'))
    fprintf (f, 'define(`GB_copy_b_to_c'', `$1 = (creal (GBX ($2, $3, $4)) != 0) || (cimag (GBX ($2, $3, $4)) != 0)'')\n') ;
elseif (isequal (ytype, 'float') && isequal (ztype, 'GxB_FC32_t'))
    fprintf (f, 'define(`GB_copy_b_to_c'', `$1 = GxB_CMPLXF (GBX ($2, $3, $4), 0)'')\n') ;
elseif (isequal (ytype, 'double') && isequal (ztype, 'GxB_FC64_t'))
    fprintf (f, 'define(`GB_copy_b_to_c'', `$1 = GxB_CMPLX (GBX ($2, $3, $4), 0)'')\n') ;
else
    fprintf (f, 'define(`GB_copy_b_to_c'', `$1 = GBX ($2, $3, $4)'')\n') ;
end

% type-specific IDIV
if (~isempty (strfind (op, 'IDIV')))
    if (unsigned)
        op = strrep (op, 'IDIV', 'IDIV_UNSIGNED') ;
        % TODO: use this instead
        % mult = strrep (op, 'IDIV', 'idiv_uint%d', bits) ;
    else
        op = strrep (op, 'IDIV', 'IDIV_SIGNED') ;
        % TODO: use this instead
        % mult = strrep (op, 'IDIV', 'idiv_int%d', bits) ;
    end
    % TODO: remove this
    op = strrep (op, ')', sprintf (', %d)', bits)) ;
end

% create the binary operator
op = strrep (op,  'xarg', '`$2''') ;
op = strrep (op, 'yarg', '`$3''') ;
fprintf (f, 'define(`GB_binaryop'', `$1 = %s'')\n', op) ;

% handle the flip
switch (binop)
    case { 'pow', 'bget', 'bset', 'bclr', 'bshift', 'atan2', 'fmod', ...
        'remainder', 'copysign', 'ldexp', 'cmplx' }
        fprintf (f, 'define(`GB_binaryop_flip'', `1'')\n') ;
    otherwise
        fprintf (f, 'define(`GB_binaryop_flip'', `0'')\n') ;
end

% create the disable flag
disable = sprintf ('GxB_NO_%s', upper (binop)) ;
disable = [disable (sprintf (' || GxB_NO_%s', upper (fname)))] ;
disable = [disable (sprintf (' || GxB_NO_%s_%s', upper (binop), upper (fname)))] ;
fprintf (f, 'define(`GB_disable'', `(%s)'')\n', disable) ;

% ff = fopen ('temp.h', 'a') ;
% fprintf (ff, '// #define GxB_NO_%s_%s\n',  upper (binop), upper (fname)) ;
% fclose (ff) ;

fclose (f) ;

trim = 42 ;

% construct the *.c file
cmd = sprintf (...
'cat control.m4 Generator/GB_binop.c | m4 | tail -n +%d > Generated2/GB_binop__%s.c', ...
trim, name) ;
fprintf ('.') ;
system (cmd) ;

% append to the *.h file
cmd = sprintf (...
'cat control.m4 Generator/GB_binop.h | m4 | tail -n +%d >> Generated2/GB_binop__include.h', trim) ;
system (cmd) ;

delete ('control.m4') ;


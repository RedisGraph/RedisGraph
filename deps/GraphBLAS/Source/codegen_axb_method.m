function codegen_axb_method (addop, multop, add, addfunc, mult, ztype, ...
    xytype, identity, terminal, omp_atomic, omp_microsoft_atomic)
%CODEGEN_AXB_METHOD create a function to compute C=A*B over a semiring
%
% codegen_axb_method (addop, multop, add, addfunc, mult, ztype, xytype, ...
%   identity, terminal, omp_atomic, omp_microsoft_atomic)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (nargin >= 5 && isempty (mult))
    return
end

f = fopen ('control.m4', 'w') ;

is_first  = false ;
is_second = false ;
is_pair   = false ;
is_positional = false ;
switch (multop)
    case { 'firsti', 'firsti1', 'firstj', 'firstj1', 'secondj', 'secondj1' }
        is_positional = true ;
    case { 'first' }
        is_first = true ;
    case { 'second' }
        is_second = true ;
    case { 'pair' }
        is_pair = true ;
end

is_any = isequal (addop, 'any') ;
is_max = isequal (addop, 'max') ;
is_min = isequal (addop, 'min') ;
is_eq  = isequal (addop, 'eq') ;
is_any_pair = is_any && isequal (multop, 'pair') ;

if (is_any_pair)
    % the any_pair_iso semiring does not access any numerical values
    add = ' ' ;
    addfunc = ' ' ;
    mult = ' ' ;
    ztype = 'iso' ;
    xytype = 'any type' ;
    identity = ' ' ;
    terminal = 'break' ;
    omp_atomic = 1 ;
    omp_microsoft_atomic = 0 ;
    % the any_pair_iso semiring is never disabled by GBCUDA_DEV
    fprintf (f, 'define(`ifndef_GBCUDA_DEV'', `#if 1'')\n') ;
    fprintf (f, 'define(`if_not_any_pair_semiring'', `#if 0'')\n') ;
else
    % all other semirings are disabled by GBCUDA_DEV
    fprintf (f, 'define(`ifndef_GBCUDA_DEV'', `#ifndef GBCUDA_DEV'')\n') ;
    fprintf (f, 'define(`if_not_any_pair_semiring'', `#if 1'')\n') ;
end

ztype_is_real = ~codegen_contains (ztype, 'FC') ;
ztype_is_fp = isequal (ztype, 'float') || isequal (ztype, 'double') ;
is_any_complex = is_any && ~ztype_is_real ;
is_plus_pair_real  = isequal (addop, 'plus') && isequal (multop, 'pair' ) && ztype_is_real ;
is_plus_times_fp = isequal (addop, 'plus') && isequal (multop, 'times') && ztype_is_fp ;

t_is_simple = isequal (multop, 'pair') || codegen_contains (multop, 'first') || codegen_contains (multop, 'second') ;
t_is_nonnan = isequal (multop (1:2), 'is') || (multop (1) == 'l') ;

switch (ztype)
    case { 'iso' }
        ztype_is_float = false ;
        ztype_ignore_overflow = true ;
        nbits = 0 ;
        bits = '0' ;
    case { 'bool' }
        ztype_is_float = false ;
        ztype_ignore_overflow = false ;
        nbits = 8 ;
        bits = '0x1L' ;
    case { 'int8_t', 'uint8_t' }
        ztype_is_float = false ;
        ztype_ignore_overflow = false ;
        nbits = 8 ;
        bits = '0xffL' ;
        xbits = '0xFF' ;
    case { 'int16_t', 'uint16_t' }
        ztype_is_float = false ;
        ztype_ignore_overflow = false ;
        nbits = 16 ;
        bits = '0xffffL' ;
        xbits = '0xFFFF' ;
    case { 'int32_t', 'uint32_t' }
        ztype_is_float = false ;
        ztype_ignore_overflow = false ;
        nbits = 32 ;
        bits = '0xffffffffL' ;
        xbits = '0xFFFFFFFF' ;
    case { 'int64_t', 'uint64_t' }
        ztype_is_float = false ;
        ztype_ignore_overflow = true ;
        nbits = 64 ;
        bits = '0' ;
        xbits = '0xFFFFFFFFFFFFFFFFL' ;
    case { 'float' }
        ztype_is_float = true ;
        ztype_ignore_overflow = true ;
        nbits = 32 ;
        bits = '0' ;
    case { 'double', 'GxB_FC32_t' }
        ztype_is_float = true ;
        ztype_ignore_overflow = true ;
        nbits = 64 ;
        bits = '0' ;
    case { 'GxB_FC64_t' }
        ztype_is_float = true ;
        ztype_ignore_overflow = true ;
        nbits = 128 ;
        bits = '0' ;
    otherwise
        error ('unknown type') ;
end

% bits: special cases for the PAIR multiplier
fprintf (f, 'define(`GB_ctype_bits'', `%s'')\n', bits) ;
fprintf (f, 'define(`GB_cnbits'', `%d'')\n', nbits) ;

% nbits: # of bits in the type, needed for the atomic compare-exchange:
if (nbits == 0)
    % iso semiring: no atomic compare-exchanged needed
    fprintf (f, 'define(`GB_atomic_compare_exchange'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_atomic_compare_exchange'', `GB_ATOMIC_COMPARE_EXCHANGE_%d (target, expected, desired)'')\n', nbits) ;
end

if (is_pair)
    % these semirings are renamed to any_pair, and not thus created
    if (isequal (addop, 'land') || isequal (addop, 'eq'   ) || ...
        isequal (addop, 'lor' ) || isequal (addop, 'max'  ) || ...
        isequal (addop, 'min' ) || isequal (addop, 'times'))
        return
    end
end

if (is_any_pair)
    fname = 'iso' ;
    unsigned = true ;
    bits = 0 ;
    zname = 'iso' ;
else
    [fname, unsigned, bits] = codegen_type (xytype) ;
    [zname, ~, ~] = codegen_type (ztype) ;
end

name = sprintf ('%s_%s_%s', addop, multop, fname) ;

% function names
fprintf (f, 'define(`_Adot2B'', `_Adot2B__%s'')\n', name) ;
fprintf (f, 'define(`_Adot3B'', `_Adot3B__%s'')\n', name) ;
fprintf (f, 'define(`_Asaxpy3B'', `_Asaxpy3B__%s'')\n', name) ;
fprintf (f, 'define(`_Asaxpy3B_M'', `_Asaxpy3B_M__%s'')\n', name) ;
fprintf (f, 'define(`_Asaxpy3B_noM'', `_Asaxpy3B_noM__%s'')\n', name) ;
fprintf (f, 'define(`_Asaxpy3B_notM'', `_Asaxpy3B_notM__%s'')\n', name) ;
fprintf (f, 'define(`_AsaxbitB'', `_AsaxbitB__%s'')\n', name) ;
fprintf (f, 'define(`GB_AxB'', `GB_AxB__%s'')\n', name) ;

% type of C, A, and B
fprintf (f, 'define(`GB_ctype'', `%s'')\n', ztype) ;
fprintf (f, 'define(`GB_atype'', `%s'')\n', xytype) ;
fprintf (f, 'define(`GB_btype'', `%s'')\n', xytype) ;

if (is_any_pair)
    fprintf (f, 'define(`GB_csize'', `0'')\n', ztype) ;
    fprintf (f, 'define(`GB_asize'', `0'')\n', xytype) ;
    fprintf (f, 'define(`GB_bsize'', `0'')\n', xytype) ;
else
    fprintf (f, 'define(`GB_csize'', `sizeof (%s)'')\n', ztype) ;
    fprintf (f, 'define(`GB_asize'', `sizeof (%s)'')\n', xytype) ;
    fprintf (f, 'define(`GB_bsize'', `sizeof (%s)'')\n', xytype) ;
end

% flag if ztype can ignore overflow in some computations
fprintf (f, 'define(`GB_ctype_ignore_overflow'', `%d'')\n', ztype_ignore_overflow) ;

% simple typecast from 1 (or 2) real scalars to any other type
switch (ztype)
    case { 'GxB_FC32_t' }
        fprintf (f, 'define(`GB_ctype_cast'', `GxB_CMPLXF (((float) $1), ((float) $2))'')\n') ;
    case { 'GxB_FC64_t' }
        fprintf (f, 'define(`GB_ctype_cast'', `GxB_CMPLX (((double) $1), ((double) $2))'')\n') ;
    case { 'iso' }
        fprintf (f, 'define(`GB_ctype_cast'', `'')\n') ;
    otherwise
        fprintf (f, 'define(`GB_ctype_cast'', `((GB_ctype) $1)'')\n') ;
end

% simple typecast from 1 (or 2) real scalars to any other type
switch (xytype)
    case { 'GxB_FC32_t' }
        fprintf (f, 'define(`GB_atype_cast'', `GxB_CMPLXF (((float) $1), ((float) $2))'')\n') ;
    case { 'GxB_FC64_t' }
        fprintf (f, 'define(`GB_atype_cast'', `GxB_CMPLX (((double) $1), ((double) $2))'')\n') ;
    case { 'any type' }
        fprintf (f, 'define(`GB_atype_cast'', `'')\n') ;
    otherwise
        fprintf (f, 'define(`GB_atype_cast'', `((GB_atype) $1)'')\n') ;
end

% identity and terminal values for the monoid
fprintf (f, 'define(`GB_identity'', `%s'')\n', identity) ;

if (is_any_pair)
    fprintf (f, 'define(`GB_is_any_pair_semiring'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_any_pair_semiring'', `0'')\n') ;
end

if (is_any_pair)
    fprintf (f, 'define(`GB_is_plus_pair_real_semiring'', `0'')\n') ;
    fprintf (f, 'define(`GB_cij_declare'', `'')\n') ;
elseif (is_plus_pair_real)
    fprintf (f, 'define(`GB_is_plus_pair_real_semiring'', `1'')\n') ;
    fprintf (f, 'define(`GB_cij_declare'', `%s cij = 0'')\n', ztype) ;
else
    fprintf (f, 'define(`GB_is_plus_pair_real_semiring'', `0'')\n') ;
    fprintf (f, 'define(`GB_cij_declare'', `%s cij'')\n', ztype) ;
end

if (is_plus_times_fp)
    % plus_times_fp32 and plus_times_fp64 are accelerated with AVX2 or AVX512f instructions.
    % More semirings will be accelerated in the future.
    fprintf (f, 'define(`GB_semiring_has_avx_implementation'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_semiring_has_avx_implementation'', `0'')\n') ;
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
    fprintf (f, 'define(`GB_monoid_is_terminal'', `1'')\n') ;
elseif (~isempty (terminal))
    % terminal monoids terminate when cij equals the terminal value
    fprintf (f, 'define(`GB_is_any_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `if (cij == %s) { break ; }'')\n', ...
        terminal) ;
    fprintf (f, 'define(`GB_dot_simd_vectorize'', `;'')\n') ;
    fprintf (f, 'define(`GB_monoid_is_terminal'', `1'')\n') ;
else
    % non-terminal monoids
    fprintf (f, 'define(`GB_is_any_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_terminal'', `;'')\n') ;
    fprintf (f, 'define(`GB_monoid_is_terminal'', `0'')\n') ;
    op = '' ;
    if (ztype_is_real)
        switch (addop)
            case { 'plus' }
                op = '+' ;
            case { 'times' }
                op = '*' ;
            case { 'lor' }
                op = '||' ;
            case { 'land' }
                op = '&&' ;
            case { 'lxor' }
                op = '^' ;
            case { 'bor' }
                op = '|' ;
            case { 'band' }
                op = '&' ;
            case { 'bxor' }
                op = '^' ;
            otherwise
                op = '' ;
        end
    end
    if (isempty (op))
        fprintf (f, 'define(`GB_dot_simd_vectorize'', `;'')\n') ;
    else
        pragma = sprintf ('GB_PRAGMA_SIMD_REDUCTION (%s,$1)', op) ;
        fprintf (f, 'define(`GB_dot_simd_vectorize'', `%s'')\n', pragma) ;
    end
end

if (ztype_is_real)
    % The ANY monoid is atomic on any architecture.
    % MIN, MAX, EQ, XNOR are implemented with atomic compare/exchange.
    fprintf (f, 'define(`GB_has_atomic'', `1'')\n') ;
    if (is_any)
        % disable the ANY monoid for saxpy4
        fprintf (f, 'define(`_Asaxpy4B'', `_Asaxpy4B__%s'')\n', '(none)') ;
        fprintf (f, 'define(`if_saxpy4_enabled'', `#if 0'')\n') ;
    else
        fprintf (f, 'define(`_Asaxpy4B'', `_Asaxpy4B__%s'')\n', name) ;
        fprintf (f, 'define(`if_saxpy4_enabled'', `#if 1'')\n') ;
    end
else
    % complex monoids are not atomic, except for 'plus'
    if (isequal (addop, 'plus'))
        fprintf (f, 'define(`GB_has_atomic'', `1'')\n') ;
        fprintf (f, 'define(`_Asaxpy4B'', `_Asaxpy4B__%s'')\n', name) ;
        fprintf (f, 'define(`if_saxpy4_enabled'', `#if 1'')\n') ;
    else
        fprintf (f, 'define(`GB_has_atomic'', `0'')\n') ;
        fprintf (f, 'define(`_Asaxpy4B'', `_Asaxpy4B__%s'')\n', '(none)') ;
        fprintf (f, 'define(`if_saxpy4_enabled'', `#if 0'')\n') ;
    end
end

if (is_any)
    % dot4 is disabled for the ANY monoid
    fprintf (f, 'define(`_Adot4B'', `_Adot4B__%s'')\n', '(none)') ;
    fprintf (f, 'define(`if_dot4_enabled'', `#if 0'')\n') ;
else
    fprintf (f, 'define(`_Adot4B'', `_Adot4B__%s'')\n', name) ;
    fprintf (f, 'define(`if_dot4_enabled'', `#if 1'')\n') ;
end

if (is_any)
    % saxpy5 is disabled for the ANY monoid
    fprintf (f, 'define(`_Asaxpy5B'', `_Asaxpy5B__%s'')\n', '(none)') ;
    fprintf (f, 'define(`if_saxpy5_enabled'', `#if 0'')\n') ;
else
    fprintf (f, 'define(`_Asaxpy5B'', `_Asaxpy5B__%s'')\n', name) ;
    fprintf (f, 'define(`if_saxpy5_enabled'', `#if 1'')\n') ;
end

% firsti or firsti1 multiply operator
if (codegen_contains (multop, 'firsti'))
    fprintf (f, 'define(`GB_is_firsti_multiplier'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_firsti_multiplier'', `0'')\n') ;
end

% firstj or firstj1 multiply operator
if (codegen_contains (multop, 'firstj'))
    fprintf (f, 'define(`GB_is_firstj_multiplier'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_firstj_multiplier'', `0'')\n') ;
end

% secondj or secondj1 multiply operator
if (codegen_contains (multop, 'secondj'))
    fprintf (f, 'define(`GB_is_secondj_multiplier'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_secondj_multiplier'', `0'')\n') ;
end

% offset for (first,second)*i1 or (first,second)*j1 multiply operator
if (codegen_contains (multop, 'i1') || codegen_contains (multop, 'j1'))
    fprintf (f, 'define(`GB_offset'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_offset'', `0'')\n') ;
end

% plus_fc32 monoid:
if (isequal (addop, 'plus') && isequal (ztype, 'GxB_FC32_t'))
    fprintf (f, 'define(`GB_is_plus_fc32_monoid'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_plus_fc32_monoid'', `0'')\n') ;
end

% plus_fc64 monoid:
if (isequal (addop, 'plus') && isequal (ztype, 'GxB_FC64_t'))
    fprintf (f, 'define(`GB_is_plus_fc64_monoid'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_plus_fc64_monoid'', `0'')\n') ;
end

% any_fc32 monoid:
if (isequal (addop, 'any') && isequal (ztype, 'GxB_FC32_t'))
    fprintf (f, 'define(`GB_is_any_fc32_monoid'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_any_fc32_monoid'', `0'')\n') ;
end

% any_fc64 monoid:
if (isequal (addop, 'any') && isequal (ztype, 'GxB_FC64_t'))
    fprintf (f, 'define(`GB_is_any_fc64_monoid'', `1'')\n') ;
else
    fprintf (f, 'define(`GB_is_any_fc64_monoid'', `0'')\n') ;
end

% min monoids:
if (is_min)
    if (codegen_contains (ztype, 'int'))
        % min monoid for signed or unsigned integers
        fprintf (f, 'define(`GB_is_imin_monoid'', `1'')\n') ;
        fprintf (f, 'define(`GB_is_fmin_monoid'', `0'')\n') ;
    else
        % min monoid for float or double
        fprintf (f, 'define(`GB_is_imin_monoid'', `0'')\n') ;
        fprintf (f, 'define(`GB_is_fmin_monoid'', `1'')\n') ;
    end
else
    % not a min monoid
    fprintf (f, 'define(`GB_is_imin_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_is_fmin_monoid'', `0'')\n') ;
end

% max monoids:
if (is_max)
    if (codegen_contains (ztype, 'int'))
        % max monoid for signed or unsigned integers
        fprintf (f, 'define(`GB_is_imax_monoid'', `1'')\n') ;
        fprintf (f, 'define(`GB_is_fmax_monoid'', `0'')\n') ;
    else
        % max monoid for float or double
        fprintf (f, 'define(`GB_is_imax_monoid'', `0'')\n') ;
        fprintf (f, 'define(`GB_is_fmax_monoid'', `1'')\n') ;
    end
else
    % not a max monoid
    fprintf (f, 'define(`GB_is_imax_monoid'', `0'')\n') ;
    fprintf (f, 'define(`GB_is_fmax_monoid'', `0'')\n') ;
end

% only PLUS, TIMES, LOR, LAND, and LXOR can be done with OpenMP atomics
% in gcc and icc.  However, only PLUS and TIMES work with OpenMP atomics
% in Microsoft Visual Studio; the LOR, LAND, and LXOR atomics don't compile.
fprintf (f, 'define(`GB_has_omp_atomic'', `%d'')\n', omp_atomic) ;
fprintf (f, 'define(`GB_microsoft_has_omp_atomic'', `%d'')\n', omp_microsoft_atomic) ;

% to get an entry from A
if (is_any_pair)
    fprintf (f, 'define(`GB_a_is_pattern'', `1'')\n') ;
    fprintf (f, 'define(`GB_geta'', `;'')\n') ;
    fprintf (f, 'define(`GB_loada'', `;'')\n') ;
elseif (is_second || is_pair || is_positional)
    % value of A is ignored for the SECOND and PAIR operators
    fprintf (f, 'define(`GB_a_is_pattern'', `1'')\n') ;
    fprintf (f, 'define(`GB_geta'', `;'')\n') ;
    fprintf (f, 'define(`GB_loada'', `$1 [$2] = GBX ($3, $4, $5)'')\n') ;
else
    fprintf (f, 'define(`GB_a_is_pattern'', `0'')\n') ;
    fprintf (f, 'define(`GB_geta'', `%s $1 = GBX ($2, $3, $4)'')\n', xytype) ;
    fprintf (f, 'define(`GB_loada'', `$1 [$2] = GBX ($3, $4, $5)'')\n') ;
end

% to get an entry from B
if (is_first || is_pair || is_positional)
    % value of B is ignored for the FIRST and PAIR operators
    fprintf (f, 'define(`GB_b_is_pattern'', `1'')\n') ;
    fprintf (f, 'define(`GB_getb'', `;'')\n') ;
    fprintf (f, 'define(`GB_loadb'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_b_is_pattern'', `0'')\n') ;
    fprintf (f, 'define(`GB_getb'', `%s $1 = GBX ($2, $3, $4)'')\n', xytype) ;
    fprintf (f, 'define(`GB_loadb'', `$1 [$2] = GBX ($3, $4, $5)'')\n') ;
end

% access the values of C
if (is_any_pair)
    fprintf (f, 'define(`GB_cx'', `'')\n') ;
    fprintf (f, 'define(`GB_putc'', `'')\n') ;
    fprintf (f, 'define(`GB_cij_write'', `'')\n') ;
else
    fprintf (f, 'define(`GB_cx'', `Cx [p]'')\n') ;
    fprintf (f, 'define(`GB_putc'', `Cx [p] = cij'')\n') ;
    fprintf (f, 'define(`GB_cij_write'', `Cx [p] = t'')\n') ;
end

% type-specific idiv
if (~isempty (strfind (mult, 'idiv')))
    if (unsigned)
        mult = strrep (mult, 'idiv', sprintf ('idiv_uint%d', bits)) ;
    else
        mult = strrep (mult, 'idiv', sprintf ('idiv_int%d', bits)) ;
    end
end

% create the multiply operator (assignment)
if (is_any_pair)
    fprintf (f, 'define(`GB_multiply'', `'')\n') ;
else
    mult2 = strrep (mult,  'xarg', '`$2''') ;
    mult2 = strrep (mult2, 'yarg', '`$3''') ;
    fprintf (f, 'define(`GB_multiply'', `$1 = %s'')\n', mult2) ;
end

% create the add update, of the form w += t
if (is_any_pair)
    add2 = ';' ;
elseif (is_min)
    if (codegen_contains (ztype, 'int'))
        % min monoid for signed or unsigned integers
        add2 = 'if ($1 > $2) { $1 = $2 ; }' ;
    else
        % min monoid for float or double, with omitnan property
        if (t_is_nonnan)
            add2 = 'if (!islessequal ($1, $2)) { $1 = $2 ; }' ;
        else
            add2 = 'if (!isnan ($2) && !islessequal ($1, $2)) { $1 = $2 ; }' ;
        end
    end
elseif (is_max)
    if (codegen_contains (ztype, 'int'))
        % max monoid for signed or unsigned integers
        add2 = 'if ($1 < $2) { $1 = $2 ; }' ;
    else
        % max monoid for float or double, with omitnan property
        if (t_is_nonnan)
            add2 = 'if (!isgreaterequal ($1, $2)) { $1 = $2 ; }' ;
        else
            add2 = 'if (!isnan ($2) && !isgreaterequal ($1, $2)) { $1 = $2 ; }';
        end
    end
else
    % use the add function as given
    add2 = strrep (add,  'w', '`$1''') ;
    add2 = strrep (add2, 't', '`$2''') ;
end
fprintf (f, 'define(`GB_add_update'', `%s'')\n', add2) ;

if (is_any_pair)
    fprintf (f, 'define(`GB_hx_write'', `;'')\n') ;
    fprintf (f, 'define(`GB_cij_gather'', `;'')\n') ;
    fprintf (f, 'define(`GB_cij_memcpy'', `;'')\n') ;
else
    fprintf (f, 'define(`GB_hx_write'', `Hx [i] = t'')\n') ;
    fprintf (f, 'define(`GB_cij_gather'', `Cx [p] = Hx [i]'')\n') ;
    fprintf (f, 'define(`GB_cij_memcpy'', `memcpy (Cx +(p), Hx +(i), (len) * sizeof(%s));'')\n', ztype) ;
        
end

% create the add function, of the form w + t
if (is_any_pair)
    fprintf (f, 'define(`GB_add_function'', `'')\n') ;
else
    add2 = strrep (addfunc,  'w', '`$1''') ;
    add2 = strrep (add2,     't', '`$2''') ;
    fprintf (f, 'define(`GB_add_function'', `%s'')\n', add2) ;
end

% create the multiply-add statement, of the form:
%   z += x*y ;
is_imin_or_imax = (isequal (addop, 'min') || isequal (addop, 'max')) && codegen_contains (ztype, 'int') ;
if (is_any_pair)
    fprintf (f, 'define(`GB_multiply_add'', `'')\n') ;
elseif (~is_imin_or_imax && ...
    (isequal (ztype, 'float') || isequal (ztype, 'double') || ...
     isequal (ztype, 'bool') || is_first || is_second || is_pair || is_positional))
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
    fprintf (f, 'define(`GB_multiply_add'', `{ %s x_op_y = %s ; %s ; }'')\n', ...
        ztype, mult2, add2) ;
end

% determine the identity byte
idbyte = '' ;

switch (addop)

    % any monoid
    case { 'any' }

    % boolean monoids (except eq / lxnor)
    case { 'lor' }
        idbyte = '0' ;
    case { 'land' }
        idbyte = '1' ;
    case { 'lxor' }
        idbyte = '0' ;

    % min/max monoids:
    case { 'min' }
        if (codegen_contains (ztype, 'uint'))
            idbyte = '0xFF' ;
        end
    case { 'max' }
        if (codegen_contains (ztype, 'uint'))
            idbyte = '0' ;
        end

    % plus monoid: special cases for some multipliers
    case { 'plus' }
        idbyte = '0' ;

    % bitwise monoids (except bxnor)
    case { 'bor' }
        idbyte = '0' ;
    case { 'band' }
        idbyte = '0xFF' ;
    case { 'bxor' }
        idbyte = '0' ;

    case { 'eq' }
        idbyte = '1' ;
    case { 'times' }
        idbyte = '' ;
    case {'bxnor' }
        idbyte = '0xFF' ;
end

if (isempty (idbyte))
    fprintf (f, 'define(`GB_has_identity_byte'', `0'')\n') ;
    fprintf (f, 'define(`GB_identity_byte'', `(none)'')\n') ;
else
    fprintf (f, 'define(`GB_has_identity_byte'', `1'')\n') ;
    fprintf (f, 'define(`GB_identity_byte'', `%s'')\n', idbyte) ;
end

% create the disable flag
if (is_any_pair)
    % never disable the any_pair_iso semiring
    fprintf (f, 'define(`GB_disable'', `0'')\n') ;
    fprintf (f, 'define(`if_disabled'', `#if 0'')\n') ;
    fprintf (f, 'define(`if_not_disabled'', `#if 1'')\n') ;
else
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
    fprintf (f, 'define(`if_disabled'', `#if GB_DISABLE'')\n') ;
    fprintf (f, 'define(`if_not_disabled'', `#if ( !GB_DISABLE )'')\n') ;
end

fclose (f) ;

nprune = 76 ;

if (is_any_pair)
    % the ANY_PAIR_ISO semiring goes in Generated1
    k = 1 ;
else
    % all other semirings go in Generated2
    k = 2 ;
end

% construct the *.c file for the semiring
cmd = sprintf ('cat control.m4 Generator/GB_AxB.c | m4 | tail -n +%d > Generated%d/GB_AxB__%s.c', nprune, k, name) ;
system (cmd) ;

fprintf ('.') ;

% append to the *.h file
cmd = sprintf ('cat control.m4 Generator/GB_AxB.h | m4 | tail -n +%d >> Generated%d/GB_AxB__include%d.h', nprune, k, k) ;
system (cmd) ;

delete ('control.m4') ;


function C = gb_bitwise (op, A, B, assumedtype)
%GB_BITWISE bitwise AND, OR, XOR, ...

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (isobject (A))
    A = A.opaque ;
end

if (isobject (B))
    B = B.opaque ;
end

atype = gbtype (A) ;
btype = gbtype (B) ;

if (contains (atype, 'complex') || contains (btype, 'complex'))
    error ('inputs must be real') ;
end

if (isequal (atype, 'logical') || isequal (btype, 'logical'))
    error ('inputs must not be logical') ;
end

if (~contains (assumedtype, 'int'))
    error ('assumedtype must be an integer type') ;
end

% C will have the same type as A on input
ctype = atype ;

if (isequal (atype, 'double') || isequal (atype, 'single'))
    A = gbnew (A, assumedtype) ;
    atype = assumedtype ;
end

if (isequal (op, 'bitshift'))

    if (~isequal (btype, 'int8'))
        % convert B to int8, and ensure all values are in range -64:64
        % ensure all entries in B are <= 64
        B = gbapply2 (['min.' btype], B, 64) ;
        if (gb_issigned (btype))
            % ensure all entries in B are >= -64
            B = gbapply2 (['max.' btype], B, -64) ;
        end
        B = gbnew (B, 'int8') ;
    end

    if (gb_isscalar (A) || gb_isscalar (B))
        % either A or B are scalars
        C = gbapply2 (['bitshift.' atype], A, B) ;
    else
        % both A and B are matrices.
        % expand B by padding it with zeros from the pattern of A
        B = gbeadd ('1st.int8', B, gb_expand (0, A, 'int8')) ;
        C = gbemult (['bitshift.' atype], A, B) ;
    end

else

    if (isequal (btype, 'double') || isequal (btype, 'single'))
        B = gbnew (B, assumedtype) ;
        btype = assumedtype ;
    end
    if (~isequal (atype, btype))
        error ('integer inputs must have the same type') ;
    end

    switch (op)
        case { 'bitxor', 'bitor' }
            C = gb_eadd (A, op, B) ;
        case { 'bitand' }
            C = gb_emult (A, op, B) ;
    end
end

if (~isequal (gbtype (C), ctype))
    C = gbnew (C, ctype) ;
end


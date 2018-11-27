function C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, ...
    Mask_complement, identity)
%
%GB_SPEC_ACCUM_MASK apply the accumulator and mask
%
% C<Mask> = accum (C,T): apply the accum, then mask, and return the result

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% C_orig = C ;

[Z simple] = GB_spec_accum (accum, C, T, identity) ;
C = GB_spec_mask (C, Mask, Z, C_replace, Mask_complement, identity) ;

% also test the simpler version (test disabled for now, to speed up testing)
%{
if (simple)
    % create a function handle for accum: GB_spec_op(accum,x,y)
    if (isempty (accum))
        C2 = accum_mask (C_orig, Mask, [ ], T, C_replace, Mask_complement) ;
    else
        global accum_struct
        accum_struct = accum ;
        C2 = accum_mask (C_orig, Mask, @afunc, T, C_replace, Mask_complement) ;
    end
    assert (isequal (C, C2)) ;
end
end

function z = afunc (x,y)
global accum_struct
z = GB_spec_op (accum_struct, x, y) ;
end
%}



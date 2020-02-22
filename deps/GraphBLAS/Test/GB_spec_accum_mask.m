function C = GB_spec_accum_mask (C, Mask, accum, T, C_replace, ...
    Mask_complement, identity)
%GB_SPEC_ACCUM_MASK apply the accumulator and mask
%
% C<Mask> = accum (C,T): apply the accum, then mask, and return the result

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[Z simple] = GB_spec_accum (accum, C, T, identity) ;
C = GB_spec_mask (C, Mask, Z, C_replace, Mask_complement, identity) ;


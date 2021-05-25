function x = gb_scalar (A)
%GB_SCALAR get contents of a scalar
% x = gb_scalar (A).  A may be a MATLAB scalar or a GraphBLAS
% scalar as a struct (not an object).  Returns the result x
% as a MATLAB non-sparse scalar.  If the scalar has no entry
% (the MATLAB sparse(0)), then x is returned as zero.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[~, ~, x] = gbextracttuples (A) ;
if (isempty (x))
    x = 0 ;
else
    x = x (1) ;
end


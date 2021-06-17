function s = gb_numel (G)
%GB_NUMEL the maximum number of entries a GraphBLAS matrix can hold.
% Implements s = numel (G)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

[m, n] = gbsize (G) ;
s = m*n ;

if (m > flintmax || n > flintmax || s > flintmax)
    s = vpa (vpa (m, 64) * vpa (n, 64), 128) ;
end


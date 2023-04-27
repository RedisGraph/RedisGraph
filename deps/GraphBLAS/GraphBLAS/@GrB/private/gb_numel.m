function s = gb_numel (G)
%GB_NUMEL the maximum number of entries a GraphBLAS matrix can hold.
% Implements s = numel (G)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

[m, n] = gbsize (G) ;
s = m*n ;

try
    if (m > flintmax || n > flintmax || s > flintmax)
        % use the VPA if available, for really huge matrices
        s = vpa (vpa (m, 64) * vpa (n, 64), 128) ;
    end
catch
end


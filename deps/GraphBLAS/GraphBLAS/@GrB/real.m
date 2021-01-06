function C = real (G)
%REAL complex real part.
% C = real (G) returns the real part of G.
%
% See also GrB/conj, GrB/imag.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

Q = G.opaque ;

if (contains (gbtype (Q), 'complex'))
    C = GrB (gbapply ('creal', Q)) ;
else
    % G is already real
    C = G ;
end


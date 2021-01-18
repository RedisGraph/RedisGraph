function [x, y] = gb_get_pair (A)
%GB_GET_PAIR get a pair of scalars from a parameter of length 2

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (isobject (A))
    A = A.opaque ;
end

type = gbtype (A) ;
desc.kind = 'full' ;
C = gbfull (A, type, 0, desc) ;                 % export as a MATLAB full matrix
x = C (1) ;
y = C (2) ;


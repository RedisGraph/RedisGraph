function assert (G)
%ASSERT generate an error when a condition is violated
%
% See also error.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

builtin ('assert', logical (G)) ;


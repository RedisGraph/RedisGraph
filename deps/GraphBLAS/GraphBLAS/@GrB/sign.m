function C = sign (G)
%SIGN Signum function.
% C = sign (G) computes the signum function for each entry in the
% GraphBLAS matrix G.  For each element of G, sign(G) returns 1 if the
% element is greater than zero, 0 if it equals zero, and -1 if it is less
% than zero.  The output C is a GraphBLAS matrix.
%
% See also abs.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

C = spones (GrB.select (G, '>0')) - spones (GrB.select (G, '<0')) ;


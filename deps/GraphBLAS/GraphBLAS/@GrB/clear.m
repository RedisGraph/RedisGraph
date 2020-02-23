function clear
%GRB.CLEAR restore default settings for SuiteSparse:GraphBLAS.
%
% Usage:
%
%   GrB.clear
%
% GrB.clear clears any non-default setting of the GraphBLAS global
% variables, including GrB.threads, GrB.chunk, and GrB.format,
% and sets them to their defaults.  It has no effect on any GrB
% objects.
%
% See also: clear, GrB.init, GrB.finalize

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

gbsetup ;


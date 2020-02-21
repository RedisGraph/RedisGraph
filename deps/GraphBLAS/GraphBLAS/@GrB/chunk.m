function c = chunk (varargin)
%GRB.CHUNK get/set the chunk size to use in GraphBLAS.
%
% Usage:
%   c = GrB.chunk ;      % get the current chunk c
%   GrB.chunk (c) ;      % set the chunk c
%
% GrB.chunk gets and/or sets the chunk size to use in GraphBLAS, which controls
% how many threads GraphBLAS uses for small problems.  The default is 4096.  If
% w is a measure of the work required (w = GrB.entries(A) + GrB.entries(B) for
% C=A+B, for example), then the number of threads GraphBLAS uses is min (max
% (1, floor (w/c)), GrB.nthreads).
%
% Changing the chunk via GrB.chunk(c) causes all subsequent GraphBLAS operations
% to use that chunk size c.  The setting persists for the current MATLAB
% session, or until 'clear all' or GrB.clear is used, at which point the setting
% reverts to the default.
%
% See also GrB.threads.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

c = gbchunk (varargin {:}) ;


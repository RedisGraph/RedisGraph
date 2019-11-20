function clear
%GRB.CLEAR free all internal workspace in SuiteSparse:GraphBLAS.
%
% Usage:
%
%   GrB.clear
%
% GraphBLAS keeps an internal workspace to speedup its operations.  It
% also uses several global settings.  These can both be cleared with
% GrB.clear.
%
% This method is optional.  Simply terminating the MATLAB session, or
% typing 'clear all' will do the same thing.  However, if you are
% finished with GraphBLAS and wish to free its internal workspace, but do
% not wish to free everything else freed by 'clear all', then use this
% method.  GrB.clear also clears any non-default setting of GrB.threads,
% GrB.chunk, and GrB.format.
%
% See also: clear, GrB.threads, GrB.chunk, GrB.format

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

gbclear ;


function b = burble (varargin)
%GRB.BURBLE get/set the burble
%
% Usage:
%   b = GrB.burble ;      % get the current burble
%   GrB.burble (b) ;      % set the burble
%
% GrB.burble gets and/or sets the burble setting, which controls diagnostic
% output in GraphBLAS.  To enable this parameter, the SuiteSparse:GraphBLAS
% library must also be compiled with burble enabled (use -DGB_BURBLE=1).  The
% default is false.  This setting is meant for diagnostic use only, during
% development of GraphBLAS itself.  It may also be useful for algorithmic
% development, and is thus documented here.  See also spparms ('spumoni', ...).
%
% See also spparms.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

b = gbburble (varargin {:}) ;


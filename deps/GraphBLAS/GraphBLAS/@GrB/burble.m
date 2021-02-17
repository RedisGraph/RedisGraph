function b = burble (b)
%GRB.BURBLE get/set the GraphBLAS burble option.
%
%   b = GrB.burble ;      % get the current burble
%   GrB.burble (b) ;      % set the burble
%
% GrB.burble gets and/or sets the burble setting, which controls diagnostic
% output in GraphBLAS.  To enable this parameter, the SuiteSparse:GraphBLAS
% library must also be compiled with burble enabled (use -DGB_BURBLE=1).
% The default burble is false.
%
% See also spparms.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

if (nargin == 0)
    b = gbburble ;
else
    b = gbburble (b) ;
end


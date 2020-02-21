function [V, varargout] = eig (G, varargin)
%EIG Eigenvalues and eigenvectors of a GraphBLAS matrix.
% See 'help eig' for details.
%
% See also eigs.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (isreal (G) && issymmetric (G))
    % G can be sparse if G is real and symmetric
    G = double (G) ;
else
    % otherwise, G must be full.
    G = full (double (G)) ;
end
if (nargin == 1)
    [V, varargout{1:nargout-1}] = builtin ('eig', G) ;
else
    args = varargin ;
    for k = 1:length (args)
        if (isa (args {k}, 'GrB'))
            args {k} = full (double (args {k})) ;
        end
    end
    [V, varargout{1:nargout-1}] = builtin ('eig', G, args {:}) ;
end


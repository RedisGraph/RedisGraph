function disp (G, level)
%DISP display the contents of a GraphBLAS matrix.
% disp (G, level) displays the GraphBLAS sparse matrix G.  Level controls
% how much is printed; 0: none, 1: terse, 2: a few entries, 3: all.  The
% default is 2 if level is not present.
%
% See also display.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 2)
    level = 2 ;
end
if (level > 0)
    name = inputname (1) ;
    if (~isempty (name))
        fprintf ('\n%s =\n', name) ;
    end
end

gbdisp (G.opaque, nnz (G), level) ;


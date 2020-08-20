function display (G) %#ok<DISPLAY>
%DISPLAY display the contents of a GraphBLAS matrix.
% display (G) displays the attributes and first few entries of a
% GraphBLAS sparse matrix object.  Use disp(G,3) to display all of the
% content of G.
%
% See also disp.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

name = inputname (1) ;
if (~isempty (name))
    fprintf ('\n%s =\n', name) ;
end
gbdisp (G.opaque, nnz (G), 2) ;


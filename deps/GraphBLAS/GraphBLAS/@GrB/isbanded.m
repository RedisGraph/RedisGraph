function s = isbanded (G, lo, hi)
%ISBANDED True if G is a banded GraphBLAS matrix.
% isbanded (G, lo, hi) is true if the bandwidth of the GraphBLAS matrix G
% is between lo and hi.

% FUTURE: this will be much faster when 'bandwidth' is a mexFunction.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[Glo, Ghi] = bandwidth (G) ;
s = (Glo <= lo) & (Ghi <= hi) ;


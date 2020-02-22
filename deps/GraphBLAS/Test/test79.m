function test79
%TEST79 run all matrices with test06

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

index = ssget ;
f = find (index.nrows == index.ncols & index.isReal & index.nrows > 1000) ;

[ignore i] = sort (index.nnz (f)) ;

f = f (i) ;
nmat = length (f) ;

for k = 1:nmat
    id = f (k) ;
    Prob = ssget (id, index)
    A = Prob.A ;
    test06 (A, A, 0) ;
end

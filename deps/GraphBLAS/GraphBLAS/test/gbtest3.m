function gbtest3
%GBTEST3 test dnn

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('gbtest3: testing sparse deep neural network\n') ;
help GrB.dnn

rng ('default') ;
levels = 4 ;
nfeatures = 6 ;
nneurons = 16 ;

for level = 1:levels
    W {level} = sprand (nneurons, nneurons, 0.5) ; %#ok<*AGROW>
    bias {level} = -0.3 * ones (1, nneurons) ;
end

Y0 = sprandn (nfeatures, nneurons, 0.5) ;

Y1 = dnn_matlab (W, bias, Y0) ;

[W, bias, Y0] = dnn_mat2gb (W, bias, Y0) ;
Y2 = GrB.dnn (W, bias, Y0) ;

err = norm (Y1-Y2,1) ;
assert (err < 1e-5) ;

fprintf ('gbtest3: all tests passed\n') ;


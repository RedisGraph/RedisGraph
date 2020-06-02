function Y = dnn_gb (W, bias, Y0)
%DNN_GB Sparse deep neural network in GraphBLAS
% Performs ReLU inference using input feature vector(s) Y0, DNN weights W,
% and bias vectors.
%
% Compare with dnn_matlab.m.
%
% Usage:
%
%   Y = dnn_gb (W, bias, Y0)
%
% The matrices can be stored by row or by column, but GrB.format ('by row')
% is significantly faster.
%
% See also dnn_matlab, dnn_mat2gb.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

Y = Y0 ;
for i=1:length(W)

    % Propagate through layer, apply bias, and threshold negative values.
    Y = GrB.select ('>0', GrB.mxm ('+.+', Y * W {i}, bias {i})) ;
     
    M = Y > 32 ;
    if (nnz (M) > 0)
        Y (M) = 32 ;
    end

end


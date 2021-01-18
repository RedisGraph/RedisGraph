function test186
%TEST186 test saxpy for all sparsity formats

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

fprintf ('test186 --------------- C<!M>A*B for all sparsity formats\n') ;

rng ('default') ;

GrB.burble (1) ;

load west0479 ;
A.matrix = west0479 ;
A.class = 'double' ;
A.pattern = logical (spones (A.matrix)) ;
m = size (A.matrix, 1) ;

semiring.add = 'plus' ;
semiring.multiply = 'times' ;
semiring.class = 'double' ;

any_pair.add = 'any' ;
any_pair.multiply = 'pair' ;
any_pair.class = 'double' ;

C0 = sparse (m, 1) ;
maxerr = 0 ;

M = sparse (rand (m, 1) > 0.5) ;
desc.mask = 'complement' ;

B = GB_spec_random (m, 1, 0.5, 1, 'double') ;

% using fine atomic tasks when A is sparse and B is bitmap
for A_sparsity = [1 2 4 8]
    for B_sparsity = [1 2 4 8]
        A.sparsity = A_sparsity ;
        B.sparsity = B_sparsity ;

        % C2<!M> = A*B using the conventional semiring
        C3 = double (~M) .* (A.matrix * B.matrix) ;
        C2 = GB_mex_mxm  (C0, M, [ ], semiring, A, B, desc) ;
        err = norm (C3 - C2.matrix, 1) / norm (C3, 1) ;
        maxerr = max (maxerr, err) ;
        assert (err < 1e-12) ;

        % C2<!M> = A*B using the any-pair semiring
        C3 = spones (C3) ;
        C2 = GB_mex_mxm  (C0, M, [ ], any_pair, A, B, desc) ;
        err = norm (C3 - C2.matrix, 1) / norm (C3, 1) ;
        maxerr = max (maxerr, err) ;
        assert (err < 1e-12) ;
    end
end

% using fine non-atomic tasks when A is sparse and B is bitmap
A.matrix = sprand (m, m, 0.8) ;
A.pattern = logical (spones (A.matrix)) ;
for A_sparsity = [1 2 4 8]
    for B_sparsity = [1 2 4 8]
        A.sparsity = A_sparsity ;
        B.sparsity = B_sparsity ;
        fprintf ('.') ;

        % C2<!M> = A*B using the conventional semiring
        C3 = double (~M) .* (A.matrix * B.matrix) ;
        C2 = GB_mex_mxm  (C0, M, [ ], semiring, A, B, desc) ;
        err = norm (C3 - C2.matrix, 1) / norm (C3, 1) ;
        maxerr = max (maxerr, err) ;
        assert (err < 1e-12) ;

        % C2<!M> = A*B using the any-pair semiring
        C3 = spones (C3) ;
        C2 = GB_mex_mxm  (C0, M, [ ], any_pair, A, B, desc) ;
        err = norm (C3 - C2.matrix, 1) / norm (C3, 1) ;
        maxerr = max (maxerr, err) ;
        assert (err < 1e-12) ;
    end
end

fprintf ('\n') ;
GrB.burble (0) ;
fprintf ('maxerr: %g\n', maxerr) ;
fprintf ('test186: all tests passed\n') ;


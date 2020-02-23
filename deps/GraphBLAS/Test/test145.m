function test145
%TEST145 test dot4
% GB_AxB_dot4 computes C+=A'*B when C is dense.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test145 -------------------- C+=A''*B when C is dense, with dot4\n') ;

rng ('default') ;

A = sparse (rand (4)) ;
B = sparse (rand (4)) ;
C = sparse (rand (4)) ;
AT = A' ;
BT = B' ;

semiring.add = 'plus' ;
semiring.multiply = 'div' ;
semiring.class = 'double' ;
[mult_op add_op id] = GB_spec_semiring (semiring) ;

dnn = struct ('axb', 'dot') ;
dtn = struct ('axb', 'dot', 'in0', 'tran') ;
dnt = struct ('axb', 'dot', 'in1', 'tran') ;
dtt = struct ('axb', 'dot', 'in0', 'tran', 'in1', 'tran') ;

% C = GB_mex_mxm (C, Mask, accum, semiring, A, B, desc)

C2 = GB_mex_mxm  (C, [ ], add_op, semiring, A, B, dnn) ;
C1 = GB_spec_mxm (C, [ ], add_op, semiring, A, B, dnn) ;
GB_spec_compare (C1, C2) ;

C2 = GB_mex_mxm  (C, [ ], add_op, semiring, AT, B, dtn) ;
C1 = GB_spec_mxm (C, [ ], add_op, semiring, AT, B, dtn) ;
GB_spec_compare (C1, C2) ;

C2 = GB_mex_mxm  (C, [ ], add_op, semiring, A, BT, dnt) ;
C1 = GB_spec_mxm (C, [ ], add_op, semiring, A, BT, dnt) ;
GB_spec_compare (C1, C2) ;

C2 = GB_mex_mxm  (C, [ ], add_op, semiring, AT, BT, dtt) ;
C1 = GB_spec_mxm (C, [ ], add_op, semiring, AT, BT, dtt) ;
GB_spec_compare (C1, C2) ;

X = 1./A ;
C1 = X*B ;

C2 = GB_mex_rdiv  (A, B,   1003) ;
assert (norm (C1-C2,1) < 1e-5)

C2 = GB_mex_rdiv2 (A, B,   false, false, 1003, 0) ;
assert (norm (C1-C2,1) < 1e-5)

C2 = GB_mex_rdiv2 (AT, B,  true,  false, 1003, 0) ;
assert (norm (C1-C2,1) < 1e-5)

C2 = GB_mex_rdiv2 (A, BT,  false, true,  1003, 0) ;
assert (norm (C1-C2,1) < 1e-5)

C2 = GB_mex_rdiv2 (AT, BT, true,  true,  1003, 0) ;
assert (norm (C1-C2,1) < 1e-5)

% update C in place with dot4:
X = 1./B ;
C1 = A*X + pi ;

C2 = GB_mex_rdiv2 (A, B,   false, false, 1003, 1, pi) ;
assert (norm (C1-C2,1) < 1e-5)

C2 = GB_mex_rdiv2 (AT, B,  true,  false, 1003, 1, pi) ;
assert (norm (C1-C2,1) < 1e-5)

C2 = GB_mex_rdiv2 (A, BT,  false, true,  1003, 1, pi) ;
assert (norm (C1-C2,1) < 1e-5)

fprintf ('test145: all tests passed\n') ;

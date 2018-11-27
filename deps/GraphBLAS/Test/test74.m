function test74
%TEST74 test GrB_mxm: dot product method
% built-in semirings, no typecast, no mask

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 1)
    fulltest = 1 ;
end

if (fulltest)
    fprintf ('--------------- GrB_mxm on all semirings (dot product method)\n') ;
    n_semirings_max = inf ;
else
    fprintf ('quick test of GrB_mxm (dot product method)\n') ;
    n_semirings_max = 1 ;
end

[mult_ops unary_ops add_ops classes] = GB_spec_opsall ;

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

ntrials = 0 ;

rng ('default') ;

m_list = [ 1  2  ]; % 10] ;
n_list = [ 1  2  ]; % 10] ;
k_list = [ 20 100]; % 50] ;

for k0 = 1:size(m_list,2)

    m = m_list (k0) ;
    n = n_list (k0) ;
    k = k_list (k0) ;

    n_semirings = 0 ;
    A = GB_spec_random (k,m,0.3,100,'none') ;
    B = GB_spec_random (k,m,0.3,100,'none') ;
    C = GB_spec_random (m,n,0.3,100,'none') ;
    M = spones (sprandn (m, n, 0.3)) ;

    fprintf ('\n\nC<M>=A''*B, C: %d-by-%d nz %d, A: %d-by-%d nz %d, B: %d-by-%d nz %d, M: nz %d\n', ...
        m, n, nnz (C.pattern), k, m, nnz (A.pattern), k, n, nnz (B.pattern), nnz (M)) ;

    for k1 = 1:length(mult_ops)
        mulop = mult_ops {k1} ;

        fprintf ('%s', mulop) ;

        for k2 = 1:length(add_ops)
            addop = add_ops {k2} ;
            fprintf ('.') ;

            for k3 = 1:length (classes)
                clas = classes {k3} ;

                semiring.multiply = mulop ;
                semiring.add = addop ;
                semiring.class = clas ;

                % create the semiring.  some are not valid because the or,and,xor
                % monoids can only be used when z is boolean for z=mult(x,y).
                try
                    [mult_op add_op id] = GB_spec_semiring (semiring) ;
                    [mult_opname mult_opclass zclass] = GB_spec_operator (mult_op);
                    [ add_opname  add_opclass] = GB_spec_operator (add_op) ;
                    identity = GB_spec_identity (semiring.add, add_opclass) ;
                catch
                    continue
                end

                if (n_semirings+1 > n_semirings_max)
                    fprintf ('\ntest74: all quick tests passed\n') ;
                    return ;
                end

                n_semirings = n_semirings + 1 ;
                A.class = clas ;
                B.class = clas ;
                C.class = clas ;

                % C<M> = A'*B, with Mask, no typecasting
                C1 = GB_mex_mxm  (C, M, [ ], semiring, A, B, dtn);
                C0 = GB_spec_mxm (C, M, [ ], semiring, A, B, dtn);
                GB_spec_compare (C0, C1, identity) ;
            end
        end
    end
end

fprintf ('\nsemirings tested: %d\n', n_semirings) ;
fprintf ('\ntest74: all tests passed\n') ;


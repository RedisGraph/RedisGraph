function test74
%TEST74 test GrB_mxm: all built-in semirings

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops, ~, add_ops, classes, ~, ~] = GB_spec_opsall ;

fprintf ('test74 -------- GrB_mxm on all semirings (all methods)\n') ;

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

dnn_Gus  = struct ( 'axb', 'gustavson' ) ;
dnn_hash = struct ( 'axb', 'hash' ) ;

ntrials = 0 ;

rng ('default') ;

m_list = [ 1  2    9  ] ;
n_list = [ 1  2   10  ] ;
k_list = [ 20 100 12  ] ;
d_list = [0.3 0.3 0.3 ] ;

for k0 = 1:size(m_list,2)

    m = m_list (k0) ;
    n = n_list (k0) ;
    k = k_list (k0) ;
    density = d_list (k0) ;

    n_semirings = 0 ;
    A = GB_spec_random (m,k,density,100,'none') ;
    B = GB_spec_random (k,n,density,100,'none') ;
    C = GB_spec_random (m,n,density,100,'none') ;
    M = spones (sprandn (m, n, 0.3)) ;

    clear AT
    AT = A ;
    AT.matrix  = A.matrix' ;
    AT.pattern = A.pattern' ;
    fprintf ('\nm %d n %d k %d: \n', m, n, k) ;

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

                % create the semiring.  some are not valid because the
                % or,and,xor monoids can only be used when z is boolean for
                % z=mult(x,y).
                try
                    [mult_op add_op id] = GB_spec_semiring (semiring) ;
                    [mult_opname mult_opclass zclass] = ...
                        GB_spec_operator (mult_op);
                    [ add_opname  add_opclass] = GB_spec_operator (add_op) ;
                    identity = GB_spec_identity (semiring.add, add_opclass) ;
                catch
                    continue
                end

                n_semirings = n_semirings + 1 ;
                AT.class = clas ;
                A.class = clas ;
                B.class = clas ;
                C.class = clas ;

                % C<M> = A'*B, with Mask, no typecasting
                C1 = GB_mex_mxm  (C, M, [ ], semiring, AT, B, dtn);
                C0 = GB_spec_mxm (C, M, [ ], semiring, AT, B, dtn);
                GB_spec_compare (C0, C1, identity) ;

                % C = A'*B, no Mask, no typecasting
                C1 = GB_mex_mxm  (C, [ ], [ ], semiring, AT, B, dtn);
                C0 = GB_spec_mxm (C, [ ], [ ], semiring, AT, B, dtn);
                GB_spec_compare (C0, C1, identity) ;

                % C = A*B, no Mask, no typecasting, Gustavson
                C1 = GB_mex_mxm  (C, [ ], [ ], semiring, A, B, dnn_Gus);
                % C0 = GB_spec_mxm (C, [ ], [ ], semiring, A, B, dnn_Gus);
                GB_spec_compare (C0, C1, identity) ;

                % C = A*B, no Mask, no typecasting, Hash
                C1 = GB_mex_mxm  (C, [ ], [ ], semiring, A, B, dnn_hash);
                % C0 = GB_spec_mxm (C, [ ], [ ], semiring, A, B, dnn_hash);
                GB_spec_compare (C0, C1, identity) ;

            end
        end
    end
end

fprintf ('\nsemirings tested: %d\n', n_semirings) ;
fprintf ('\ntest74: all tests passed\n') ;


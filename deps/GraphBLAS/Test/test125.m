function test125
%TEST125 test GrB_mxm: row and column scaling
% all built-in semirings, no typecast, no mask

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops, ~, add_ops, classes, ~, ~] = GB_spec_opsall ;

if (nargin < 1)
    fulltest = 1 ;
end

if (fulltest)
    fprintf ('-------------- GrB_mxm on all semirings (row,col scale)\n') ;
    n_semirings_max = inf ;
else
    fprintf ('quick test of GrB_mxm (dot product method)\n') ;
    n_semirings_max = 1 ;
end

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

ntrials = 0 ;

rng ('default') ;

    n = 10 ;

    n_semirings = 0 ;
    A = GB_spec_random (n,n,0.3,100,'none') ;
    clear B
    B.matrix = 100 * spdiags (rand (n,1), 0, n, n) ;
    B.class = 'none' ;
    B.pattern = logical (spones (B.matrix)) ;

    C = GB_spec_random (n,n,0.3,100,'none') ;
    M = spones (sprandn (n, n, 0.3)) ;

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
                    [mult_opname mult_opclass zclass] = GB_spec_operator (mult_op);
                    [ add_opname  add_opclass] = GB_spec_operator (add_op) ;
                    identity = GB_spec_identity (semiring.add, add_opclass) ;
                catch
                    continue
                end

                if (n_semirings+1 > n_semirings_max)
                    fprintf ('\ntest125: all quick tests passed\n') ;
                    return ;
                end

                n_semirings = n_semirings + 1 ;
                A.class = clas ;
                B.class = clas ;
                C.class = clas ;

                % C = A*B
                C1 = GB_mex_mxm  (C, [ ], [ ], semiring, A, B, dnn);
                C0 = GB_spec_mxm (C, [ ], [ ], semiring, A, B, dnn);
                GB_spec_compare (C0, C1, identity) ;

                % C = B*A
                C1 = GB_mex_mxm  (C, [ ], [ ], semiring, B, A, dnn);
                C0 = GB_spec_mxm (C, [ ], [ ], semiring, B, A, dnn);
                GB_spec_compare (C0, C1, identity) ;

            end
        end
    end

fprintf ('\nsemirings tested: %d\n', n_semirings) ;
fprintf ('\ntest125: all tests passed\n') ;


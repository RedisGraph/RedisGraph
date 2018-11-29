function test75
%TEST75 test GrB_mxm and GrB_vxm on all semirings (A'B dot product)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

m = 200 ;
n = 5 ;
A_sparse = sprandn (m, n, 0.1) ;
A_sparse (:,3) = 0 ;
A_sparse (2,3) = 1.7 ;
A_sparse (18,3) = 2.2 ;
A_sparse (:,1:2) = sparse (rand (m,2)) ;
A_sparse (1,1) = 0;
A_sparse (18,1) = 0;
A_sparse (:,5) = 0 ;
A_sparse (1,5) = 11 ;
A_sparse (2,5) = 23 ;
A_sparse (18,5) = 33 ;

B_sparse = sprandn (m, n, 0.1) ;
B_sparse (:,1) = 0 ;
B_sparse (1,1) = 3 ;
B_sparse (18,1) = 2 ;
B_sparse (:,[2 n]) = sparse (rand (m,2)) ;
B_sparse (3,2) = 0 ;
B_sparse (18,2) = 0 ;
A_sparse (:,3) = 0 ;
B_sparse (2,1) = 7 ;
B_sparse (18,1) = 8 ;
B_sparse (19,1) = 9 ;

x_sparse = sparse (rand (m,1)) ;
x_sparse (99) = 0 ;

y_sparse = sparse (zeros (m,1)) ;
y_sparse (99) = 1 ;

A.matrix = A_sparse ;
A.class = 'see below' ;
A.pattern = logical (spones (A_sparse)) ;

B.matrix = B_sparse ;
B.class = 'see below' ;
B.pattern = logical (spones (B_sparse)) ;

X.matrix = x_sparse ;
X.class = 'see below' ;
X.pattern = logical (spones (x_sparse)) ;

Y.matrix = y_sparse ;
Y.class = 'see below' ;
Y.pattern = logical (spones (y_sparse)) ;

fprintf ('\n-------------- GrB_mxm, vxm (dot product) on all semirings\n') ;

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

Cin = sparse (n, n) ;
Xin = sparse (n, 1) ;

Mask = sparse (ones (n,n)) ;
mask = sparse (ones (n,1)) ;

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

n_semirings = 0 ;

% eq_eq_bool: 18, 8, 1

for k1 = 1:length(mult_ops)
    mulop = mult_ops {k1} ;
    fprintf ('\n%s', mulop) ;

    for k2 = 1:length(add_ops)
        addop = add_ops {k2} ;

        for k3 = 1:length (classes)
            clas = classes {k3} ;

            semiring.multiply = mulop ;
            semiring.add = addop ;
            semiring.class = clas ;

            % create the semiring.  some are not valid because the or,and,xor,eq
            % monoids can only be used when z is boolean for z=mult(x,y).
            try
                [mult_op add_op id] = GB_spec_semiring (semiring) ;
                [mult_opname mult_opclass zclass] = GB_spec_operator (mult_op) ;
                [ add_opname  add_opclass] = GB_spec_operator (add_op) ;
                identity = GB_spec_identity (semiring.add, add_opclass) ;
            catch me
                if (~isempty (strfind (me.message, 'gotcha')))
                    semiring
                    pause
                end
                continue
            end

            % there are 1344 semirings that pass this test:
            % 17 ops: 8:(1st, 2nd, min, max, plus, minus, times, div)
            %         6:(is*)
            %         3:(or,and,xor)
            %       TxT->T
            %       each has 44 monoids: all 11 types: max,min,plus,times
            %       and 4 for boolean or,and,xor,eq
            %       17*48 = 816
            % 6 ops: eq,ne,gt,lt,ge,le
            %       TxT->bool
            %       each has 11 types
            %       and 8 monoids (max,min,plus,times,or,and,xor,eq)
            %       6*11*8 = 528
            % 816 + 528 = 1344
            % but only 960 are unique.
            % see GrB_AxB_builtin for details.

            A.class = clas ;
            B.class = clas ;
            X.class = clas ;
            Y.class = clas ;

            n_semirings = n_semirings + 1 ;
            fprintf ('.') ;

            % C = A'*B, with mask
            tic
            C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn);
            t2 = toc ;
            C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtn);
            GB_spec_compare (C1, C2, id) ;

            % X = u*A, with mask
            tic
            C1 = GB_mex_vxm  (Xin, mask, [ ], semiring, X, A, [ ]);
            t2 = toc ;
            C2 = GB_spec_vxm (Xin, mask, [ ], semiring, X, A, [ ]);
            GB_spec_compare (C1, C2, id) ;

            if (k3 == 1)
                % repeat but with typecasing, to test generic A'*B
                A.class = 'double' ;

                % C = A'*B, with mask
                tic
                C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn);
                t2 = toc ;
                C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtn);
                GB_spec_compare (C1, C2, id) ;

                % X = u*A, with mask
                tic
                C1 = GB_mex_vxm  (Xin, mask, [ ], semiring, X, A, [ ]);
                t2 = toc ;
                C2 = GB_spec_vxm (Xin, mask, [ ], semiring, X, A, [ ]);
                GB_spec_compare (C1, C2, id) ;

                % X = u*A, with mask
                tic
                C1 = GB_mex_vxm  (Xin, mask, [ ], semiring, Y, A, [ ]);
                t2 = toc ;
                C2 = GB_spec_vxm (Xin, mask, [ ], semiring, Y, A, [ ]);
                GB_spec_compare (C1, C2, id) ;

            end
        end
    end
end

n_semirings

fprintf ('\ntest75: all tests passed\n') ;


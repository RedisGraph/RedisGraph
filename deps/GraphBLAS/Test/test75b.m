function test75b
%TEST75B GrB_mxm and GrB_vxm on all semirings (shorter test than test75)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops, ~, add_ops, classes, ~, ~] = GB_spec_opsall ;

% cstart = grb_get_coverage ;
% fprintf ('coverage start: %d\n', cstart) ;
% cc = zeros (15,1) ;

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

Cin = sparse (n, n) ;

Din = 10 * sparse (rand (n, n)) ;
D.matrix = Din ;
D.class = 'see below' ;
D.pattern = true (n,n) ;

Xin = sparse (n, 1) ;

Mask = sparse (ones (n,n)) ;
mask = sparse (ones (n,1)) ;

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dtn_dot   = struct ( 'inp0', 'tran', 'axb', 'dot' ) ;
dtn_saxpy = struct ( 'inp0', 'tran', 'axb', 'saxpy' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

n_semirings = 0 ;

% ccall = zeros (1,15)  ;

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

            A.class = clas ;
            B.class = clas ;
            X.class = clas ;
            Y.class = clas ;
            D.class = add_op.opclass ;

            n_semirings = n_semirings + 1 ;
            fprintf ('.') ;

            % C<M> = A'*B, with mask
% c1 = grb_get_coverage  ;
%             C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn_dot);
%             C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtn) ;
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (1) = cc (1) + cdelta ;
% c1 = grb_get_coverage  ;
%             C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn_saxpy);
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (2) = cc (2) + cdelta ;

            % C<M> += A'*B, C dense, typecasting of C
% c1 = grb_get_coverage  ;
%             C1 = GB_mex_mxm  (Din, Mask, add_op, semiring, A, B, dtn_dot) ;
%             C2 = GB_spec_mxm (Din, Mask, add_op, semiring, A, B, dtn) ;
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (3) = cc (3) + cdelta ;
% c1 = grb_get_coverage  ;
%             C1 = GB_mex_mxm  (Din, Mask, add_op, semiring, A, B, dtn_saxpy) ;
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (4) = cc (4) + cdelta ;
% c1 = grb_get_coverage  ;

            % C<M> += A'*B, C dense, no typecasting of C
%             C1 = GB_mex_mxm  (D, Mask, add_op, semiring, A, B, dtn_dot) ;
%             C2 = GB_spec_mxm (D, Mask, add_op, semiring, A, B, dtn) ;
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (5) = cc (5) + cdelta ;
% c1 = grb_get_coverage  ;
%             C1 = GB_mex_mxm  (D, Mask, add_op, semiring, A, B, dtn_saxpy) ;
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (6) = cc (6) + cdelta ;
% c1 = grb_get_coverage  ;

            % C += A'*B, C dense, typecasting of C
            % (test coverage: 96)
            C1 = GB_mex_mxm  (Din, [ ], add_op, semiring, A, B, dtn_dot) ;
            C2 = GB_spec_mxm (Din, [ ], add_op, semiring, A, B, dtn) ;
            GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (7) = cc (7) + cdelta ;
% c1 = grb_get_coverage  ;
%            C1 = GB_mex_mxm  (Din, [ ], add_op, semiring, A, B, dtn_saxpy) ;
%            GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (8) = cc (8) + cdelta ;
% c1 = grb_get_coverage  ;

            % C += A'*B, C sparse, no typecasting of C
            % (test coverage: 1,234)
            C1 = GB_mex_mxm  (D, [ ], add_op, semiring, A, B, dtn_dot) ;
            C2 = GB_spec_mxm (D, [ ], add_op, semiring, A, B, dtn) ;
            GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (10) = cc (10) + cdelta ;
% c1 = grb_get_coverage  ;
%             C1 = GB_mex_mxm  (D, [ ], add_op, semiring, A, B, dtn_saxpy) ;
%             GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (11) = cc (11) + cdelta ;
% c1 = grb_get_coverage  ;

            % X = u*A, with mask (test coverage: 12)
            C1 = GB_mex_vxm  (Xin, mask, [ ], semiring, X, A, [ ]) ;
            C2 = GB_spec_vxm (Xin, mask, [ ], semiring, X, A, [ ]) ;
            GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (12) = cc (12) + cdelta ;
% c1 = grb_get_coverage  ;

%             if (k3 == 1)
%               % repeat but with typecasting, to test generic A'*B
%               A.class = 'double' ;

%               % C = A'*B, with mask
%               C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn);
%               C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtn);
%               GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (13) = cc (13) + cdelta ;
% c1 = grb_get_coverage  ;

%               % X = u*A, with mask
%               C1 = GB_mex_vxm  (Xin, mask, [ ], semiring, X, A, [ ]);
%               C2 = GB_spec_vxm (Xin, mask, [ ], semiring, X, A, [ ]);
%               GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (14) = cc (14) + cdelta ;
% c1 = grb_get_coverage  ;

%               % X = u*A, with mask
%               C1 = GB_mex_vxm  (Xin, mask, [ ], semiring, Y, A, [ ]);
%               C2 = GB_spec_vxm (Xin, mask, [ ], semiring, Y, A, [ ]);
%               GB_spec_compare (C1, C2, id) ;
% c2 = grb_get_coverage ; cdelta = c2 - c1 ; cc (15) = cc (15) + cdelta ;

%            end

% ccall (n_semirings,:) = cc ;
        end
    end
end

n_semirings
% cc
% cfin = grb_get_coverage ;
% fprintf ('coverage end: %d\n', cfin) ;
% save saveme2 cc ccall

fprintf ('\ntest75b: all tests passed\n') ;



function test06 (A,B)
%TEST06 test GrB_mxm on all semirings
%
% Usage: test06(A)
%
% with no input, a small 10-by-10 matrix is used.  If A is a scalar, it is a
% matrix id number from the SuiteSparse collection otherwise A is the sparse
% matrix to use in the test

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n-------------- GrB_mxm on all semirings\n') ;

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

rng ('default') ;
if nargin < 1
    % w = load ('../Demo/Matrix/west0067') ;
    % A = sparse (w (:,1)+1, w (:,2)+1, w (:,3)) ;
    w = load ('../Demo/Matrix/ibm32a') ;
    nz = size (w,1) ;
    n = max (max (w (:,1:2))) + 1 ;
    A = sparse (w (:,1)+1, w (:,2)+1, 20 * rand (nz,1) - 10, n, n) ;
    B = A (16:25, 16:25) .* rand (10) ;
    A = A (1:10,1:10) ;
elseif (isscalar (A))
    Prob = ssget (A)
    A = Prob.A ;
    clear Prob
    A (1,2) = 1 ;
    B = 2*A ;
end

assert (issparse (A)) ;

[m n] = size (A) ;
Cin = sparse (m, n) ;
assert (m == n) ;

rng ('default') ;
[i, j, ~] = find (A) ;
nz = nnz (A) ;
p = randperm (nz, floor (nz/2)) ;
Mask = sparse (i (p), j (p), ones (length (p),1), m, n) + ...
       spones (sprandn (m, n, 1/n)) ;

tic
C = A*B ;
tm1 = toc ;

tic
C = A'*B ;
tm2 = toc ;

tic
C = A*B' ;
tm3 = toc ;

tic
C = A'*B' ;
tm4 = toc ;

if (n > 500)
    fprintf ('MATLAB time: %g %g %g %g\n', tm1, tm2, tm3, tm4) ;
    fprintf ('with mask:\n') ;
end

tic
C = (A*B) .* Mask ;
tmm1 = toc ;

tic
C = (A'*B) .* Mask ;
tmm2 = toc ;

tic
C = (A*B') .* Mask ;
tmm3 = toc ;

tic
C = (A'*B') .* Mask ;
tmm4 = toc ;

if (n > 500)
    fprintf ('MATLAB time: %g %g %g %g\n', tmm1, tmm2, tmm3, tmm4) ;
end

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

n_semirings = 0 ;

n = size (A,1) ;

for k1 = 1:length(mult_ops)
    mulop = mult_ops {k1} ;
    if (n <= 500)
        fprintf ('\n%s', mulop) ;
    end

    for k2 = 1:length(add_ops)
        addop = add_ops {k2} ;

        for k3 = 1:length (classes)
            clas = classes {k3} ;
            if (n <= 500)
                fprintf ('.') ;
            end

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

            n_semirings = n_semirings + 1 ;
            if (n > 500)
                fprintf ('%3d ', n_semirings) ;
                fprintf ('mult: %6s add:%6s class %8s : ', mulop, addop, clas) ;
            end

            % C = A*B, no mask
            tic
            C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dnn);
            t1 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dnn);
            GB_spec_compare (C1, C2, id) ;
            end

            % C = A'*B, no mask
            tic
            C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dtn);
            t2 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dtn);
            GB_spec_compare (C1, C2, id) ;
            end

            % C = A*B', no mask
            tic
            C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dnt);
            t3 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dnt);
            GB_spec_compare (C1, C2, id) ;
            end

            % C = A'*B', no mask
            tic
            C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dtt);
            t4 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dtt);
            GB_spec_compare (C1, C2, id) ;
            end

            if (n > 500)
                fprintf ('speedups %10.4f %10.4f %10.4f %10.4f ', ...
                    tm1 /t1, tm2/t2, tm3/t3, tm4/t4) ;
            end

            % C = A*B, with mask
            tic
            C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dnn);
            t1 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dnn);
            GB_spec_compare (C1, C2, id) ;
            end

            % C = A'*B, with mask
            tic
            C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn);
            t2 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtn);
            GB_spec_compare (C1, C2, id) ;
            end

            % C = A*B', with mask
            tic
            C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dnt);
            t3 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dnt);
            GB_spec_compare (C1, C2, id) ;
            end

            % C = A'*B', with mask
            tic
            C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtt);
            t4 = toc ;
            if (n < 100)
            C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtt);
            GB_spec_compare (C1, C2, id) ;
            end

            if (n > 500)
                fprintf ('mask %10.4f %10.4f %10.4f %10.4f\n', ...
                    tmm1 /t1, tmm2/t2, tmm3/t3, tmm4/t4) ;
            end

        end
    end
end

n_semirings

fprintf ('\ntest06: all tests passed\n') ;


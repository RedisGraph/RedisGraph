function test06 (A,B,fulltests,method_list)
%TEST06 test GrB_mxm on all semirings
%
% Usage: test06(A)
%        test06(A,B)
%        test06(A,B,fulltests)
%
% with no input, a small 10-by-10 matrix is used.  If A is a scalar, it is a
% matrix id number from the SuiteSparse collection otherwise A is the sparse
% matrix to use in the test

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('test06: GrB_mxm on all semirings\n') ;

[mult_ops, ~, add_ops, classes, ~, ~] = GB_spec_opsall ;

if (nargin < 3)
    fprintf ('\n-------------- GrB_mxm on all semirings\n') ;
    fulltests = 1 ;
end

if (nargin < 2)
    B = [ ] ;
end

if (nargin < 4)
    method_list = 0:3 ;
end

rng ('default') ;
if (nargin < 1 || isempty (A))
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
    if (isempty (B))
        B = 2*A ;

%       n = size (A,2) ;
%       p = randperm (n) ;
%       B = sparse (1:n, p, ones (n,1)) ;
%       p = randperm (n) ;
%       B = B + sparse (1:n, p, ones (n,1)) ;
%       p = randperm (n) ;
%       B = B + sparse (1:n, p, ones (n,1)) ;
%       e = ones (n,1) ;
%       % B = spdiags([e -2*e e], -1:1, n, n) ;
%       % B = spdiags([-2*e e], 0:1, n, n) ;
%       % B = speye(n);
%       T = A ; A = B ; B = T ;

    end
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

% Mask = spones (A) ;
% Mask = sparse (i (p), j (p), ones (length (p),1), m, n) ;

tic
C = A'*B ;
tm2 = toc ;

tic
C = A*B' ;
tm3 = toc ;

tic
C = A'*B' ;
tm5 = toc ;

if (n > 500)
    fprintf ('MATLAB time: %g %g %g %g\n', tm1, tm2, tm3, tm5) ;
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
tmm5 = toc ;

if (n > 500)
    fprintf ('MATLAB time: %g %g %g %g\n', tmm1, tmm2, tmm3, tmm5) ;
end

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

n_semirings = 0 ;

if (fulltests)
    k1_list = 1:length(mult_ops) ;
    k2_list = 1:length(add_ops) ;
    k3_list = 1:length (classes) ;
else
    % just use plus-times-double semiring
    k1_list = 8 ;
    k2_list = 3 ;
    k3_list = 11 ;
end

n = size (A,1) ;

for k1 = k1_list % 1:length(mult_ops)
    mulop = mult_ops {k1} ;
    if (n <= 500)
        fprintf ('\n%s', mulop) ;
    end

    for k2 = k2_list % 1:length(add_ops)
        addop = add_ops {k2} ;

        for k3 = k3_list % 1:length (classes)
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

            % there are 1440 semirings that pass this test:
            % 19 ops: 10:(1st, 2nd, min, max, plus, minus, rminus, times,
            %            div, rdiv)
            %         6:(is*)
            %         3:(or,and,xor)
            %       TxT->T
            %       each has 44 monoids: all 11 types: max,min,plus,times
            %       and 4 for boolean or,and,xor,eq
            %       17*48 = 912
            % 6 ops: eq,ne,gt,lt,ge,le
            %       TxT->bool
            %       each has 11 types
            %       and 8 monoids (max,min,plus,times,or,and,xor,eq)
            %       6*11*8 = 528
            % 912 + 528 = 1440
            % but only 1040 are unique.
            % see GrB_AxB_builtin for details.

            n_semirings = n_semirings + 1 ;

            for method = method_list % 0:3

                if (n > 500)
                    fprintf ('%3d ', n_semirings) ;
                    fprintf ('[%6s %6s %8s] : ', mulop, addop, clas) ;
                end

                if (method == 1)
                    algo = 'hash' ;
                    if (n > 500)
                        fprintf ('hash ') ;
                    end
                elseif (method == 2)
                    algo = 'gustavson' ;
                    if (n > 500)
                        fprintf ('g/s  ') ;
                    end
                elseif (method == 3)
                    algo = 'dot' ;
                    if (n > 500)
                        fprintf ('dot  ') ;
                    end
                else
                    algo = 'default' ;
                    if (n > 500)
                        fprintf ('auto ') ;
                    end
                end
                if (isequal (algo, 'dot'))
                    ok = (n < 1000) ;
                else
                    ok = true ;
                end

                dnn.axb = algo ;
                dnt.axb = algo ;
                dtn.axb = algo ;
                dtt.axb = algo ;

                t1 = nan ; method1 = 'x' ; method1m = 'x' ;
                t2 = nan ; method2 = 'x' ; method2m = 'x' ;
                t3 = nan ; method3 = 'x' ; method3m = 'x' ;
                t4 = nan ; method4 = 'x' ; method4m = 'x' ;

                % C = A*B, no mask
                % tic
                if (ok)
                C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dnn) ;
                [t1 method1] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dnn);
                GB_spec_compare (C1, C2, id) ;
                end
                end

                % C = A'*B, no mask
                if (ok)
                C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dtn);
                [t2 method2] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dtn);
                GB_spec_compare (C1, C2, id) ;
                end
                end

                % C = A*B', no mask
                if (ok)
                C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dnt);
                [t3 method3] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dnt);
                GB_spec_compare (C1, C2, id) ;
                end
                end

                % C = A'*B', no mask
                if (ok)
                C1 = GB_mex_mxm  (Cin, [ ], [ ], semiring, A, B, dtt);
                [t4 method4] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, [ ], [ ], semiring, A, B, dtt);
                GB_spec_compare (C1, C2, id) ;
                end
                end

                if (n > 500)
                    fprintf (...
                    'speedups %10.4f(%s) %10.4f(%s) %10.4f(%s) %10.4f(%s) ', ...
                    tm1/t1, method1(1), tm2/t2, method2(1), ...
                    tm3/t3, method3(1), tm5/t4, method4(1)) ;
                end

                % C = A*B, with mask
                % tic
                C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dnn);
                [t1 method1m] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dnn);
                GB_spec_compare (C1, C2, id) ;
                end

                % C = A'*B, with mask
                % tic
                C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtn);
                [t2 method2m] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtn);
                GB_spec_compare (C1, C2, id) ;
                end

                % C = A*B', with mask
                % tic
                C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dnt);
                [t3 method3m] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dnt);
                GB_spec_compare (C1, C2, id) ;
                end

                % C = A'*B', with mask
                % tic
                C1 = GB_mex_mxm  (Cin, Mask, [ ], semiring, A, B, dtt);
                [t4 method4m] = grbresults ; % toc ;
                if (n < 200)
                C2 = GB_spec_mxm (Cin, Mask, [ ], semiring, A, B, dtt);
                GB_spec_compare (C1, C2, id) ;
                end

                if (n > 500)
                    fprintf (...
                    'speedups %10.4f(%s) %10.4f(%s) %10.4f(%s) %10.4f(%s) ', ...
                    tmm1/t1, method1m(1), tmm2/t2, method2m(1), ...
                    tmm3/t3, method3m(1), tmm5/t4, method4m(1)) ;
                    fprintf ('\n') ;
                end

            end
        end
    end
end

% n_semirings

if (fulltests)
    fprintf ('\ntest06: all tests passed\n') ;
end


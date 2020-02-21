function test63
%TEST63 test GraphBLAS operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops, ~, ~, classes, ~, ~] = GB_spec_opsall ;

fprintf ('\n ---------------------------- testing GB_mex_op\n') ;

rng ('default') ;

n_operators = 0 ;
for k2 = 1:length(mult_ops)
    mulop = mult_ops {k2} ;
    fprintf ('[%s]', mulop) ;

    for k1 = 1:length (classes)
        clas = classes {k1} ;

        % create the op
        clear op
        op.opname = mulop ;
        op.opclass = clas ;
        % GB_mex_binaryop (op) ;
        fprintf ('.') ;

        for m = [ 1:3:10 ]% 100]
            for n = [1:3:10 ]% 100]
                for hi = [-1:5 200]
                    for lo = [-3:5 100]
                        A = full ((hi*sprand (m,n,0.8)-lo) .* sprand (m,n,0.5));
                        B = full ((hi*sprand (m,n,0.8)-lo) .* sprand (m,n,0.5));
                        n = 0 ;
                        % use pure GraphBLAS
                        Z1 = GB_mex_op (op, A, B) ;
                        % use MATLAB as much as possible
                        Z2 = GB_spec_op (op, A, B) ;
                        % the results should match perfectly
                        assert (isequalwithequalnans (Z1, Z2))
                    end
                end
            end
        end
        n_operators = n_operators + 1  ;
    end
end

fprintf ('\nall tests passed\n') ;

fprintf ('Number of built-in GraphBLAS operators: %d\n',  n_operators) ;

fprintf ('\ntest63: all tests passed\n') ;


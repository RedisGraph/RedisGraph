function gbtest42
%GBTEST42 test for nan

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
types = { 'single', 'double' } ;

for trial = 1:40
    fprintf ('.') ;

    for k1 = 1:length(types)
        atype = types {k1} ;
        A = cast (full (sprand (4,4,0.5)), atype) ;
        A (A > 0.5) = nan ;

        A_nan = zeros (4, 4) ;
        A_nan (isnan (A)) = nan ;
        A_notnan = zeros (4, 4) ;
        A_notnan (~isnan (A)) = A (~isnan (A)) ;

        for k2 = 1:length(types)
            xtype = types {k2} ;
            xnan = cast (nan, xtype) ;

            G = GrB.select (A, '==', xnan) ;
            X1 = full (double (G)) ;
            X2 = double (A_nan) ;
            assert (isequaln (X1, X2)) ;

            G = GrB.select (A, '~=', xnan) ;
            X1 = full (double (G)) ;
            X2 = double (A_notnan) ;
            assert (isequaln (X1, X2)) ;

            G = GrB.prune (A, xnan) ;
            X1 = full (double (G)) ;
            X2 = double (A_notnan) ;
            assert (isequaln (X1, X2)) ;

        end
    end

end

fprintf ('\ngbtest42: all tests passed\n') ;


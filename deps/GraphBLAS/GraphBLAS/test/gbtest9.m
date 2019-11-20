function gbtest9
%GBTEST9 test eye and speye

types = gbtest_types ;

A = eye ;
G = GrB.eye ;
assert (gbtest_eq (A, G)) ;
G = GrB.speye ;
assert (gbtest_eq (A, G)) ;

for m = -1:10
    fprintf ('.') ;

    A = eye (m) ;
    G = GrB.eye (m) ;
    assert (gbtest_eq (A, G)) ;
    G = GrB.speye (m) ;
    assert (gbtest_eq (A, G)) ;

    for n = -1:10

        A = eye (m, n) ;
        G = GrB.eye (m, n) ;
        assert (gbtest_eq (A, G)) ;
        G = GrB.speye (m, n) ;
        assert (gbtest_eq (A, G)) ;

        for k = 1:length (types)
            type = types {k} ;

            A = eye (m, n, type) ;
            G = GrB.eye (m, n, type) ;
            assert (gbtest_eq (A, G)) ;
            G = GrB.speye (m, n, type) ;
            assert (gbtest_eq (A, G)) ;

            A = eye ([m n], type) ;
            G = GrB.eye ([m n], type) ;
            assert (gbtest_eq (A, G)) ;
            G = GrB.speye ([m n], type) ;
            assert (gbtest_eq (A, G)) ;

            A = eye (m, type) ;
            G = GrB.eye (m, type) ;
            assert (gbtest_eq (A, G)) ;
            G = GrB.speye (m, type) ;
            assert (gbtest_eq (A, G)) ;

        end
    end
end

fprintf ('\ngbtest9: all tests passed\n') ;


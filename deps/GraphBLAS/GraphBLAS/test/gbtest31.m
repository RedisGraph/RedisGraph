function gbtest31
%GBTEST31 test GrB and casting

types = gbtest_types ;
fprintf ('gbtest31: typecasting\n') ;
rng ('default') ;

for k = 1:length (types)
    type = types {k} ;
    fprintf ('%s ', type) ;

    for m = 0:5
        for n = 0:5
            A = zeros (m, n, type) ;
            G = GrB (m, n, type) ;
            C = cast (G, type) ;
            assert (gbtest_eq (A, C)) ;
        end
    end

    A = 100 * rand (5, 5) ;
    A = cast (A, type) ;
    G = GrB (A) ;

    G2 = sparse (G) ;
    assert (gbtest_eq (G, G2)) ;

    for k2 = 1:length (types)
        type2 = types {k} ;
        G2 = GrB (G, type2) ;
        A2 = cast (A, type2) ;
        C = cast (G2, type2) ;
        assert (gbtest_eq (A2, C)) ;
    end

    F = 100 * ones (5, 5) ;
    F = cast (F, type) ;
    id = F (1,1) ;

    A = 100 * sparse (diag (1:5)) ;

    G = GrB (A, type) ;
    G2 = GrB (F) ;
    G2 (logical (speye (5))) = 100:100:500 ;

    for k2 = 1:length (types)
        type2 = types {k} ;
        G3 = full (G, type2, id) ;
        G5 = full (G, type2, GrB (id)) ;
        G4 = GrB (G2, type2) ;
        assert (gbtest_eq (G3, G4)) ;
        assert (gbtest_eq (G3, G5)) ;
        assert (gbtest_eq (double (G3), double (G4))) ;
        assert (gbtest_eq (single (G3), single (G4))) ;
        assert (gbtest_eq (uint16 (G3), uint16 (G4))) ;
    end

end

fprintf ('\ngbtest31: all tests passed\n') ;

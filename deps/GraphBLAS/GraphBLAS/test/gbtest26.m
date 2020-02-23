function gbtest26
%GBTEST26 test typecasting

types = gbtest_types ;

rng ('default') ;
for k1 = 1:length (types)

    atype = types {k1} ;
    fprintf ('\n================================================ %s\n', atype) ;
    A = cast (100 * rand (3), atype) %#ok<*NOPRT>
    H = GrB (A) ;
    B = cast (H, atype) ;
    assert (gbtest_eq (A, B)) ;

    for k2 = 1:length (types)

        gtype = types {k2} ;
        fprintf ('\n------------ %s:\n', gtype) ;
        G = GrB (H, gtype)
        K = GrB (G, atype) %#ok<*NASGU>
        C = cast (G, atype)
    end
end

fprintf ('gbtest26: all tests passed\n') ;


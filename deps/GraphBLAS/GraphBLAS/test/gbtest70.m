function gbtest70
%GBTEST70 test GrB.random

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.


rng ('default') ; A = sprand (4, 5, 0.5) ;
rng ('default') ; C0 = sprand (A) ;
rng ('default') ; C1 = GrB.random (A) ;
assert (isequal (C0, C1)) ;

types = gbtest_types ;

rng ('default') ;

for k = 1:length(types)
    type = types {k} ;

    rng ('default') ;
    G = GrB.random (30, 40, 0.6) ; %#ok<*NASGU>

    r = cast ([3 40], type) ;
    G = GrB.random (300, 400, 0.6, 'range', r) ;
    assert (isequal (GrB.type (G), type)) ;

    if (~isequal (type, 'logical'))
        [i,j,x] = find (G) ; %#ok<*ASGLU>
        if (isinteger (r))
            assert (min (r) == min (r)) ;
            assert (max (r) == max (r)) ;
        else
            d = min (x) - min (r) ; assert (d > 0 && d < 0.01) ;
            d = max (r) - max (x) ; assert (d > 0 && d < 0.01) ;
        end
    end

    G = GrB.random (30, 40, 0.6, 'normal') ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = GrB.random (30, 40, inf, 'normal') ;
    assert (isequal (GrB.type (G), 'double')) ;
    assert (nnz (G) == prod (size (G))) ; %#ok<*PSIZE>

    G = GrB.random (30, 40, 0.6, 'normal', 'range', r) ;
    assert (isequal (GrB.type (G), type)) ;

    G = GrB.random (30, 40, 0.6, 'uniform') ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = GrB.random (30, 40, 0.6, 'uniform', 'range', r) ;
    assert (isequal (GrB.type (G), type)) ;

    G = GrB.random (30, 0.6, 'symmetric') ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = GrB.random (30, inf, 'symmetric') ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;
    assert (nnz (G) == prod (size (G))) ;

    G = GrB.random (30, 30, 0.6, 'unsymmetric') ;
    assert (~issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = GrB.random (30, 0.6, 'normal', 'symmetric') ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = GrB.random (30, 0.6, 'normal', 'range', r, 'symmetric') ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), type)) ;

    S = sprandsym (30, 0.6) ;
    G = sprandsym (GrB (S)) ;
    assert (isequal (spones (G), spones (S))) ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = sprandsym (GrB (S), 'uniform') ;
    assert (isequal (spones (G), spones (S))) ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;

    G = sprandsym (GrB (S), 'normal') ;
    assert (isequal (spones (G), spones (S))) ;
    assert (issymmetric (G)) ;
    assert (isequal (GrB.type (G), 'double')) ;

    S = sprandn (30, 40, 0.6) ;
    G = sprandn (GrB (S)) ;
    assert (isequal (spones (G), spones (S))) ;
    assert (isequal (GrB.type (G), 'double')) ;

    S = sprand (30, 40, 0.6) ;
    G = sprand (GrB (S)) ;
    assert (isequal (spones (G), spones (S))) ;
    assert (isequal (GrB.type (G), 'double')) ;

end


fprintf ('gbtest70: all tests passed\n') ;


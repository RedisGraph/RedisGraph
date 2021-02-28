function gbtest46
%GBTEST46 test GrB.subassign and GrB.assign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
d.kind = 'sparse' ;

types = gbtest_types ;
for k = 1:length (types)
    type = types {k} ;
    A = cast (rand (4) * 100, type) ;
    C = GrB.subassign (A, {1}, {1}, cast (pi, type)) ;
    A (1,1) = pi ;
    assert (isequal (A, C)) ;
end

for trial = 1:40

    A = rand (4) ;
    G = GrB (A) ;
    pg = GrB (pi) ;

    C1 = A ;
    C1 (1:3,1) = pi ;

    C2 = GrB.subassign (A, pi, { 1:3}, { 1 }) ;
    C3 = GrB.subassign (G, pi, { 1:3}, { 1 }) ;
    C4 = GrB.subassign (G, pg, { 1:3}, { 1 }) ;
    C5 = GrB.subassign (G, pg, { 1:3}, { 1 }, d) ;
    assert (isequal (C1, C2)) ;
    assert (isequal (C1, C3)) ;
    assert (isequal (C1, C4)) ;
    assert (isequal (C1, C5)) ;
    assert (isequal (class (C5), 'double')) ;

    C2 = GrB.assign (A, pi, { 1:3}, { 1 }) ;
    C3 = GrB.assign (G, pi, { 1:3}, { 1 }) ;
    C4 = GrB.assign (G, pg, { 1:3}, { 1 }) ;
    C5 = GrB.assign (G, pg, { 1:3}, { 1 }, d) ;
    assert (isequal (C1, C2)) ;
    assert (isequal (C1, C3)) ;
    assert (isequal (C1, C4)) ;
    assert (isequal (C1, C5)) ;
    assert (isequal (class (C5), 'double')) ;

end

fprintf ('gbtest46: all tests passed\n') ;


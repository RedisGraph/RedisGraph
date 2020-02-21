function gbtest47
%GBTEST47 test GrB.entries, GrB.nonz, numel

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

A = 100 * rand (4) ;
types = gbtest_types ;
for k = 1:length (types)
    type = types {k} ;
    B = cast (A, type) ;
    x1 = GrB.entries (B, 'list') ;
    x2 = unique (nonzeros (B)) ;
    assert (isequal (x1, x2)) ;
    assert (isequal (type, class (x1))) ;
    assert (isequal (type, class (x2))) ;
end

A = magic (4) ;
c0 = nnz (A) ;
c1 = GrB.nonz (A) ;
c2 = GrB.nonz (GrB (A)) ;
assert (c0 == c1) ;
assert (c1 == c2) ;

c1 = GrB.nonz (A, 0) ;
c2 = GrB.nonz (GrB (A), 0) ;
assert (c0 == c1) ;
assert (c1 == c2) ;

A = sparse (A) ;
c0 = nnz (A) ;

c1 = GrB.nonz (A) ;
c2 = GrB.nonz (GrB (A)) ;
assert (c0 == c1) ;
assert (c1 == c2) ;

c1 = GrB.nonz (A, 0) ;
c2 = GrB.nonz (GrB (A), 0) ;
assert (c0 == c1) ;
assert (c1 == c2) ;

A = sparse (A) ;
c0 = nnz (A ~= 1) ;
c1 = GrB.nonz (A, 1) ;
c2 = GrB.nonz (GrB (A), 1) ;
assert (c0 == c1) ;
assert (c1 == c2) ;

try
    x = vpa (1) ; %#ok<*NASGU>
    have_symbolic = true ;
catch
    % symbolic toolbox not available
    have_symbolic = false ;
end

for trial = 1:40
    fprintf ('.') ;

    A = rand (4) ;
    A (A > .5) = 0 ;
    G = GrB (A) ;

    c1 = GrB.entries (A) ;
    c2 = GrB.entries (G) ;
    assert (c1 == c2) ;
    assert (c1 == numel (A)) ;

    c1 = GrB.nonz (A) ;
    c2 = GrB.nonz (G) ;
    assert (c1 == c2) ;
    assert (c1 == nnz (A)) ;

    B = sparse (A) ;
    G = GrB (B) ;

    c1 = GrB.entries (B) ;
    c2 = GrB.entries (G) ;
    assert (c1 == c2) ;
    assert (c1 == nnz (B)) ;

    c1 = GrB.nonz (B) ;
    c2 = GrB.nonz (G) ;
    assert (c1 == c2) ;
    assert (c1 == nnz (B)) ;

    c1 = GrB.nonz (B, 0) ;
    c2 = GrB.nonz (G, 0) ;
    assert (c1 == c2) ;
    assert (c1 == nnz (B)) ;

    x1 = GrB.entries (B, 'list') ;
    x2 = GrB.entries (G, 'list') ;
    assert (isequal (x1, x2)) ;

    x1 = GrB.nonz (B, 'list') ;
    x2 = GrB.nonz (G, 'list') ;
    x0 = unique (nonzeros (B)) ;
    assert (isequal (x0, x2)) ;
    assert (isequal (x1, x2)) ;

    d1 = GrB.entries (B, 'row') ;
    d2 = GrB.entries (G, 'row') ;
    d3 = length (find (sum (spones (B), 2))) ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.nonz (B, 'row') ;
    d2 = GrB.nonz (G, 'row') ;
    d3 = length (find (sum (spones (B), 2))) ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.entries (B, 'row', 'list') ;
    d2 = GrB.entries (G, 'row', 'list') ;
    d3 = find (sum (spones (B), 2)) ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.nonz (B, 'row', 'list') ;
    d2 = GrB.nonz (G, 'row', 'list') ;
    d3 = find (sum (spones (B), 2)) ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.entries (B, 'col') ;
    d2 = GrB.entries (G, 'col') ;
    d3 = length (find (sum (spones (B), 1))) ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.nonz (B, 'col') ;
    d2 = GrB.nonz (G, 'col') ;
    d3 = length (find (sum (spones (B), 1))) ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.entries (B, 'col', 'list') ;
    d2 = GrB.entries (G, 'col', 'list') ;
    d3 = find (sum (spones (B), 1))' ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    d1 = GrB.nonz (B, 'col', 'list') ;
    d2 = GrB.nonz (G, 'col', 'list') ;
    d3 = find (sum (spones (B), 1))' ;
    assert (isequal (d1, d2)) ;
    assert (isequal (d1, d3)) ;

    % requires vpa in the Symbolic toolbox:
    if (have_symbolic)
        Huge = GrB (2^30, 2^30) ;
        e = numel (Huge) ;
        assert (logical (e == 2^60)) ;
    end

end

fprintf ('\ngbtest47: all tests passed\n') ;


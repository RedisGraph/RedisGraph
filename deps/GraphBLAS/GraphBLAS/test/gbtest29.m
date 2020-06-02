function gbtest29
%GBTEST29 test subsref and subsasgn with logical indexing

rng ('default') ;

types = gbtest_types ;
for k = 1:length (types)
    type = types {k} ;
    C = cast (magic (3), type) ;
    M = logical (C > 3) ;
    A = cast (2 * magic (3), type) ;
    C (M) = A (M) ;
    G = GrB (magic (3), type) ;
    G (M) = A (M) ;
    assert (isequal (G, C)) ;
end

for trial = 1:40
    fprintf ('.')
    for m = 0:5
        for n = 0:5
            A = sprand (m, n, 0.5) ;
            C = sprand (m, n, 0.5) ;
            M = sprand (m, n, 0.5) ~= 0 ;
            G = GrB (A) ;

            x1 = A (M) ;
            x2 = G (M) ;

            % In MATLAB, if A and M are row vectors then A(M) is also a
            % row vector.  That contradicts this blog post:
            % https://www.mathworks.com/company/newsletters/articles/matrix-indexing-in-matlab.html
            % and also this documentation:
            % https://www.mathworks.com/help/matlab/math/array-indexing.html
            % Both those pages state that A(M) where M is a logical matrix
            % always returns a column vector.  GraphBLAS always returns a
            % column vector.  So x1(:) and x2(:) are compared below:
            assert (gbtest_eq (x1 (:), x2 (:))) ;

            % Both GraphBLAS and MATLAB can accept either row or column vectors
            % x on input to C (M) = x.  MATLAB treats this as C(M)=x(:), and so
            % does GraphBLAS.  So the subsasgn is in agreement between MATLAB
            % and GraphBLAS, and all these uses work:

            C1 = C ;
            C1 (M) = A (M) ;%#ok<*SPRIX> % C1(M) is MATLAB, A(M) is MATLAB

            C2 = GrB (C) ;
            C2 (M) = A (M) ;        % C2(M) is GrB, A(M) is MATLAB

            C3 = GrB (C) ;
            C3 (M) = G (M) ;        % C3(M) is GrB, and G(M) is GrB

            % this uses the MATLAB subasgn, after typecasting G(M) from class
            % GrB to class double, using GrB/double:
            C4 = C ;
            C4 (M) = G (M) ;

            assert (gbtest_eq (C1, C2)) ;
            assert (gbtest_eq (C1, C3)) ;
            assert (gbtest_eq (C1, C4)) ;

            % also try with a GrB mask matrix M.  In this case, A(M) where A
            % is a MATLAB matrix and M is a GrB logical matrix fails.
            % GrB/subsref can handle this case, but MATLAB
            % doesn't call it.  It tries using the built-in subsref instead,
            % and it doesn't know what to do with a GrB logical matrix M.

            M = GrB (M) ;
            C5 = GrB (C) ;
            C5 (M) = G (M) ;

            assert (gbtest_eq (C1, C5)) ;

            % test scalar assigment with logical indexing
            K = logical (M) ;
            C1 (K) = pi ;
            C2 (M) = pi ;
            C3 (M) = GrB (pi) ;
            C4 (K) = GrB (pi) ;
            C5 (M) = pi ;

            assert (gbtest_eq (C1, C2)) ;
            assert (gbtest_eq (C1, C3)) ;
            assert (gbtest_eq (C1, C4)) ;
            assert (gbtest_eq (C1, C5)) ;

        end
    end
end

fprintf ('\ngbtest29: all tests passed\n') ;



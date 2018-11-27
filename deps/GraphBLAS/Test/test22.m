function test22(fulltest)
%TEST22 test GrB_transpose

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 1)
    % do a short test, by default
    fulltest = 0 ;
end

[ops unary_ops add_ops classes] = GB_spec_opsall ;

if (fulltest)
    fprintf ('\n==== exhaustive test for GB_mex_transpose:\n') ;
    problems = [
        10,    8,   40,  -5, 100
        10,  20,  100,  -99, 200
        100, 200, 1000, -99, 200
         50,  50,  500,  -2, 3
        ] ;
else
    fprintf ('\n==== test GB_mex_transpose:\n') ;
    problems = [
        10,    8,   40,  -5, 100
        ] ;
end

% try several problems
for k0 = 1:size (problems,1) ;

    % create nnz triplets for a matrix of size nrows-by-nrows
    nrows = problems (k0,1) ;
    ncols = problems (k0,2) ;
    nnz = problems (k0,3) ;
    y1 = problems (k0,4) ;
    y2 = problems (k0,5) ;

    % create A
    rng ('default') ;
    I = irand (0, nrows-1, nnz, 1) ;
    J = irand (0, ncols-1, nnz, 1) ;
    Y = y2 * rand (nnz, 1) + y1 ;
    clear A
    A.matrix = sparse (double (I)+1, double (J)+1, Y, nrows, ncols) ;

    % create Cin; note that it has the same dimensions as A, so if A
    % gets transpose, Cin must also be transposed (so use Cin2 instead)
    I = irand (0, nrows-1, nnz, 1) ;
    J = irand (0, ncols-1, nnz, 1) ;
    Y = y2 * rand (nnz, 1) + y1 ;
    clear Cin
    Cin.matrix = sparse (double (I)+1, double (J)+1, Y, nrows, ncols) ;

    clear Cin2
    Cin2.matrix = Cin.matrix' ;

    clear Cempty
    Cempty.matrix = sparse (nrows, ncols) ;
    Cempty2.matrix = Cempty.matrix' ;

    % create a boolean Mask with roughly the same density as A and Cin
    Mask = cast (sprandn (nrows, ncols, nnz/(nrows*ncols)), 'logical') ;

    fprintf ('\nnrows: %d ncols %d nnz %d ymin %g ymax %g\n', ...
        nrows, ncols, nnz, min (Y), max (Y)) ;

    % try every class for A
    for k1 = 1:length (classes)
        aclass = classes {k1} ;
        A.class = aclass ;
        Cempty.class = aclass ;
        Cempty2.class = aclass ;
        fprintf ('%s', aclass) ;

        % C = A'
        C = GB_mex_transpose  (Cempty2, [ ], [ ], A, [ ]) ;
        assert (spok (C.matrix*1) == 1) ;
        S = GB_spec_transpose (Cempty2, [ ], [ ], A, [ ]) ;
        assert (isequal (C.class, A.class)) ;
        assert (isequal (C.class, S.class)) ;
        assert (isequal (full (double (C.matrix)), double (S.matrix))) ;
        if (isequal (C.class, 'double'))
            assert (isequal (C.matrix, A.matrix')) ;
        end

        % C = A
        clear D
        D = struct ('inp0', 'tran') ;
        C = GB_mex_transpose  (Cempty, [ ], [ ], A, D) ;
        assert (spok (C.matrix*1) == 1) ;
        S = GB_spec_transpose (Cempty, [ ], [ ], A, D) ;
        assert (isequal (C.class, A.class)) ;
        assert (isequal (C.class, S.class)) ;
        assert (isequal (full (double (C.matrix)), double (S.matrix))) ;
        if (isequal (C.class, 'double'))
            assert (isequal (C.matrix, A.matrix)) ;
        end

        % C<Mask> = A'
        Cempty2.class = A.class ;
        C = GB_mex_transpose  (Cempty2, Mask', [ ], A, [ ]) ;
        assert (spok (C.matrix*1) == 1) ;
        S = GB_spec_transpose (Cempty2, Mask', [ ], A, [ ]) ;
        assert (isequal (C.class, A.class)) ;
        assert (isequal (C.class, S.class)) ;
        assert (isequal (full (double (C.matrix)), double (S.matrix))) ;
        if (isequal (C.class, 'double'))
            assert (isequal (C.matrix .* Mask', (A.matrix').*Mask')) ;
        end

        % C<Mask> = A
        clear D
        D = struct ('inp0', 'tran') ;
        Cempty.class = A.class ;
        C = GB_mex_transpose  (Cempty, Mask, [ ], A, D) ;
        assert (spok (C.matrix*1) == 1) ;
        S = GB_spec_transpose (Cempty, Mask, [ ], A, D) ;
        assert (isequal (C.class, A.class)) ;
        assert (isequal (C.class, S.class)) ;
        assert (isequal (full (double (C.matrix)), double (S.matrix))) ;
        if (isequal (C.class, 'double'))
            assert (isequal (C.matrix .* Mask, A.matrix .* Mask)) ;
        end

        % try every class for Cin
        for k2 = 1:length (classes)
            cinclass = classes {k2} ;
            fprintf ('.') ;
            Cin2.class = cinclass ;
            Cin.class = cinclass ;

            % try every operator
            for k3 = 0:size (ops,1)
                if (k3 == 0)
                    op = '' ;
                    nclasses = 1 ;
                else
                    op = ops {k3,1} ;
                    nclasses = length (classes) ;
                end

                % try every operator class
                for k4 = 1:nclasses
                    if (isempty (op))
                        opclass = '' ;
                    else
                        opclass = classes {k4} ;
                    end

                    clear accum
                    accum.opname = op ;
                    accum.opclass = opclass ;
                    % z = cast (1, opclass) ;
                    % opint = isinteger (z) || islogical (z) ;

                    % C = op (Cin2,A')
                    C = GB_mex_transpose  (Cin2, [ ], accum, A, [ ]) ;
                    assert (spok (C.matrix*1) == 1) ;
                    S = GB_spec_transpose (Cin2, [ ], accum, A, [ ]) ;
                    assert (isequal (C.class, cinclass)) ;
                    assert (isequal (C.class, S.class)) ;
                    if (~(isequalwithequalnans (full (double (C.matrix)), ...
                        double (S.matrix))))
                        assert (false)
                    end

                    % C = op (Cin,A)
                    clear D
                    D = struct ('inp0', 'tran') ;
                    C = GB_mex_transpose  (Cin, [ ], accum, A, D) ;
                    assert (spok (C.matrix*1) == 1) ;
                    S = GB_spec_transpose (Cin, [ ], accum, A, D) ;
                    assert (isequal (C.class, cinclass)) ;
                    assert (isequal (C.class, S.class)) ;
                    assert (isequalwithequalnans (full (double (C.matrix)), ...
                        double (S.matrix))) ;

                    % try with a Mask (Mask must be sparse; logical and double)
                    for k5 = [1 11]
                        mask_class = classes {k5} ;
                        M = cast (Mask, mask_class) ;

                        % C = op (Cin2,A')
                        C = GB_mex_transpose  (Cin2, M', accum, A, [ ]) ;
                        assert (spok (C.matrix*1) == 1) ;
                        S = GB_spec_transpose (Cin2, M', accum, A, [ ]) ;
                        assert (isequal (C.class, cinclass)) ;
                        assert (isequal (C.class, S.class)) ;
                        assert (isequalwithequalnans (...
                            full (double (C.matrix)), ...
                            double (S.matrix))) ;

                        % C = op (Cin,A)
                        clear D
                        D = struct ('inp0', 'tran') ;
                        C = GB_mex_transpose  (Cin, M, accum, A, D) ;
                        assert (spok (C.matrix*1) == 1) ;
                        S = GB_spec_transpose (Cin, M, accum, A, D) ;
                        assert (isequal (C.class, cinclass)) ;
                        assert (isequal (C.class, S.class)) ;
                        assert (isequalwithequalnans (...
                            full (double (C.matrix)), ...
                            double (S.matrix))) ;

                    end
                end
            end
        end
    end
end

fprintf ('\ntest22: all tests passed\n') ;


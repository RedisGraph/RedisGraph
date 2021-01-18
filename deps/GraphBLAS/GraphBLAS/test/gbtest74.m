function gbtest74
%GBTEST74 test bitwise operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

int_types = {
'int8'
'int16'
'int32'
'int64'
'uint8'
'uint16'
'uint32'
'uint64' } ;

int_nbits = [ 8, 16, 32, 64, 8, 16, 32, 64 ] ;

rng ('default') ;

for k = 1:8

    type = int_types {k} ;
    nbits = int_nbits (k) ;
    fprintf ('\n%s', type) ;

    for trial = 1:40
        fprintf ('.') ;

        % dense case

        imax = double (intmax (type) / 4) ;
        A = cast (imax * rand (4), type) ;
        B = cast ((nbits-1) * rand (4), type) + 1 ;
        A2 = GrB (A) ;
        B2 = GrB (B) ;
        V = rand (4) > 0.5 ;

        C1 = bitget (A, B) ;
        C2 = bitget (A2, B2) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B) ;
        C2 = bitset (A2, B2) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, 1) ;
        C2 = bitset (A2, B2, 1) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, 0) ;
        C2 = bitset (A2, B2, 0) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, V) ;
        C2 = bitset (A2, B2, V) ;
        assert (isequal (C1, C2)) ;

        for a = 0:3
            for b = 1:4
                C1 = bitset (a, b, 1) ;
                C2 = bitset (a, b, GrB (1)) ;
                assert (isequal (C1, C2)) ;
            end
        end

        C1 = bitset (a, B, 1) ;
        C2 = bitset (a, B, GrB (1)) ;
        assert (isequal (C1, C2)) ;

        C1 = bitand (A, B) ;
        C2 = bitand (A2, B2) ;
        assert (isequal (C1, C2)) ;

        C1 = bitor (A, B) ;
        C2 = bitor (A2, B2) ;
        assert (isequal (C1, C2)) ;

        C1 = bitxor (A, B) ;
        C2 = bitxor (A2, B2) ;
        assert (isequal (C1, C2)) ;

        C1 = bitcmp (A) ;
        C2 = bitcmp (A2) ;
        assert (isequal (C1, C2)) ;

        % dense double case, with assumedtype
        A = double (A) ;
        B = double (B) ;
        A2 = GrB (A) ;
        B2 = GrB (B) ;

        C1 = bitget (A, B, type) ;
        C2 = bitget (A2, B2, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, type) ;
        C2 = bitset (A2, B2, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, 1, type) ;
        C2 = bitset (A2, B2, 1, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, 0, type) ;
        C2 = bitset (A2, B2, 0, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitset (A, B, V, type) ;
        C2 = bitset (A2, B2, V, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitand (A, B, type) ;
        C2 = bitand (A2, B2, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitor (A, B, type) ;
        C2 = bitor (A2, B2, type) ;
        assert (isequal (C1, C2)) ;

        C1 = bitxor (A, B, type) ;
        C2 = bitxor (A2, B2, type) ;
        assert (isequal (C1, C2)) ;

        if (~ispc)
            % MATLAB R2019b on Windows has a bug here, so this
            % test is skipped.  Here is the pure MATLAB test:
            %
            %   ver
            %   A = 1287128410976072704
            %   fprintf ('A:         %30o\n', A) ;
            %   C = bitcmp (A, 'uint64')
            %   fprintf ('bitcmp(A): %30o\n', C) ;
            %
            % printing input and output in octal, with spaces
            % added for readability:
            %
            % A:                 0 107 346 307 414 442 532 000
            %
            % Linux bitcmp(A):   1 670 431 470 363 335 244 000
            % Win. bitcmp(A):    1 670 431 470 363 335 250 000
            %
            % GraphBLAS obtains the Linux result on all
            % platforms, including Windows.
            %
            % Expanding the last 5 octal digits into binary,
            % with a space where I think the double mantissa
            % runs about of bits when converted to uint64.
            %
            % A:                 32000 = 011.01 0.000.000.000
            %
            % Linux bitcmp(A):   44000 = 100.10 0.000.000.000
            % Win. bitcmp(A):    50000 = 101.00 0.000.000.000
            %
            % Note that A starts out as double, so it only
            % has about 53 bits of mantissa.  I would expect
            % the result to have 10 or 11 trailing zeros, as a
            % result.  On Linux (and also GraphBLAS on Windows),
            % bitcmp(A) has 10 trailing zero bits, and the
            % remaining bits are properly complemented.
            %
            % On MATLAB in Windows, the result is not comprehensible.

            C1 = bitcmp (A, type) ;
            C2 = bitcmp (A2, type) ;
            assert (isequal (C1, C2)) ;
        end

        C1 = bitshift (A, B, type) ;
        C2 = bitshift (A2, B2, type) ;
        assert (isequal (C1, C2)) ;

        % sparse case

        A = sprand (10, 10, 0.5) * imax ;
        Afull = cast (full (A), type) ;
        % B ranges in value from 0 to 8
        B = round (sprand (10, 10, 0.5) * nbits) ;
        Bfull = cast (full (B), type) ;
        A2 = GrB.prune (GrB (Afull)) ;
        B2 = GrB.prune (GrB (Bfull)) ;

        C1 = bitxor (Afull, Bfull) ;
        C2 = bitxor (A2, B2) ;
        assert (isequal (C1, full (C2))) ;

        C1 = bitand (Afull, Bfull) ;
        C2 = bitand (A2, B2) ;
        assert (isequal (C1, full (C2))) ;

        C1 = bitor (Afull, Bfull) ;
        C2 = bitor (A2, B2) ;
        assert (isequal (C1, full (C2))) ;

        C1 = bitshift (Afull, Bfull) ;
        C2 = bitshift (A2, B2) ;
        assert (isequal (C1, full (C2))) ;

        C1 = bitcmp (Afull) ;
        C2 = bitcmp (A2) ;
        assert (isequal (C1, full (C2))) ;

        % the MATLAB bitget and bitset cannot be used for B == 0,
        % so find where Bfull is explicitly zero.
        B_is_nonzero = (B ~= 0) ;
        A_ok = Afull (B_is_nonzero) ;
        B_ok = Bfull (B_is_nonzero) ;
        X1 = bitget (A_ok, B_ok) ;

        % the GraphBLAS bitget and bitset can tolerate B == 0 
        C2 = full (bitget (A2, B2)) ;
        X2 = C2 (B_is_nonzero) ;
        assert (isequal (X1, X2))

        X1 = bitset (A_ok, B_ok) ;
        C2 = full (bitset (A2, B2)) ;
        X2 = C2 (B_is_nonzero) ;
        assert (isequal (X1, X2))

        X1 = bitset (A_ok, B_ok, 0) ;
        C2 = full (bitset (A2, B2, 0)) ;
        X2 = C2 (B_is_nonzero) ;
        assert (isequal (X1, X2))

        % dense case with any A and B

        imax = double (intmax (type)) ;
        imin = double (intmin (type)) ;
        A1 = GrB ((imax-imin) * rand (4) - imin, type) ;
        B1 = GrB ((imax-imin) * rand (4) - imin, type) ;
        A  = cast (A1, type) ;
        B  = cast (B1, type) ;
        A2 = GrB.prune (GrB (A1)) ;
        B2 = GrB.prune (GrB (B1)) ;

        C1 = bitxor (A, B) ;
        C2 = bitxor (A1, B1) ;
        assert (isequal (C1, full (C2))) ;

        C1 = bitand (A, B) ;
        C2 = bitand (A2, B2) ;
        assert (isequal (C1, full (C2))) ;

        C1 = bitor (A, B) ;
        C2 = bitor (A2, B2) ;
        assert (isequal (C1, full (C2))) ;

        if (verLessThan ('matlab', '9.7') && isequal (type, 'int64'))
            % skip this test (older MATLAB versions have a bug in bitshift)
        else
            C1 = bitshift (A, B) ;
            C2 = bitshift (A2, B2) ;
            assert (isequal (C1, full (C2))) ;
        end

        C1 = bitcmp (A) ;
        C2 = bitcmp (A2) ;
        assert (isequal (C1, full (C2))) ;

    end
end

fprintf ('\ngbtest74: all tests passed\n') ;


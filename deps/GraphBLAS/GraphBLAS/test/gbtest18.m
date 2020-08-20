function gbtest18
%GBTEST18 test comparators (and, or, >, ...)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;
for trial = 1:21

    fprintf ('.') ;

    if (mod (trial, 10) == 1)
        m = 1 ;
        n = 1 ;
    else
        m = 4 ;
        n = 5 ;
    end

    MA = sprand (m,n, 0.5) ;    A (2,2) = 2 ; %#ok<*NASGU>
    MB = sprand (m,n, 0.5) ;    B (2,2) = 2 ;

    if (rand < 0.1)
        MA = logical (MA) ;
        MB = logical (MB) ;
    end

    GA = GrB (MA) ;
    GB = GrB (MB) ;

    C1 = (MA < MB) ;
    C2 = (GA < GB) ;
    C3 = (MA < GB) ;
    C4 = (GA < MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (MA <= MB) ;
    C2 = (GA <= GB) ;
    C3 = (MA <= GB) ;
    C4 = (GA <= MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (MA > MB) ;
    C2 = (GA > GB) ;
    C3 = (MA > GB) ;
    C4 = (GA > MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (MA >= MB) ;
    C2 = (GA >= GB) ;
    C3 = (MA >= GB) ;
    C4 = (GA >= MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (MA == MB) ;
    C2 = (GA == GB) ;
    C3 = (MA == GB) ;
    C4 = (GA == MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (MA ~= MB) ;
    C2 = (GA ~= GB) ;
    C3 = (MA ~= GB) ;
    C4 = (GA ~= MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    if (~islogical (MA))
        C1 = (MA .^ MB) ;
        C2 = (GA .^ GB) ;
        C3 = (MA .^ GB) ;
        C4 = (GA .^ MB) ;
        assert (gbtest_eq (C1, C2)) ;
        assert (gbtest_eq (C1, C3)) ;
        assert (gbtest_eq (C1, C4)) ;
    end

    C1 = (MA & MB) ;
    C2 = (GA & GB) ;
    C3 = (MA & GB) ;
    C4 = (GA & MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;
    if (islogical (MA))
        % C1 = min (MA , MB) ;
        C2 = min (GA , GB) ;
        C3 = min (MA , GB) ;
        C4 = min (GA , MB) ;
        assert (gbtest_eq (C1, C2)) ;
        assert (gbtest_eq (C1, C3)) ;
        assert (gbtest_eq (C1, C4)) ;
    end

    C1 = (MA | MB) ;
    C2 = (GA | GB) ;
    C3 = (MA | GB) ;
    C4 = (GA | MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;
    if (islogical (MA))
        % C1 = max (MA , MB) ;
        C2 = max (GA , GB) ;
        C3 = max (MA , GB) ;
        C4 = max (GA , MB) ;
        assert (gbtest_eq (C1, C2)) ;
        assert (gbtest_eq (C1, C3)) ;
        assert (gbtest_eq (C1, C4)) ;
    end

    C1 = xor (MA , MB) ;
    C2 = xor (GA , GB) ;
    C3 = xor (MA , GB) ;
    C4 = xor (GA , MB) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = ~MA ;
    C2 = ~GA ;
    assert (gbtest_eq (C1, C2)) ;

    b = (trial - 5) / 10 ;
    if (islogical (MA))
        b = logical (b) ;
    end
    g = GrB (b) ;

    assert (gbtest_eq (MA <  b, GA <  b)) ;
    assert (gbtest_eq (MA <= b, GA <= b)) ;
    assert (gbtest_eq (MA >  b, GA >  b)) ;
    assert (gbtest_eq (MA >= b, GA >= b)) ;
    assert (gbtest_eq (MA == b, GA == b)) ;
    assert (gbtest_eq (MA ~= b, GA ~= b)) ;
    if (~islogical (MA))
        assert (gbtest_eq (MA .^ b, GA .^ b)) ;
    end

    assert (gbtest_eq (MA <  b, GA <  g)) ;
    assert (gbtest_eq (MA <= b, GA <= g)) ;
    assert (gbtest_eq (MA >  b, GA >  g)) ;
    assert (gbtest_eq (MA >= b, GA >= g)) ;
    assert (gbtest_eq (MA == b, GA == g)) ;
    assert (gbtest_eq (MA ~= b, GA ~= g)) ;
    if (~islogical (MA))
        assert (gbtest_eq (MA .^ b, GA .^ g)) ;
    end

    assert (gbtest_eq (MA <  b, MA <  g)) ;
    assert (gbtest_eq (MA <= b, MA <= g)) ;
    assert (gbtest_eq (MA >  b, MA >  g)) ;
    assert (gbtest_eq (MA >= b, MA >= g)) ;
    assert (gbtest_eq (MA == b, MA == g)) ;
    assert (gbtest_eq (MA ~= b, MA ~= g)) ;
    if (~islogical (MA))
        assert (gbtest_eq (MA .^ b, MA .^ g)) ;
    end

    assert (gbtest_eq (b <  MA, b <  GA)) ;
    assert (gbtest_eq (b <= MA, b <= GA)) ;
    assert (gbtest_eq (b >  MA, b >  GA)) ;
    assert (gbtest_eq (b >= MA, b >= GA)) ;
    assert (gbtest_eq (b == MA, b == GA)) ;
    assert (gbtest_eq (b ~= MA, b ~= GA)) ;
    if (b >= 0 && ~islogical (MA))
        assert (gbtest_eq (b .^ MA, b .^ GA)) ;
    end

    assert (gbtest_eq (b <  MA, g <  GA)) ;
    assert (gbtest_eq (b <= MA, g <= GA)) ;
    assert (gbtest_eq (b >  MA, g >  GA)) ;
    assert (gbtest_eq (b >= MA, g >= GA)) ;
    assert (gbtest_eq (b == MA, g == GA)) ;
    assert (gbtest_eq (b ~= MA, g ~= GA)) ;
    if (b >= 0 && ~islogical (MA))
        assert (gbtest_eq (b .^ MA, g .^ GA)) ;
    end

    assert (gbtest_eq (b <  MA, g <  MA)) ;
    assert (gbtest_eq (b <= MA, g <= MA)) ;
    assert (gbtest_eq (b >  MA, g >  MA)) ;
    assert (gbtest_eq (b >= MA, g >= MA)) ;
    assert (gbtest_eq (b == MA, g == MA)) ;
    assert (gbtest_eq (b ~= MA, g ~= MA)) ;
    if (b >= 0 && ~islogical (MA))
        assert (gbtest_eq (b .^ MA, g .^ MA)) ;
    end

    k = (mod (trial,2) == 0) ;
    gbk = GrB (k) ;

    C1 = (MA & k) ;
    C2 = (GA & gbk) ;
    C3 = (MA & gbk) ;
    C4 = (GA & k) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;
    if (islogical (MA))
        % C1 = min (MA , k) ;
        C2 = min (GA , gbk) ;
        C3 = min (MA , gbk) ;
        C4 = min (GA , k) ;
        assert (gbtest_eq (C1, C2)) ;
        assert (gbtest_eq (C1, C3)) ;
        assert (gbtest_eq (C1, C4)) ;
    end

    C1 = (k   & MA) ;
    C2 = (gbk & GA) ;
    C3 = (gbk & MA) ;
    C4 = (k   & GA) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (MA | k) ;
    C2 = (GA | gbk) ;
    C3 = (MA | gbk) ;
    C4 = (GA | k) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = xor (MA, k) ;
    C2 = xor (GA, gbk) ;
    C3 = xor (MA, gbk) ;
    C4 = xor (GA, k) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = (k   | MA) ;
    C2 = (gbk | GA) ;
    C3 = (gbk | MA) ;
    C4 = (k   | GA) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    C1 = xor (k   , MA) ;
    C2 = xor (gbk , GA) ;
    C3 = xor (gbk , MA) ;
    C4 = xor (k   , GA) ;
    assert (gbtest_eq (C1, C2)) ;
    assert (gbtest_eq (C1, C3)) ;
    assert (gbtest_eq (C1, C4)) ;

    if (~islogical (MA))
        C1 = (k   .^ MA) ;
        C2 = (gbk .^ GA) ;
        C3 = (gbk .^ MA) ;
        C4 = (k   .^ GA) ;
        assert (gbtest_eq (C1, C2)) ;
        assert (gbtest_eq (C1, C3)) ;
        assert (gbtest_eq (C1, C4)) ;
    end

end

fprintf ('\ngbtest18: all tests passed\n') ;


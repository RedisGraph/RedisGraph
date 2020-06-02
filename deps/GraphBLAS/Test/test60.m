function test60
%TEST60 test min and max operators with NaNs

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('min\n') ;
for x = [3 nan inf]
    for y = [3 nan inf -inf]
        a = min (x, y);
        c = min (x, y, 'includenan');
        b = GB_mex_op ('min', x, y) ;
        fprintf ('x: %4g y: %4g  matlab(omit): %4g matlab(incl): %4g GrB %4g match: %d\n', ...
            x, y, a, c, b, isequalwithequalnans (a,b)) ;
    end
end

fprintf ('\nmax\n') ;
for x = [3 nan inf]
    for y = [3 nan inf -inf]
        a = max (x, y);
        c = max (x, y, 'includenan');
        b = GB_mex_op ('max', x, y) ;
        fprintf ('x: %4g y: %4g  matlab(omit): %4g matlab(incl): %4g GrB %4g match: %d\n', ...
            x, y, a, c, b, isequalwithequalnans (a,b)) ;
    end
end

fprintf ('\ntest60: all tests passed\n') ;


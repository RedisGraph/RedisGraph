function test60
%TEST60 test min and max operators with NaNs

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('min\n') ;
for x = [3 nan]
    for y = [3 nan]
        a = min (x, y);
        c = min (x, y, 'includenan');
        b = GB_mex_op ('min', x, y) ;
        fprintf ('x: %3g y: %3g  matlab: %3g %3g GrB %3g match: %d\n', ...
            x, y, a, c, b, isequalwithequalnans (a,b)) ;
    end
end

fprintf ('\nmax\n') ;
for x = [3 nan]
    for y = [3 nan]
        a = max (x, y);
        c = max (x, y, 'includenan');
        b = GB_mex_op ('max', x, y) ;
        fprintf ('x: %3g y: %3g  matlab: %3g %3g GrB %3g match: %d\n', ...
            x, y, a, c, b, isequalwithequalnans (a,b)) ;
    end
end

fprintf ('\ntest60: all tests passed\n') ;


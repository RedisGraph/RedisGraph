function test104
%TEST104 export/import

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng 'default'
fprintf ('\ntest104: export/import tests\n') ;

for m = [0 1 5 100]
    for n = [0 1 5 100]
        for d = [0 0.1 0.5 1]
            A = GB_spec_random (m, n, d) ;
            for format_matrix = 0:3
                for format_export = 0:3
                    C = GB_mex_export_import (A, format_matrix, format_export) ;
                    GB_spec_compare (C, A) ;
                end
            end
        end
    end
end

fprintf ('\ntest104: all tests passed\n') ;

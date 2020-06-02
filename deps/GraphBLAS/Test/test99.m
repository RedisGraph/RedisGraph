function test99
%TEST99 test GB_mex_transpose with explicit zeros in the Mask

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

fprintf ('test99: GB_mex_transpose with explicit zeros in the Mask\n') ;

for n = 10:20
    fprintf ('.') ;
    for d = 0:.1:1
        for k = 1:10

            C = sprand (n, n, 0.1) ;
            A = sprand (n, n, 0.1) ;
            if (d == 1)
                Mask = sparse (rand (n)) ;
            else
                Mask = sprand (n, n, d) ;
            end
            if (nnz (Mask) > 0)
                [i j x] = find (Mask) ;
                nz = length (x) ;
                p = randperm (nz, floor(nz/2)) ;
                x (p) = 0 ;
                i = uint64 (i-1) ;
                j = uint64 (j-1) ;
                Mask = GB_mex_Matrix_build (i,j,x,n,n,[]) ;
                Mask = Mask.matrix ;
            end

            for dkind = 1:4

                if (dkind == 1)
                    desc = struct ('outp', 'replace') ;
                elseif (dkind == 2)
                    desc = struct ('outp', 'replace', 'mask', 'scmp') ;
                elseif (dkind == 3)
                    desc = struct ('mask', 'scmp') ;
                else
                    desc = [ ] ;
                end
                desc.inp0 = 'tran' ;

                C2 = GB_spec_transpose (C, Mask, [], A, desc) ;
                C3 = GB_mex_transpose  (C, Mask, [], A, desc) ;
                GB_spec_compare (C2, C3) ;

                C2 = GB_spec_transpose (C, Mask, [], C, desc) ;
                C3 = GB_mex_transpose  (C, Mask, [], C, desc) ;
                GB_spec_compare (C2, C3) ;

            end
        end
    end
end

fprintf ('\ntest99: all tests passed\n') ;


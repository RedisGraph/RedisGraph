function test41
%TEST41 test AxB symbolic

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n -------------- simple GB_mex_AxB symbolic and numeric tests\n') ;

rng ('default') ;

for at = [false true]
    for bt = [false true]
        for ct = [false true]


                % create the problem

                % A or A' will be 4-by-5
                if (at)
                    A = sprand (5000, 4000, 0.01) ;
                else
                    A = sprand (4000, 5000, 0.01) ;
                end

                % B or B' will be 5-by-3
                if (bt)
                    B = sprand (3000, 5000, 0.01) ;
                else
                    B = sprand (5000, 3000, 0.01) ;
                end

                % C will be 4-by-3
                % C' will be 3-by-4

                fprintf ('\nat %d bt %d ct %d\n', at, bt, ct) ;

                fprintf ('matlab:  ') ;
                tic
                if (at)
                    if (bt)
                        if (ct)
                            C = (A'*B')' ;
                        else
                            C = (A'*B') ;
                        end
                    else
                        if (ct)
                            C = (A'*B)' ;
                        else
                            C = (A'*B) ;
                        end
                    end
                else
                    if (bt)
                        if (ct)
                            C = (A*B')' ;
                        else
                            C = (A*B') ;
                        end
                    else
                        if (ct)
                            C = (A*B)' ;
                        else
                            C = (A*B) ;
                        end
                    end
                end
                toc

                fprintf ('GrB num: ') ;
                tic
                S = GB_mex_AxB (A, B, at, bt) ;
                if (ct)
                    S = S' ;
                end
                toc
                assert (isequal (S, C)) ;

                E = spones (C) ;
                fprintf ('GrB sym: ') ;
                tic
                S = GB_mex_AxB_symbolic (A, B, at, bt, ct) ;
                toc

                assert (spok (S) == 1) ;
                assert (isequal (S, E)) ;

        end
    end
end

fprintf ('\ntest41: all tests passed\n') ;


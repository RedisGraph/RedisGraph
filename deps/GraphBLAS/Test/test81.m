function test81
%TEST81 test GrB_Matrix_extract with index range, stride, & backwards

rng ('default') ;

n = 4 ;
A = sprand (n, n, 0.5) ;
S = sparse (n, n) ;

for ilo = 1:n
    for ihi = 1:n
        fprintf ('.') ;
        for i_inc = [-n:n inf]
            clear I
            I.begin = ilo-1 ;
            I.end = ihi-1 ;
            if (isfinite (i_inc))
                I.inc = i_inc ;
                iinc = i_inc ;
            else
                iinc = 1 ;
            end

            for jlo = 1:n
                for jhi = 1:n
                    for j_inc = [-n:n inf]

                        clear J
                        J.begin = jlo-1 ;
                        J.end = jhi-1 ;
                        if (isfinite (j_inc))
                            J.inc = j_inc ;
                            jinc = j_inc ;
                        else
                            jinc = 1 ;
                        end

                        C1 = A (ilo:iinc:ihi, jlo:jinc:jhi) ;
                        [sm sn] = size (C1) ;
                        S = sparse (sm, sn) ;
                        C2 = GB_mex_Matrix_extract (S, [ ], [ ], A, I, J, [ ]) ;
                        assert (isequal (C1, C2.matrix)) ;

                    end
                end
            end
        end
    end
end

fprintf ('\ntest81: all tests passed\n') ;


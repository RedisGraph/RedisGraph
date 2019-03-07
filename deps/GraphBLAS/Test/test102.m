function test102
%TEST102 test GB_AxB_flopcount

fprintf ('\ntest102: testing GB_AxB_flopcount\n') ;

rng ('default') ;

for m = [0 1 10 100]
    for n = [0 1 10 100]
        for d = [0.01 0.1 0.5 1.0]

            % create the mask M and its hypersparse version
            M = sprand (m, n, d) ;
            Mhyper.matrix = M ;
            Mhyper.pattern = spones (M) ;
            Mhyper.is_hyper = true ;

            for k = [0 1 10 100]

                % create B and its hypersparse version
                B = sprand (k, n, d) ;
                Bhyper.matrix = B ;
                Bhyper.pattern = spones (B) ;
                Bhyper.is_hyper = true ;

                % create A and its hypersparse version
                A = sprand (m, k, d) ;
                Ahyper.matrix = A ;
                Ahyper.pattern = spones (A) ;
                Ahyper.is_hyper = true ;

                % flop counts for C=A*B
                mflops = flopcount ([ ], A, B) ;
                total = mflops (end) ;

                floptest ([ ], A, B, m, mflops) ;
                floptest ([ ], Ahyper, Bhyper, m, total) ;
                floptest ([ ], Ahyper, B, m, total) ;
                floptest ([ ], A, Bhyper, m, total) ;

                % flop counts for C<M>=A*B
                mflops = flopcount (M, A, B) ;
                total = mflops (end) ;

                floptest (M, A, B, m, mflops) ;
                floptest (M, A, Bhyper, m, total) ;
                floptest (M, Ahyper, B, m, total) ;
                floptest (M, Ahyper, Bhyper, m, total) ;
                floptest (Mhyper, A, B, m, total) ;
                floptest (Mhyper, Ahyper, B, m, total) ;
                floptest (Mhyper, A, Bhyper, m, total) ;
                floptest (Mhyper, A, B, m, total) ;

            end
        end
    end
end

fprintf ('\ntest102: all tests passed\n') ;


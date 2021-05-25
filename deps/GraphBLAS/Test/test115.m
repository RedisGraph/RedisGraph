function test115
%TEST115 test GB_assign, scalar expansion and zombies, with duplicates

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

rng ('default') ;

    n = 200 ;
    C = sparse (rand (n)) ;
    C (:,1) = sparse (rand (n,1) > 0.5) ;

    ntrials = 10 ;

    for k = 1:2:ntrials

        % make some zombies
        Work (k).A = sprandn (n, n, 0.1) ;
        Work (k).I = [ ] ;
        Work (k).J = [ ] ;
        Work (k).Mask = sparse (ones (n)) ;
        Work (k).accum = [ ] ;
        Work (k).desc = [ ] ;
        Work (k).scalar = 0 ;

        % scalar expansion
        Work (k+1).A = sparse (pi) ;
        Work (k+1).I = uint64 (irand (1, n, n, 1) - 1) ;
        Work (k+1).J = uint64 (irand (1, n, n, 1) - 1) ;
        Work (k+1).Mask = sparse (ones (n)) ;
        Work (k+1).accum = [ ] ;
        Work (k+1).desc = [ ] ;
        Work (k+1).scalar = 1 ;
    end

    C1 = GB_mex_assign  (C, Work) ;

    C2 = C ;
    for k = 1:ntrials
        M = Work (k).Mask ;
        A = Work (k).A ;
        I = double (Work (k).I + 1) ;
        J = double (Work (k).J + 1) ;
        scalar = Work (k).scalar ;
        C2 = GB_spec_subassign (C2, M, [ ], A, I, J, [ ], scalar) ;
    end

    GB_spec_compare (C1, C2) ;

    for C_sparsity = [2 4]
        for M_sparsity = [2 4]
            C1 = GB_mex_assign (C, Work, [C_sparsity M_sparsity]) ;
            GB_spec_compare (C1, C2) ;
        end
    end

fprintf ('\ntest115: all tests passed\n') ;


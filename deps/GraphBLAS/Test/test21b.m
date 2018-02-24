function test21b
%TEST21B test GrB_assign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n--------------exhaustive test of GB_mex_assign\n') ;

[accum_ops unary_ops add_ops classes] = GB_spec_opsall ;

dn = struct ;
dt = struct ( 'inp0', 'tran' ) ;

% try all accum
for k1 = 0:length(accum_ops)
    if (k1 == 0)
        accum_op = ''  ;
        nclasses = 1 ;
    else
        accum_op = accum_ops {k1}  ;
        nclasses = length (classes) ;
    end
    fprintf ('\naccum: [%s]', accum_op) ;

    % try all classes
    for k2 = 1:nclasses
        clear accum
        if (~isempty (accum_op))
            accum_class = classes {k2}  ;
            accum.opname = accum_op ;
            accum.opclass = accum_class ;
        else
            accum = '' ;
            accum_class = '' ;
        end

        rng (k1 * 100 + k2, 'v4') ;

        for Mask_complement = [false true]

            if (Mask_complement)
                dn.mask = 'scmp' ;
                dt.mask = 'scmp' ;
            else
                dn.mask = 'default' ;
                dt.mask = 'default' ;
            end

            for C_replace = [false true]

                if (C_replace)
                    dn.outp = 'replace' ;
                    dt.outp = 'replace' ;
                else
                    dn.outp = 'default' ;
                    dt.outp = 'default' ;
                end

                kk3 = randperm (length (classes), 1) ;

                % try all matrix classes, to test casting
                for k3 = kk3 % 1:length (classes)
                    aclas = classes {k3}  ;
                    fprintf ('.') ;

                    % try some matrices
                    for m = [1 5 10 ]
                        for n = [ 1 5 10 ]
                            for sm = [ 0 1 5 10 ]
                                if (sm > m)
                                    continue
                                end
                                for sn = [ 0 1 5 10 ]
                                    if (sn > n)
                                        continue
                                    end
                                    for scalar = [false true]

                                        %---------------------------------------


                                        if (sm == 0)
                                            I = [ ] ;
                                            am = m ;
                                        else
                                            I = randperm (m,sm) ; % I = I(1:sm);
                                            am = sm ;
                                        end
                                        I0 = uint64 (I-1) ;
                                        if (sn == 0)
                                            J = [ ] ;
                                            an = n ;
                                        else
                                            J = randperm (n,sn) ; % J = J(1:sn);
                                            an = sn ;
                                        end
                                        J0 = uint64 (J-1) ;

                                        if (scalar)
                                            % test scalar expansion
                                            % fprintf ('test expansion\n') ;
                                            A.matrix = sparse (rand (1)) * 100 ;
                                            A.pattern = sparse (logical (true));
                                            A.class = aclas ;
                                        else
                                            A = GB_spec_random (am,an,0.2,100,aclas) ;
                                        end

                                        C = GB_spec_random (m,n,0.2,100,aclas) ;
                                        Mask = sprandn (m,n,0.2) ~= 0 ;

                                        % C(I,J) = accum (C (I,J),A)
                                        % Mask = [ ] ;

                                        C0 = GB_spec_assign (C, [ ], accum, A, I, J, dn, scalar);
                                        C1 = GB_mex_assign  (C, [ ], accum, A, I0, J0, dn);
                                        GB_spec_compare (C0, C1) ;

                                        % C(I,J)<Mask> = accum (C (I,J),A)
                                        C0 = GB_spec_assign (C, Mask, accum, A, I, J, dn, scalar);
                                        C1 = GB_mex_assign  (C, Mask, accum, A, I0, J0, dn);
                                        GB_spec_compare (C0, C1) ;

                                        %---------------------------------------

                                        % C(I,J)<Mask> = accum (C(I,J), A');
                                        % note transposing twice
                                        clear AT
                                        AT.matrix  = A.matrix' ;
                                        AT.pattern = A.pattern' ;
                                        AT.class = A.class ;

                                        C0 = GB_spec_assign (C, [ ], accum, AT, I, J, dt, scalar);
                                        C1 = GB_mex_assign  (C, [ ], accum, AT, I0, J0, dt);
                                        GB_spec_compare (C0, C1) ;

                                        C0 = GB_spec_assign (C, Mask, accum, AT, I, J, dt, scalar);
                                        C1 = GB_mex_assign  (C, Mask, accum, AT, I0, J0, dt);
                                        GB_spec_compare (C0, C1) ;

                                        %---------------------------------------

                                    end
                                end
                            end
                        end
                    end
                end
            end
        end
    end
end

fprintf ('\ntest21b: all tests passed\n') ;



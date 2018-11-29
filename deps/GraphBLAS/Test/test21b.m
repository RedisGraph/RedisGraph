function test21b (fulltest)
%TEST21B test GrB_assign

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin < 1)
    % do a short test, by default
    fulltest = 0 ;
end

[accum_ops unary_ops add_ops classes] = GB_spec_opsall ;

dn = struct ;
dt = struct ( 'inp0', 'tran' ) ;

if (fulltest)
    fprintf ('\n--------------exhaustive test of GB_mex_subassign\n') ;
    k1test = 0:length(accum_ops) ;
else
    fprintf ('\n--------------quick test of GB_mex_subassign\n') ;
    k1test = [0 4] ; % Was [0 2 4] ;
end

quick = 0 ;

% try all accum
for k1 = k1test
    if (k1 == 0)
        accum_op = ''  ;
        nclasses = 1 ;
    else
        accum_op = accum_ops {k1}  ;
        nclasses = length (classes) ;
    end
    fprintf ('\naccum: [%s]', accum_op) ;

    if (fulltest)
        k2test = 1:nclasses ;
    else
        k2test = [1 11] ; % Was [1 2 11] ;
    end

    % try all classes
    for k2 = k2test % 1:nclasses
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

            for C_replace = [true false]

                if (C_replace)
                    % fprintf ('C_replace') ;
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

                    % try some matrices
                    for m = [1 5 10 ]
                        for n = [ 1 5 10 ]
                            for sm = [ -3 -2 -1 0 1 5 10 ]
                                if (sm > m)
                                    continue
                                end

                                fprintf ('.') ;

                                for sn = [ 0 1 5 10 ]
                                    if (sn > n)
                                        continue
                                    end
                                    for scalar = [false true]

                                        %---------------------------------------

                                        if (sm == -3)

                                            % I = (m-2):-2:1
                                            if (m < 5)
                                                continue
                                            end
                                            clear I0
                                            I0.begin = m-3 ;
                                            I0.inc = -2 ;
                                            I0.end = 0 ;
                                            I = (m-2):-2:1 ;
                                            am = length (I) ;

                                        elseif (sm == -2)

                                            % I = 1:2:(m-2)
                                            if (m < 5)
                                                continue
                                            end
                                            clear I0
                                            I0.begin = 0 ;
                                            I0.inc = 2 ;
                                            I0.end = m-3 ;
                                            I = 1:2:(m-2) ;
                                            am = length (I) ;

                                        elseif (sm == -1)

                                            % I = 1:(m-2)
                                            if (m < 5)
                                                continue
                                            end
                                            clear I0
                                            I0.begin = 0 ;
                                            I0.end = m-3 ;
                                            I = 1:(m-2) ;
                                            am = length (I) ;

                                        elseif (sm == 0)

                                            % I = ":"
                                            I = [ ] ;
                                            am = m ;
                                            I0 = uint64 (I-1) ;

                                        else

                                            % I = random list of length sm
                                            I = randperm (m,sm) ;
                                            am = sm ;
                                            I0 = uint64 (I-1) ;

                                        end

                                        if (sn == 0)
                                            J = [ ] ;
                                            an = n ;
                                        else
                                            J = randperm (n,sn) ; % J = J(1:sn);
                                            an = sn ;
                                        end
                                        J0 = uint64 (J-1) ;

                                        for A_is_hyper = 0:1
                                        for A_is_csc   = 0:1
                                        for C_is_hyper = 0:1
                                        for C_is_csc   = 0:1
                                        for M_is_hyper = 0:1
                                        for M_is_csc   = 0:1

                                        quick = quick+1 ;
                                        if (~fulltest)
                                            % only do every 11th test
                                            if (mod (quick, 11) ~= 1)
                                                continue
                                            end
                                        end

                                        if (scalar)
                                            % test scalar expansion
                                            % fprintf ('test expansion\n') ;
                                            A.matrix = sparse (rand (1)) * 100 ;
                                            A.pattern = sparse (logical (true));
                                            A.class = aclas ;
                                            if (A_is_hyper || ~A_is_csc)
                                                continue
                                            end
                                        else
                                            A = GB_spec_random (am,an,0.2,100,aclas, A_is_csc, A_is_hyper) ;
                                        end

                                        C = GB_spec_random (m,n,0.2,100,aclas, C_is_csc, C_is_hyper) ;
                                        Mask = GB_random_mask (m,n,0.2, M_is_csc, M_is_hyper) ;

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
                    end
                end
            end
        end
    end
end

fprintf ('\ntest21b: all tests passed\n') ;



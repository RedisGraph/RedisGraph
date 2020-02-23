function test62
%TEST62 test GrB_apply

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n ------------ testing GrB_apply\n') ;

rng ('default')

[accum_ops, unary_ops, ~, classes, ~, ~] = GB_spec_opsall ;

dt = struct ('inp0', 'tran') ;

% class of the matrix C
for k1 = 1:length (classes)
    cclass = classes {k1}  ;
    fprintf ('\n%s', cclass) ;

    % class of the matrix A
    for k2 = 1:length (classes)
        aclass = classes {k2}  ;

        % create a matrix
        for m = [1 10 25]
            for n = [1 10 25]
                fprintf ('.') ;
                clear A
                A.matrix = sprandn (m, n, 0.1) ;
                A.class = aclass ;

                Mask = (sprandn (m, n, 0.1) ~= 0) ;
                MaskT = Mask' ;

                clear B
                B.matrix = sprandn (m*n, 1, 0.1) ;
                B.class = aclass ;

                mask = (sprandn (m*n, 1, 0.1) ~= 0) ;

                clear Cin
                Cin.matrix = sprandn (m, n, 0.1) ;
                Cin.class = cclass ;

                clear CinT
                CinT.matrix = Cin.matrix' ;
                CinT.class = cclass ;

                clear Cin2
                Cin2.matrix = sprandn (m*n, 1, 0.1) ;
                Cin2.class = cclass ;


                % unary operator
                for k3 = 1:length(unary_ops)
                    unary_op = unary_ops {k3}  ;
                    nclasses = 1;length (classes) ;
                    % fprintf ('unary: %s\n', unary_op) ;
                    % unary operator class
                    for k4 = nclasses
                        clear unary
                        if (~isempty (unary_op))
                            unary_class = classes {k4}  ;
                            unary.opname = unary_op ;
                            unary.opclass = unary_class ;
                        else
                            unary = '' ;
                            unary_class = '' ;
                        end

                        % accum operator
                        for k5 = 0:length(accum_ops)
                            if (k5 == 0)
                                accum_op = ''  ;
                                nclasses = 1 ;
                            else
                                accum_op = accum_ops {k5}  ;
                                nclasses = 1;length (classes) ;
                            end
                            % accum operator class
                            for k6 = nclasses
                                clear accum
                                if (~isempty (accum_op))
                                    accum_class = classes {k6}  ;
                                    accum.opname = accum_op ;
                                    accum.opclass = accum_class ;
                                else
                                    accum = '' ;
                                    accum_class = '' ;
                                end

                                % apply to A, no mask
                                C1 = GB_mex_apply  (Cin, [ ], accum, unary, A, [ ]) ;
                                C2 = GB_spec_apply (Cin, [ ], accum, unary, A, [ ]) ;
                                GB_spec_compare (C1, C2) ;

                                % apply to A', no mask
                                C2 = GB_spec_apply (CinT, [ ], accum, unary, A, dt) ;
                                C1 = GB_mex_apply  (CinT, [ ], accum, unary, A, dt) ;
                                GB_spec_compare (C1, C2) ;

                                % apply to vector B, no mask
                                C1 = GB_mex_apply  (Cin2, [ ], accum, unary, B, [ ]) ;
                                C2 = GB_spec_apply (Cin2, [ ], accum, unary, B, [ ]) ;
                                GB_spec_compare (C1, C2) ;

                                % apply to A, with mask
                                C1 = GB_mex_apply  (Cin, Mask, accum, unary, A, [ ]) ;
                                C2 = GB_spec_apply (Cin, Mask, accum, unary, A, [ ]) ;
                                GB_spec_compare (C1, C2) ;

                                % apply to A', with mask
                                C1 = GB_mex_apply  (CinT, MaskT, accum, unary, A, dt) ;
                                C2 = GB_spec_apply (CinT, MaskT, accum, unary, A, dt) ;
                                GB_spec_compare (C1, C2) ;

                                % apply to vector B, with mask
                                C1 = GB_mex_apply  (Cin2, mask, accum, unary, B, [ ]) ;
                                C2 = GB_spec_apply (Cin2, mask, accum, unary, B, [ ]) ;
                                GB_spec_compare (C1, C2) ;

                            end
                        end
                    end
                end
            end
        end
    end
end

fprintf ('\ntest62: all tests passed\n') ;


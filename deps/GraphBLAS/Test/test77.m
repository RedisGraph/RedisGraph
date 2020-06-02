function test77 (fulltest)
%TEST77 test GxB_kron

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[bin_ops, ~, ~, classes, ~, ~] = GB_spec_opsall ;

if (nargin < 1)
    fulltest = 0 ;
end

if (fulltest)
    fprintf ('--------------lengthy tests of GxB_kron\n') ;
    k1test = 1:length(classes) ;
else
    fprintf ('--------------quick tests of GxB_kron\n') ;
    k1test = 11 ; % Was [1 2 4 10 11] ;
end

rng ('default') ;

dnn = struct ;
dtn = struct ( 'inp0', 'tran' ) ;
dnt = struct ( 'inp1', 'tran' ) ;
dtt = struct ( 'inp0', 'tran', 'inp1', 'tran' ) ;

n_semirings = 0 ;
for k1 = k1test % 1:length (classes)
    clas = classes {k1}  ;

    fprintf ('\n%s:\n', clas) ;

    if (fulltest)
        k2test = 1:length(bin_ops) ;
    else
        k2test = randperm (length(bin_ops), 1) ; % Was 2
    end

    for k2 = k2test % 1:length(bin_ops)
        binop = bin_ops {k2}  ;

        fprintf (' %s', binop) ;

        op.opname = binop ;
        op.opclass = clas ;
        fprintf (' binary op: [ %s %s ] ', binop, clas) ;

        for k4 = randi([0,length(bin_ops)]) % 0:length(bin_ops)

            clear accum
            if (k4 == 0)
                accum = ''  ;
                nclasses = 1 ;
                fprintf ('accum: [ none ]') ;
            else
                accum.opname = bin_ops {k4}  ;
                nclasses = length (classes) ;
                fprintf ('accum: %s ', accum.opname) ;
            end

            for k5 = randi ([1 nclasses]) % nclasses

                if (k4 > 0)
                    accum.opclass = classes {k5}  ;
                    fprintf ('%s\n', accum.opclass) ;
                else
                    fprintf ('\n') ;
                end

                for Mask_complement = [false true]

                    if (Mask_complement)
                        dnn.mask = 'scmp' ;
                        dtn.mask = 'scmp' ;
                        dnt.mask = 'scmp' ;
                        dtt.mask = 'scmp' ;
                    else
                        dnn.mask = 'default' ;
                        dtn.mask = 'default' ;
                        dnt.mask = 'default' ;
                        dtt.mask = 'default' ;
                    end

                    for C_replace = [false true]

                        if (C_replace)
                            dnn.outp = 'replace' ;
                            dtn.outp = 'replace' ;
                            dnt.outp = 'replace' ;
                            dtt.outp = 'replace' ;
                        else
                            dnn.outp = 'default' ;
                            dtn.outp = 'default' ;
                            dnt.outp = 'default' ;
                            dtt.outp = 'default' ;
                        end

                        % try some matrices
                        for am = 5 %  % Was [1 5 10 ]
                            for an = 3 % [1 10 ] % Was [ 1 5 10 ]
                                for bm = 4 %  % Was [1 4 9 ]
                                    for bn = 2 %  % Was [1 4 9 ]
                                        fprintf ('.') ;

                                        Ax= sparse (100 * sprandn (am,an, 0.5));
                                        Bx= sparse (100 * sprandn (bm,bn, 0.5));
                                        cm = am * bm ;
                                        cn = an * bn ;
                                        Cx= sparse (100 * sprandn (cm,cn, 0.2));
                                        Mask = sprandn (cm,cn,0.2) ~= 0 ;
                                        AT = Ax' ;
                                        BT = Bx' ;

                                        for A_is_hyper = 0:1
                                        for A_is_csc   = 0:1
                                        for B_is_hyper = 0:1
                                        for B_is_csc   = 0:1
                                        for C_is_hyper = 0:1
                                        for C_is_csc   = 0:1

                                        clear A
                                        A.matrix = Ax ;
                                        A.is_hyper = A_is_hyper ;
                                        A.is_csc   = A_is_csc   ;

                                        clear B
                                        B.matrix = Bx ;
                                        B.is_hyper = B_is_hyper ;
                                        B.is_csc   = B_is_csc   ;

                                        clear C
                                        C.matrix = Cx ;
                                        C.is_hyper = C_is_hyper ;
                                        C.is_csc   = C_is_csc   ;

                                        %---------------------------------------
                                        % kron(A,B)
                                        %---------------------------------------

                                        % C = kron(A,B)
                                        C0 = GB_spec_kron  ...
                                            (C, [ ], accum, op, A, B, dnn);
                                        C1 = GB_mex_kron  ...
                                            (C, [ ], accum, op, A, B, dnn);
                                        GB_spec_compare (C0, C1) ;

                                        % C = kron(A,B) with Mask
                                        C0 = GB_spec_kron ...
                                            (C, Mask, accum, op, A, B, dnn);
                                        C1 = GB_mex_kron ...
                                            (C, Mask, accum, op, A, B, dnn);
                                        GB_spec_compare (C0, C1) ;

                                        %---------------------------------------
                                        % kron(A',B)
                                        %---------------------------------------

                                        % C = kron(A',B), no Mask
                                        C0 = GB_spec_kron ...
                                            (C, [ ], accum, op, AT, B, dtn);
                                        C1 = GB_mex_kron ...
                                            (C, [ ], accum, op, AT, B, dtn);
                                        GB_spec_compare (C0, C1) ;

                                        % C = kron(A',B), with Mask
                                        C0 = GB_spec_kron ...
                                            (C, Mask, accum, op, AT, B, dtn);
                                        C1 = GB_mex_kron ...
                                            (C, Mask, accum, op, AT, B, dtn);
                                        GB_spec_compare (C0, C1) ;

                                        %---------------------------------------
                                        % kron(A,B')
                                        %---------------------------------------

                                        % no mask
                                        C0 = GB_spec_kron ...
                                            (C, [ ], accum, op, A, BT, dnt);
                                        C1 = GB_mex_kron ...
                                            (C, [ ], accum, op, A, BT, dnt);
                                        GB_spec_compare (C0, C1) ;

                                        % with mask
                                        C0 = GB_spec_kron ...
                                            (C, Mask, accum, op, A, BT, dnt);
                                        C1 = GB_mex_kron ...
                                            (C, Mask, accum, op, A, BT, dnt);
                                        GB_spec_compare (C0, C1) ;

                                        %---------------------------------------
                                        % kron(A',B')
                                        %---------------------------------------

                                        % no Mask
                                        C0 = GB_spec_kron ...
                                            (C, [ ], accum, op, AT, BT, dtt);
                                        C1 = GB_mex_kron ...
                                            (C, [ ], accum, op, AT, BT, dtt);
                                        GB_spec_compare (C0, C1) ;

                                        % with mask
                                        C0 = GB_spec_kron  ...
                                            (C, Mask, accum, op, AT, BT, dtt);
                                        C1 = GB_mex_kron ...
                                            (C, Mask, accum, op, AT, BT, dtt);
                                        GB_spec_compare (C0, C1) ;

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

fprintf ('\ntest77: all tests passed\n') ;


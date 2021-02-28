function test114
%TEST114 performance of reduce-to-scalar

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

rng ('default') ;

[save save_chunk] = nthreads_get ;
chunk = 4096 ;
nthreads_list = [1 2 4 8 16 2 40 64 160] ;
nthreads_max = GB_mex_omp_max_threads ;
ntrials = 10 ;

%-------------------------------------------------------------------------------
% big matrix ...
fprintf ('\nbig matrix, no early exit\n') ;
n = 8000 ;
A = sparse (ones (n)) ;

tic
for trial = 1:ntrials
    s = full (max (max (A))) ;
end
tm = toc ;
fprintf ('MATLAB max: %g\n', tm) ;

tic
for trial = 1:ntrials
    s = full (min (min (A))) ;
end
tm = toc ;
fprintf ('MATLAB min: %g\n', tm) ;

tic
for trial = 1:ntrials
    s = full (sum (sum (A))) ;
end
tm = toc ;
fprintf ('MATLAB sum: %g\n', tm) ;

tic
for trial = 1:ntrials
    s = full (prod (prod (A))) ;
end
tm = toc ;
fprintf ('MATLAB prod: %g\n', tm) ;

S.matrix = A ;
S.pattern = logical (spones (A)) ;

[mult_ops unary_ops add_ops classes semirings selops] = GB_spec_opsall ;

ops = { 'or', 'and', 'xor', 'eq' } ;
for k1 = 1:4
    op = ops {k1} ;
    fprintf ('\nGraphBLAS: op %s\n', op) ;
    S.class = 'logical' ;
    cin = logical (0) ;
    for nthreads = nthreads_list
        if (nthreads > nthreads_max)
            break ;
        end
        nthreads_set (nthreads,chunk) ;
        t = 0 ;
        tic
        for trial = 1:ntrials
            c1 = GB_mex_reduce_to_scalar (cin, [ ], op, S) ;
            t = t + grbresults ;
        end
        if (nthreads == 1)
            t1 = t ;
        end
        fprintf ('nthreads %3d %12.4f  speedup %12.4f\n', ...
            nthreads, t, t1/t) ;
    end
end

ops = { 'min', 'max', 'plus', 'times' } ;
for k1 = 1:4
    op = ops {k1} ;
    fprintf ('\nGraphBLAS: op %s\n', op) ;
    for k2 = 2:11
        aclass = classes {k2} ;
        S.class = aclass ;
        fprintf ('\ntype: %s\n', aclass) ;
        switch aclass
            case 'logical'
                cin = logical (0) ;
            case 'int8'          % GrB_INT8
                cin = int8 (0) ;
            case 'uint8'         % GrB_UINT8
                cin = uint8 (0) ;
            case 'int16'         % GrB_INT16
                cin = int16 (0) ;
            case 'uint16'        % GrB_UINT16
                cin = uint16 (0) ;
            case 'int32'         % GrB_INT32
                cin = int32 (0) ;
            case 'uint32'        % GrB_UINT32
                cin = uint32 (0) ;
            case 'int64'         % GrB_INT64
                cin = int64 (0) ;
            case 'uint64'        % GrB_UINT64
                cin = uint64 (0) ;
            case 'single'        % GrB_FP32
                cin = single (0) ;
            case 'double'        % GrB_FP64
                cin = double (0) ;
        end
        for nthreads = nthreads_list
            if (nthreads > nthreads_max)
                break ;
            end
            nthreads_set (nthreads,chunk) ;
            t = 0 ;
            tic
            for trial = 1:ntrials
                c1 = GB_mex_reduce_to_scalar (cin, [ ], op, S) ;
                t = t + grbresults ;
            end
            if (nthreads == 1)
                t1 = t ;
            end
            fprintf ('nthreads %3d %12.4f  speedup %12.4f\n', ...
                nthreads, t, t1/t) ;
        end
    end
end

nthreads_set (save, save_chunk) ;

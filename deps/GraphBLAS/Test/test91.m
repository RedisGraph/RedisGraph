function test91
%TEST91 test subref performance on dense vectors

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

fprintf ('\n------------------------------ testing GB_mex_Matrix_subref\n') ;

ntrials = 10 ;

addpath old
rng ('default')

n = 10 * 1e6 ;
A = sparse (rand (n,1)) ;
fprintf ('A is a sparse %d-by-1 vector, all nonzero\n', n) ;
F = full (A) ;


for ilen = [1 10 100 1000 10000 100000 1e6]

    fprintf ('\n----- C(I) = A (I), I is random with length(I) = %d\n', ilen) ;

    I  = irand (1, n, ilen, 1) ;
    I0 = uint64 (I-1) ;

    fprintf ('MATLAB sparse:\n') ;
    tic
    for trials = 1:ntrials
        C1 = A (I) ;
    end
    toc

    fprintf ('MATLAB full:\n') ;
    tic
    for trials = 1:ntrials
        Cfull = F (I) ;
    end
    toc

    J0 = uint64 (0) ;

%   fprintf ('GB old:\n') ;
%   tic
%   for trials = 1:ntrials
%       C2 = GB_mex_Matrix_subref_old (A, I0, J0) ;
%   end
%   toc
%   assert (isequal (C1, C2)) ;

    fprintf ('GB new:\n') ;
    tic
    for trials = 1:ntrials
        C3 = GB_mex_Matrix_subref (A, I0, J0) ;
    end
    toc
    assert (isequal (C1, C3)) ;

end

fprintf ('\n----- C(:) = A (:)\n') ;

fprintf ('MATLAB:\n') ;
tic
for trials = 1:ntrials
    C1 = A (:) ;
end
toc

% fprintf ('GB old:\n') ;
% tic
% for trials = 1:ntrials
%     C2 = GB_mex_Matrix_subref_old (A, [ ], J0) ;
% end
% toc
% assert (isequal (C1, C2)) ;

fprintf ('GB new:\n') ;
tic
for trials = 1:ntrials
    C3 = GB_mex_Matrix_subref (A, [ ], J0) ;
end
toc
assert (isequal (C1, C3)) ;

F = full (A) ;

fprintf ('\nMATLAB (full):\n') ;
tic
for trials = 1:ntrials
    C0 = F (:) ;
    C0 (1) = 1 ;    % make sure the copy gets done, not a lazy copy
end
toc

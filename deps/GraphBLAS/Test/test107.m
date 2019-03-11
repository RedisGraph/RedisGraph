function test107
%TEST107 user-defined terminal monoid

rng ('default') ;

% clear all
% delete 'GB_mex_reduce_terminal.mex*'
% make

% create a matrix with entries [0..2]
A = 2 * sparse (rand (4)) ;
s = full (max (max (A))) ;

c = GB_mex_reduce_terminal (A, 2) ;
assert (c == s) ;

% now add the terminal value somewhere
A (1,2) = 2 ;
s = full (max (max (A))) ;
c = GB_mex_reduce_terminal (A, 2) ;
assert (c == s) ;
clear A

ntrials = 10 ;

%-------------------------------------------------------------------------------
% big matrix ...
fprintf ('\nbig matrix, no early exit\n') ;
n = 6000 ;
A = sparse (rand (n)) ;

tic
for trial = 1:ntrials
    s = full (max (max (A))) ;      % fastest
end
toc

tic
for trial = 1:ntrials
    c1 = GB_mex_reduce_to_scalar (0, [ ], 'max', A) ;
end
toc

% FUTURE: faster GrB_reduce for pre-compilled user-defined objects

tic
for trial = 1:ntrials
    c2 = GB_mex_reduce_terminal (A, 1) ;
end
toc

tic
for trial = 1:ntrials
    c3 = GB_mex_reduce_terminal (A, 2) ;
end
toc

assert (s == c1) ;
assert (s == c2) ;
assert (s == c3) ;

%-------------------------------------------------------------------------------
fprintf ('\nbig matrix, with early exit\n') ;

A (n,1) = 1 ;

tic
for trial = 1:ntrials
    s = full (max (max (A))) ;
end
toc

tic
for trial = 1:ntrials
    c1 = GB_mex_reduce_to_scalar (0, [ ], 'max', A) ;
end
toc

tic
for trial = 1:ntrials
    c2 = GB_mex_reduce_terminal (A, 1) ;  % fastest
end
toc

assert (s == c1) ;
assert (s == c2) ;

%-------------------------------------------------------------------------------
fprintf ('\nbig matrix, with inf \n') ;

A (n,1) = inf ;

tic
for trial = 1:ntrials
    s = full (max (max (A))) ;
end
toc

tic
for trial = 1:ntrials
    c1 = GB_mex_reduce_to_scalar (0, [ ], 'max', A) ;   % fastest
end
toc

tic
for trial = 1:ntrials
    c2 = GB_mex_reduce_terminal (A, inf) ;              % fastest
end
toc

assert (s == c1) ;
assert (s == c2) ;


%-------------------------------------------------------------------------------
fprintf ('\nbig matrix, with 2 \n') ;

A (n,1) = 2 ;

tic
for trial = 1:ntrials
    s = full (max (max (A))) ;
end
toc

tic
for trial = 1:ntrials
    c1 = GB_mex_reduce_to_scalar (0, [ ], 'max', A) ;
end
toc

tic
for trial = 1:ntrials
    c2 = GB_mex_reduce_terminal (A, 2) ;                % fastest
end
toc

assert (s == c1) ;
assert (s == c2) ;

%-------------------------------------------------------------------------------
fprintf ('\nbig matrix, with nan\n') ;

A (n,1) = nan ;

tic
for trial = 1:ntrials
    s = full (max (max (A))) ;
end
toc

tic
for trial = 1:ntrials
    c1 = GB_mex_reduce_to_scalar (0, [ ], 'max', A) ;
end
toc

assert (s == c1) ;

%-------------------------------------------------------------------------------
fprintf ('\nsum\n') ;

A (n,1) = 1 ;

tic
for trial = 1:ntrials
    ss = full (sum (sum (A))) ;
end
toc

tic
for trial = 1:ntrials
    cc = GB_mex_reduce_to_scalar (0, [ ], 'plus', A) ;
end
toc

err = abs (ss - cc) / ss 
assert (err < 1e-12) ;

fprintf ('test107: all tests passed\n') ;


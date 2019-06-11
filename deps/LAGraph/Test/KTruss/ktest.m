% Test ktruss.m.  Requires ssget.

clear
index = ssget ;
f = find (index.pattern_symmetry == 1) ;
[~,i] = sort (index.nnz (f)) ;
f = f (i) ;

% f = [739 750 2662];
% f = [168 739 750] ; 
% f = [168] 
f = [ 262 2459 ] ;

nmat = length (f)
skip = [ 1415 ] ;

for kk = 1:nmat

    % get a matrix
    id = f (kk) ;
    Prob = ssget (id, index) ;
    A = Prob.A ;
    if (isfield (Prob, 'Zeros')) 
        A = A + Prob.Zeros ;
    end
    fprintf ('\n%4d %4d %-40s\n', kk, id, Prob.name) ;
    clear Prob 

    % make symmetric, binary, and remove the diagonal
    A = spones (A) ;
    A = spones (A + A') ;
    A = A - diag (diag (A)) ;

    n = size (A,1) ;

    ttot = 0 ;

    % find all the k-trusses
    for k = 3:100

        tic
        [C, nsteps] = ktruss (A,k) ;
        ne = nnz (C) / 2 ;
        nt = full (sum (sum (C))) / 6 ;
        t = toc ;
        ttot = ttot + t ;

        fprintf ('k %4d sec ne %10d nt %10d nsteps %6d time: %g\n', ...
            k, ne, nt, nsteps, t) ;

        if (ne == 0)
            break ;
        end
    end

    fprintf ('total time: %g\n', ttot) ;
end


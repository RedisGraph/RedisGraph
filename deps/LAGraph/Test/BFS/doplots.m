function doplots (a2,m2,s2,q2, ap,mp,sp,q, nnzA, what)
k = length (m2) ;
n = sum(q)
d = nnzA / n ;

fprintf ('\n---------------------- %s\n', what) ;

% Aydin's rule:
nsteps = length (q) ;
for step = 1:nsteps
    r = q (step) / n ;
    if (r > 0.01)
        % pull
        mbest (step) = mp (step) ;
    else
        % push
        mbest (step) = m2 (step) ;
    end
end

% my rule:
visited = 0 ;
nsteps = length (q) ;
for step = 1:nsteps
    thisq = q (step) ;
    pushwork (step) = d * thisq ;

    % early_exit_prob = (visited+1)/n ;
    % ntrials_before_exit = 1/early_exit_prob ;
    expected_trials = n / (visited+1) ;
    per_dot = min (d, expected_trials) ;

    % pullwork (step) = (n-visited) * d ; % * log2 (thisq) ;
    pullwork (step) = (n-visited) * per_dot * (3 * (1 + log2 (thisq))) ;

    fprintf ('%12g %12g : step %4d do ', pushwork(step), pullwork(step), step) ;

    tbest = min (mp (step), m2 (step)) ;

    if (pullwork (step) < pushwork (step))
        % pull
        my (step) = mp (step) ;
        fprintf ('pull (%g)\n', my(step)/tbest) ;
    else
        % push
        my (step) = m2 (step) ;
        fprintf ('push (%g)\n', my(step)/tbest) ;
    end

    visited = visited + thisq ;
end

subplot (2,3,6) ; semilogy (1:k, pushwork, 'ko-', 1:k, pullwork, 'ro-') ;

% black is push
% red is pull
subplot (2,3,1) ; semilogy (1:k, a2, 'ko-', 1:k, ap, 'ro-') ;
subplot (2,3,2) ; semilogy (1:k, m2, 'ko-', 1:k, mp, 'ro-', 1:k, my, 'co-') ;
title (what) ;
subplot (2,3,3) ; semilogy (1:k, s2, 'ko-', 1:k, sp, 'ro-') ;

assert (isequal (q,q2)) ; 

best = min (m2, mp) ;
fprintf ('\n') ;
fprintf ('push : %12.6e\n', sum (m2)) ;
fprintf ('pull : %12.6e\n', sum (mp)) ;
fprintf ('opt  : %12.6e\n', sum (best)) ;
fprintf ('aydin: %12.6e\n', sum (mbest)) ;
fprintf ('my   : %12.6e\n', sum (my)) ;

% select method automatically

% push method:  each iteration does y = A(:,q)*q.
% one option is to compute the flop count: nnz(A(:,q))
% or, just take work = nnz(A)*(q/n)

% push = nnzA * (q/n) ;
push = d * q ;

subplot (2,3,4) ; plot (push,m2) ;
xlabel ('estimated push work') ;
ylabel ('time') ;

% if (length (q) < 20)
%     [push' 1e6*m2' 1e12*m2'./push']
% end

% pull method:  let nu = # of unvisited nodes.
% where nu = (n - cumsum(q)).
% for each row i the dot product does at most
% nnz(A(i,:)) + q work.  but if these values differ
% by a large amount, a binary search is used:
%
% in that case work is min(nnz(A,:),q) * log2 (max(nnz(A,:),q))
%
% don't compute this ahead of time, but assume
% nnz(A(i,:)) = approx = nnz(A)/n = d.

% work is thus (min (d,q) * log2(max(d,q))) * nu

% but this does not account for early exit.
% the probability of early exit is vis/n
% if vis nodes have been visited 

vis = cumsum ([0 q(1:end-1)]) ;
nu = n - vis ; 
pull = (min (d,q)) .* nu ;
pull = d .* nu ;
pull = (min (d,q) .* log2 (max (d,q))) .* nu ;
pull = (d+q) .* nu ;

pull = pull .* (nu/n) ;

% format long e
% if (length (q) < 20)
%     [nu' pull' 1e6*mp' 1e12*mp'./pull']
% end

subplot (2,3,5) ; plot (pull,mp) ;
xlabel ('estimated pull work') ;
ylabel ('actual time') ;



function iset = mis (A, check)
%GRB.MIS variant of Luby's maximal independent set algorithm.
%
% Usage:
%
%   iset = GrB.mis (A) ;
%
% Given an n-by-n symmetric adjacency matrix A of an undirected graph,
% compute a maximal set of independent nodes and return it in a boolean
% n-vector, 'iset' where iset(i) of true implies node i is a member of
% the set.
%
% The matrix A must not have any diagonal entries (self edges), and it
% must be symmetric.  These conditions are not checked by default, and
% results are undefined if they do not hold.  In particular, diagonal
% entries will cause the method to stall.  To check these conditions,
% use:
%
%   iset = GrB.mis (A, 'check') ;
%
% Reference: M Luby. 1985. A simple parallel algorithm for the maximal
% independent set problem. In Proceedings of the seventeenth annual ACM
% symposium on Theory of computing (STOC '85). ACM, New York, NY, USA,
% 1-10.  DOI: https://doi.org/10.1145/22145.22146
%
% See also GrB.offdiag.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (A) ;
if (m ~= n)
    gb_error ('A must be square') ;
end

% convert A to logical
A = GrB.apply ('1.logical', A) ;

if (nargin < 2)
    check = false ;
else
    if (isequal (check, 'check'))
        check = true ;
    else
        gb_error ('unknown option') ;
    end
end

if (check)
    if (nnz (diag (A)) > 0)
        gb_error ('A must not have any diagonal entries') ;
    end
    if (~issymmetric (A))
        gb_error ('A must be symmetric') ;
    end
end

neighbor_max = GrB (n, 1) ;
new_neighbors = GrB (n, 1, 'logical') ;
candidates = GrB (n, 1, 'logical') ;

% Initialize independent set vector
iset = GrB (n, 1, 'logical') ;

% descriptor: C_replace
r_desc.out = 'replace' ;

% descriptor: C_replace + structural complement of mask
sr_desc.mask = 'complement' ;
sr_desc.out  = 'replace' ;

% compute the degree of each nodes
degrees = GrB.vreduce ('+.double',  A) ;

% Singletons require special treatment.  Since they have no neighbors, their
% prob is never greater than the max of their neighbors, so they never get
% selected and cause the method to stall.  To avoid this case they are removed
% from the candidate set at the begining, and added to the iset.

% candidates (degree != 0) = true
candidates = GrB.assign (candidates, degrees, true) ;

% add all singletons to iset
% iset (degree == 0) = 1
iset = GrB.assign (iset, degrees, true, sr_desc) ; 

% Iterate while there are candidates to check.
ncand = GrB.entries (candidates) ;
last_ncand = ncand ;

while (ncand > 0)

    % compute a random probability scaled by inverse of degree
    % NOTE: this is slower than it should be; rand may not be parallel,
    % See GraphBLAS/Demo/Source/mis.c and the prand_* functions for a better
    % approach using user-defined types and operators.
    prob = 0.0001 + rand (n,1) ./ (1 + 2 * degrees) ;
    prob = GrB.assign (prob, candidates, prob, r_desc) ;

    % compute the max probability of all neighbors
    neighbor_max = GrB.mxm (neighbor_max, candidates, ...
        'max.second.double', A, prob, r_desc) ;

    % select node if its probability is > than all its active neighbors
    new_members = GrB.eadd (prob, '>', neighbor_max) ;

    % add new members to independent set.
    iset = GrB.eadd (iset, '|', new_members) ;

    % remove new members from set of candidates
    candidates = GrB.apply (candidates, new_members, 'identity', ...
        candidates, sr_desc) ;

    ncand = GrB.entries (candidates) ;
    if (ncand == 0)
        break ;                    % early exit condition
    end

    % Neighbors of new members can also be removed from candidates
    new_neighbors = GrB.mxm (new_neighbors, candidates, ...
        '|.&.logical', A, new_members) ;

    candidates = GrB.apply (candidates, new_neighbors, 'identity', ...
        candidates, sr_desc) ;

    ncand = GrB.entries (candidates) ;

    % this will not occur, unless the input is corrupted somehow
    if (last_ncand == ncand)
        gb_error ('method stalled; rerun with ''check'' option') ;
    end
    last_ncand = ncand ;
end

% drop explicit false values
iset = GrB.prune (iset) ;


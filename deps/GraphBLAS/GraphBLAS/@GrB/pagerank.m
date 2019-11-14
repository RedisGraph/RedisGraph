function r = pagerank (A, opts)
%GRB.PAGERANK PageRank of a graph.
% r = GrB.pagerank (A) computes the PageRank of a graph with adjacency matrix
% A.  r = GrB.pagerank (A, options) allows for non-default options to be
% selected.  For compatibility with MATLAB, defaults are identical to the
% MATLAB pagerank method in @graph/centrality and @digraph/centrality:
%
%   opts.tol = 1e-4         stopping criterion
%   opts.maxit = 100        maximum # of iterations to take
%   opts.damp = 0.85        dampening factor
%   opts.weighted = false   true: use edgeweights of A; false: use spones(A)
%   opts.type = 'double'    compute in 'single' or 'double' precision
%
% A can be a GraphBLAS or MATLAB matrix.  A can have any format ('by row' or
% 'by col'), but GrB.pagerank is slightly faster if A is 'by col'.
%
% See also centrality.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% check inputs and set defaults
if (nargin < 2)
    opts = struct ;
end
if (~isfield (opts, 'tol'))
    opts.tol = 1e-4 ;
end
if (~isfield (opts, 'maxit'))
    opts.maxit = 100 ;
end
if (~isfield (opts, 'damp'))
    opts.damp = 0.85 ;
end
if (~isfield (opts, 'weighted'))
    opts.weighted = false ;
end
if (~isfield (opts, 'type'))
    opts.type = 'double' ;
end

% get options
tol = opts.tol ;
maxit = opts.maxit ;
damp = opts.damp ;
type = opts.type ;
weighted = opts.weighted ;

[m, n] = size (A) ;
if (m ~= n)
    gb_error ('A must be square') ;
end

% native, if A is already of the right type, and stored by column
native = (GrB.isbycol (A) & isequal (GrB.type (A), type)) ;

% construct the matrix G and outdegree d
if (weighted)
    % use the weighted edges of G
    d = GrB.vreduce ('+', spones (GrB (A, type))) ;
    if (native)
        G = A ;
    else
        G = GrB (A, type, 'by col') ;
    end
else
    % use the pattern of G 
    if (native)
        G = GrB.apply ('1', A) ;
    else
        G = GrB.apply ('1', GrB (A, type, 'by col')) ;
    end
    d = GrB.vreduce ('+', G) ;
end

% d (i) = outdegree of node i, or 1 if i is a sink
sinks = find (d == 0) ;
any_sinks = ~isempty (sinks) ;
if (any_sinks)
    % d (sinks) = 1 ;
    d = GrB.subassign (d, { sinks }, 1) ;
end

% place explicit zeros on the diagonal of G so that r remains full
I = int64 (0) : int64 (n-1) ;
desc0.base = 'zero-based' ;
G = G + GrB.build (I, I, zeros (n, 1, type), n, n, desc0) ;

% teleport factor
tfactor = cast ((1 - damp) / n, type) ;

% sink factor
dn = cast (damp / n, type) ;

% use G' in GrB.mxm, and return the result as a MATLAB full vector
% FUTURE: when GraphBLAS is fast for dense vector, use them instead
desc.in0 = 'transpose' ;
desc.kind = 'full' ;

% initial PageRank: all nodes have rank 1/n
r = ones (n, 1, type) / n ;

% prescale d with damp so it doesn't have to be done in each iteration
d = d / damp ;

% compute the PageRank
for iter = 1:maxit
    rold = r ;
    teleport = tfactor ;
    if (any_sinks)
        % add the teleport factor from all the sinks
        teleport = teleport + dn * sum (r (sinks)) ;
    end
    % r = damp * G' * (r./d) + teleport
    r = GrB.mxm (G, '+.*', r ./ d, desc) + teleport ;
    if (norm (r - rold, inf) < tol)
        % convergence has been reached
        return ;
    end
end

warning ('GrB:pagerank', 'pagerank failed to converge') ;


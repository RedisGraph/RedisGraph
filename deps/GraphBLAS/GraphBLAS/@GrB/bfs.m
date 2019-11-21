function [v, parent] = bfs (A, s, varargin)
%GRB.BFS breadth-first search of a graph, using its adjacency matrix.  v
%= GrB.bfs (A, s) computes the breadth-first search of the directed graph
%represented by the square adjacency matrix A.  The breadth-first search
%starts at node s.  The output v is a sparse vector of size n-by-1, with
%the level of each node, where v(s)=1, and v(i)=k if the path with the
%fewest edges from from s to i has k-1 edges.  If i is not reachable from
%s, then v(i) is implicitly zero and does not appear in the pattern of v.
%
% [v, parent] = GrB.bfs (A, s) also computes the parent vector,
% representing the breadth-first search tree.  parent(s)=s denotes the
% root of the tree, and parent(c)=p if node p is the parent of c in the
% tree.  The parent vector is sparse, and parent (i) is not present if i
% is not found in the breadth-first search.
%
% Optional string arguments can be provided, after A and s:
%
%   'undirected' or 'symmetric':  A is assumed to be symmetric, and
%       represents an undirected graph.  Results are undefined if A is
%       unsymmetric, and 'check' is not specified.
%
%   'directed' or 'unsymmetric':  A is assumed to be unsymmetric, and
%       presents a directed graph.  This is the default.
%
%   'check': with the 'undirected' or 'symmetric' option, A is checked to
%       ensure that it is symmetric.  The default is not to check.
%
% For best performance, if A represents a directed graph, it should be a
% GraphBLAS matrix stored by row on input.  That is, GrB.format (A)
% should report 'by row'.  (If A represents a directed graph but is
% stored 'by col' on input, it is first converted to 'by row', which is
% costly).  If A is an undirected graph, then it can be stored in either
% format ('by row' or 'by col').
%
% A must be square.  Only the pattern, spones (A), is considered; the
% values of its entries (the edge weights of the graph) are ignored. 
%
% Example:
%
%   A = bucky ;
%   [v pi] = GrB.bfs (A, 1)
%   plot (graph (A))
%   n = size (A,1) ;
%   for level = 1:n
%       level
%       inlevel = find (v == level)
%       parents = full (double (pi (inlevel)))
%       if (isempty (inlevel))
%           break ;
%       end
%   end
%
% See also graph/bfsearch, graph/shortestpathtree.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[m, n] = size (A) ;
if (m ~= n)
    gb_error ('A must be square') ;
end

% get the string options
kind = 'directed' ;
check = false ;
for k = 1:nargin-2
    arg = lower (varargin {k}) ;
    switch arg
        case { 'undirected', 'symmetric' }
            kind = 'undirected' ;
        case { 'directed', 'unsymmetric' }
            kind = 'directed' ;
        case { 'check' }
            check = true ;
        otherwise
            gb_error ('unknown option') ;
    end
end

d = struct ('out', 'replace', 'mask', 'complement') ;

% determine the method to use, and convert A if necessary
if (isequal (kind, 'undirected'))
    if (check && ~issymmetric (A))
        gb_error ('A must be symmetric') ;
    end
    if (GrB.isbycol (A))
        % A is stored by column but undirected, so use q*A' instead of q*A
        d.in1 = 'transpose' ;
    end
else
    if (GrB.isbycol (A))
        % this can be costly
        A = GrB (A, 'by row') ;
    end
end

% determine the integer type to use, and initialize v as a full vector
int_type = 'int64' ;
if (n < intmax ('int32'))
    int_type = 'int32' ;
end
v = full (GrB (1, n, int_type)) ;

% initialize the queue
q = GrB (1, n, int_type) ;                   % q = sparse (1,n)

if (nargout == 1)

    % just compute the level of each node
    q = GrB.subassign (q, { s }, 1) ;            % q (s) = 1
    for level = 1:n
        % assign the current level: v<q> = level
        v = GrB.subassign (v, q, level) ;
        % quit if q is empty
        if (~any (q)), break, end
        % move to the next level:  q<~v,replace> = q*A,
        % using the boolean semiring
        q = GrB.mxm (q, v, '|.&.logical', q, A, d) ;
    end

else

    % compute both the level and the parent
    parent = full (GrB (1, n, int_type)) ;       % parent = zeros (1,n)
    parent = GrB.subassign (parent, { s }, s) ;  % parent (s) = s
    q = GrB.subassign (q, { s }, s) ;            % q (s) = s
    id = GrB (1:n, int_type) ;                   % id = 1:n
    for level = 1:n
        % assign the current level: v<q> = level
        v = GrB.subassign (v, q, level) ;
        if (~any (q)), break, end
        % move to the next level:  q<~v,replace> = q*A,
        % using the min-first semiring
        q = GrB.mxm (q, v, 'min.1st', q, A, d) ;
        % assign parents
        parent = GrB.assign (parent, q, q) ;
        % q(i) = i for all entries in q
        q = GrB.assign (q, q, id) ;
    end
    % remove zeros from parent
    parent = GrB.prune (parent) ;

end

% remove zeros from v
v = GrB.prune (v) ;


function v = bfs_matlab (AT, s)
%BFS_MATLAB a simple breadth-first-search in MATLAB
%
% v = bfs_matlab (AT, s)
%
% AT is a square binary matrix, corresponding to the adjanceny matrix of a
% graph, with AT(j,i)=1 denoting the edge (i,j).  Self loops are permitted.
% s is a scalar input with the source node.  The output v is the level
% each node in the graph, where v(s)=1 (the first level), v(j)=2 if there is
% an edge (s,j) (the 2nd level), etc.  v(j)=k if node j is in the kth level,
% where the shortest path (in terms of # of edges) from  s to j has
% length k+1.  The source node s defaults to 1.

[m n] = size (AT) ;
if (m ~= n)
    error ('AT must be square') ;
end
n = size (AT,1) ;
v = zeros (n,1) ;

if (nargin < 2)
    s = 1 ;
end

% ensure AT is binary
AT = spones (AT) ;

q = sparse (n,1) ;
q (s) = 1 ;         % q is the current level
nq = 1 ;            % # nodes in the current level
nvisited = 0 ;      % # nodes visited in the graph

for level = 1:n

    % assign the level to all nodes in q
    v (find (q)) = level ;

    % break if the whole graph has been visited
    nvisited = nvisited + nq ;
    if (nvisited == n)
        break ;
    end

    % find all neighbors of q, as a binary vector
    qnew = spones (AT * q) ;

    % discard nodes in qnew that are already seen
    qnew (v ~= 0) = 0 ;

    % move to the new level
    q = qnew ;
    nq = nnz (q) ;

    % stop if the new level is empty
    if (nq == 0)
        break ;
    end

end


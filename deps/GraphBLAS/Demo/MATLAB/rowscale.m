function C = rowscale (A)
%ROWSCALE row scale an adjacency matrix by out-degree
% C = rowscale (A)

% scale the adjacency matrix by out-degree
dout = sum (A,2) ;              % dout(i) is the out-degree of node i
is_nonempty = (dout > 0) ;      % find vertices with outgoing edges
nonempty = find (is_nonempty) ; % list of vertices with outgoing edges
n = size (A,1) ;

% divide each non-empty row by its out-degree
dinv = 1 ./ dout (is_nonempty) ;
D = sparse (nonempty, nonempty, dinv, n, n) ;
C = D * A ;

function c = lcc (A)
%LCC compute the local clustering coefficient of a graph
%
% c = lcc (A)
%
% A is the adjacency matrix of a graph, and must be square.  This method is
% slow, by design, so it most closely matches the algorithm specification.  A
% better method would use a single matrix multiplication.

% make A binary and remove the diagonal
[m n] = size (A) ;
if (m ~= n)
    error ('A must be square') ;
end
A = spones (A) ;
A = tril (A,-1) + triu (A,1) ;

c = zeros (n,1) ;

for i = 1:n

    outadj = find (A (i,:)) ;
    inadj  = find (A (:,i)) ;
    both   = union (outadj, inadj) ;
    d = length (both) ;
    if (d >= 2)
        c (i) = nnz (A (both, both)) / (d * (d-1)) ;
    end

end

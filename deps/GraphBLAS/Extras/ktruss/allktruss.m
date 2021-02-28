function [stats,Cout] = allktruss (A)
%ALLKTRUSS compute all k-trusses of a graph.
%
% [stats,C] = allktruss (A)
%
% A is the adjacency matrix of a graph, and must be square, symmetric, and with
% a zero-free diagonal.  It is treated as if binary (A = spones(A)).
%
% With a single output, only the statistics on all non-empty k-trusses of A
% are returned:
%
%   stats.kmax is the smallest k for which the k-truss of A is empty.
%   stats.time (3:kmax) is the time take for each k-truss
%   stats.ntri (3:kmax) is the number of triangles in each k-truss
%   stats.nedges (3:kmax) is the number of edges in the k-truss
%   stats.nsteps (3:kmax) is the # of steps required to compute each k-truss
%
% An optional second output, C, is a cell array of each k-truss.  C{1} and C{2}
% are empty.  C{3} to C{kmax} are the 3-truss to kmax-truss of A.  See ktruss.m
% for a description of each k-truss. The kmax-truss is the first non-empty
% k-truss.
%
% See also allktruss_mex, which computes the same thing in a mexFunction.

%-------------------------------------------------------------------------------
% initializations
%-------------------------------------------------------------------------------

n = size (A,1) ;
stats.kmax = 0 ;
stats.time = zeros (1,n+1) ;
stats.ntri = zeros (1,n+1) ;
stats.nedges = zeros (1,n+1) ;
stats.nsteps = zeros (1,n+1) ;

nn = 0 ;

% count the triangles in the original graph
tic ;
last_cnz = nnz (A) ;
C = spones (A) ;
C = (C*C) .* C ;
nsteps = 1 ;

for k = 3:(n+1)

    % find the k-truss
    while (1)

        % remove edges with not enough support
        if (k > 3)
            C = C .* (C >= (k-2)) ;
        end

        % check if the k-truss has been found
        cnz = nnz (C) ;
        if (cnz == last_cnz)
            % C is the k-truss of A
            stats.ntri (k) = full (sum (sum (C))) / 6 ;
            stats.nedges (k) = cnz / 2 ;
            stats.nsteps (k) = nsteps ;
            nsteps = 0 ;
            if (nargout > 1)
                Cout {k} = C ;
            end
            stats.time (k) = toc ;

            if (cnz == 0)
                % this is the last k-truss
                stats.kmax = k ;
                stats.time = stats.time (1:k) ;
                stats.ntri = stats.ntri (1:k) ;
                stats.nedges = stats.nedges (1:k) ;
                stats.nsteps = stats.nsteps (1:k) ;
                return
            end

            % start finding the next k-truss
            tic ;
            break ;
        end

        % continue searching for this k-truss
        C = spones (C) ;
        last_cnz = cnz ;
        nsteps = nsteps + 1 ;

        % count the triangles for the next iteration
        C = (C*C) .* C ;
    end
end


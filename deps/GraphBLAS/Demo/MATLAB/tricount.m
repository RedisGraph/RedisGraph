function [ntri t] = tricount (method, A, E)
%TRICOUNT count the number of triangles in an undirected unweighted graph
%
% A triangle is a clique of size 3, or three nodes i,j,k such that edges
% (i,j), (j,k), and (i,k) all exist.  This function counts the total number
% of triangles in a graph.
%
% [ntri t] = tricount (method, A)
% [ntri t] = tricount (method, A, E) ; for 'minitri' method
%
% The square matrix A represents a unweighted graph and is treated as if
% binary.  Its pattern is symmetrized with A=A+A'.  The diagonal is ignored.
% The resulting matrix A is the adjacency matrix of the graph, where edge (i,j)
% is present if A(i,j) = 1.
%
% ntri is the number of triangles in the graph.  t is the time taken by the
% triangle counting method.  This time ignores the time taken to sanitize the
% input graph A, and to form any alternative matrices required (E, tril(A),
% triu(A), and so on), depending on the method.
%
% method is a string denoting which method to use, where L=tril(A) and
% U=triu(A).  If empty, the default is the Sandia method, which is by far the
% fastest method.  It also uses the least amount of memory.
%
%   minitri:    ntri = nnz (A*E == 2) / 3
%   Burkhardt:  ntri = sum (sum ((A^2) .* A)) / 6
%   Cohen:      ntri = sum (sum ((L * U) .* A)) / 2
%   Sandia:     ntri = sum (sum ((U * U) .* U))
%   Sandia2:    ntri = sum (sum ((L * L) .* L))
%
% E is an optional sparse matrix that represents the same graph as A, but in an
% edge incidence format.  Each column e of E has exactly two nonzeros, where
% E([i j],e) is [1 1] if the edge (i,j) is in the graph.  The edges (e) appear
% in any order.  It is required by minitri only.  If not present, it is
% computed from A.
%
% Note that the "Sandia" method presented by Wolf, Deveci, Berry, Hammond,
% Rajamanickam, 'Fast linear algebra- based triangle counting with
% KokkosKernels', IEEE HPEC'17, https://dx.doi.org/10.1109/HPEC.2017.8091043,
% computes (L*L).*L using KokkosKernels, but that package stores its matrices
% in compressed sparse row form.  MATLAB stores its matrices in compressed
% sparse column form, so the MATLAB equivalent of the Sandia method is
% sum(sum((U*U).*U)).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

%-------------------------------------------------------------------------------
% check inputs
%-------------------------------------------------------------------------------

% Sanitize the input matrix.  This time is not counted since we could assume
% the input matrix is OK, and just use it as-is.  This is the approach taken by
% the 2018 GraphChallenge.org.

[m n] = size (A) ;
if (m ~= n)
    error ('A must be square') ;
end

% ensure A is sparse, binary, symmetric, and has no diagonal entries.  Do
% not count this against the prep time, since this condition is assumed
% true in the GraphChallenge input matrices.
A = spones (sparse (A)) ;
A = spones (A+A') ;

% count the time to find tril(A) and triu(A), however
tic
L = tril (A, -1) ;
tril_time = toc ;

tic
U = triu (A, 1) ;
triu_time = toc ;

% remove diagonal of A
A = L + U ;
n = size (A,1) ;

% default method
if (isempty (method))
    method = 'Sandia' ;
end

%-------------------------------------------------------------------------------
% count the triangles
%-------------------------------------------------------------------------------

switch  method

    %===========================================================================
    case 'minitri'
    %===========================================================================

        % Wolf, Berry, and Stark, 'A task-based lineaer algebra building blocks
        % approach for scalable graph analytics', 2015 IEEE HPEC, pp1-6.
        %
        % C = A*E
        % ntri = nnz (C == 2) / 3
        %
        % Consider the edge (i,j) in a column e of E.  Column C(:,e) will be
        % A(:,i)+A(:,j), via C=A*E.  If there is a row index k such that A(k,i)
        % and A(k,j) are both 1, then (i,j,k) forms a triangle, and C(:,e) is
        % 2.  This triangle is counted three times, once for each edge in the
        % triangle, so the count has to be divided by 3.
        %
        % In the 2018 GraphChallenge.org MATLAB source code for this method,
        % constructing E is excluded from the timings since it is provided
        % on input.

        % In GraphBLAS notation:
        % C = 0
        % C = A*E, via GrB_mxm, with no mask
        % C = f(C), using GrB_apply, where f(x) = ((x==2) ? 1:0)
        % reduce C to a scalar via GrB_reduce, and divide by 3

        if (nargin >= 3)
            % ensure the incidence matrix is binary
            E = spones (E) ;
        else
            % form the incidence matrix from the adjacency matrix A
            E = adj_to_edges (A) ;
        end

        % check the incidence matrix
        if (~all (sum (E) == 2))
            error ('invalid incidence matrix E: nnz in each column must be 2') ;
        end
        A2 = E*E' ;
        A2 = tril (A2,-1) + triu (A2,1) ;       % remove the diagonal
        if (~isequal (A,A2))
            error ('invalid incidence matrix E: does not match adj. matrix A') ;
        end
        clear A2

        %-----------------------------------------------------------------------
        % minitri method:
        t.prep_time = 0 ;   % assume both A and E are available already
        tic ;
        ntri = nnz ((A*E) == 2) / 3 ;
        t.triangle_count_time = toc ;
        %-----------------------------------------------------------------------

    %===========================================================================
    case 'Burkhardt'
    %===========================================================================

        % Burkhardt, 'Graphing trillions of triangles', Information
        % Visualization, 16(3), Sept 2017,
        % https://doi.org/10.1177%2F1473871616666393
        %
        % C = A^2
        % ntri = sum (sum (C .* A)) / 6
        %
        % Consider the edge (i,j), so that A(i,j)=1.  The term C(i,j) is the
        % dot product A(i,:)*A(:,j), since A is symmetric.  If there is a node
        % k that forms a triangle (i,j,k), then A(i,k)*A(k,j) is 1.  Thus
        % C(i,j) is the number of triangles incident on the edge (i,j).  The
        % triangle (i,j,k) will be counted 6 times, in C(i,j), C(j,i), C(j,k),
        % C(k,j) C(i,k), and C(k,i).  A(i,j)=0 then C(i,j) is useless since it
        % does not count any triangles at all.  These counts are pruned via
        % C.*A.

        %-----------------------------------------------------------------------
        % Burkhardt method:
        t.prep_time = 0 ;   % uses A directly, with no prep
        tic ;
        ntri = sum (sum ((A^2) .* A)) / 6 ;
        t.triangle_count_time = toc ;
        %-----------------------------------------------------------------------

    %===========================================================================
    case 'Cohen'
    %===========================================================================

        % Cohen, 'Graph twiddling in a map-reduce world', Computing in Science
        % and Eng., 11(4):29-41, July 2009.  See also Azad, Buluc, Gilbert,
        % 'Parallel triangle counting and enumeration using matrix algebra',
        % 2015 IEEE IPDPSW, pp 804–811.
        %
        % L = tril (A) ;     % or tril(A,-1), since A has no diagonal entries
        % U = triu (A) ;
        % C = L*U
        % ntri = sum (sum (C .* A)) / 2
        %
        % This method is a variant of Burkhardt's that reduces the work.
        %
        % Consider the edge (i,j), so that A(i,j)=1.  Let s = min(i,j)-1.  The
        % term C(i,j) is the dot product L(i,:)*U(:,j), and since L and U are
        % strictly lower and upper triangular, respectively, this is the same
        % as the dot product A(i,1:s)*A(1:s,j).  If there is a node k <
        % min(i,j) that forms a triangle (i,j,k), then A(i,k)*A(k,j) is 1.
        % Thus C(i,j) is the number of triangles (i,j,k) incident on the edge
        % (i,j) for which k < min(i,j).  The triangle (i,j,k) will be counted
        % exactly twice, in C(i,j) and C(j,i).

        %-----------------------------------------------------------------------
        % Cohen method:
        t.prep_time = tril_time + triu_time ;   % needs L and U
        tic ;
        ntri = sum (sum ((L * U) .* A)) / 2 ;
        t.triangle_count_time = toc ;
        %-----------------------------------------------------------------------

    %===========================================================================
    case 'Sandia'   % sum (sum ((U*U).*U))
    %===========================================================================

        % Wolf, Deveci, Berry, Hammond, Rajamanickam, 'Fast linear algebra-
        % based triangle counting with KokkosKernels', IEEE HPEC'17,
        % https://dx.doi.org/10.1109/HPEC.2017.8091043
        %
        % A variant of Cohen's method that only requires L.
        %
        % L = tril (A) ;
        % C = L*L
        % ntri = sum (sum (C .* L))
        %
        % This method is a variant of Cohen's method that reduces the work.
        %
        % Consider the edge (i,j), so that A(i,j)=1, and suppose i > j.  C(i,j)
        % is the dot product L(i,:)*L(:,j).  Since L is strictly lower
        % triangular, L(i,:) is nonzero only in L(i,1:i-1), and L(:,j) is
        % nonzero only in L(j+1:n,j).  Combining these two sets gives the
        % nonzero intersection, C(i,j) = L(i,j+1:i-1)*L(j+1:i-1,j).  Suppose
        % there is a node k in this range j+1:i-1, inclusive, that has an edge
        % to both i and j.  That is, L(i,k) = L(k,j) = 1.  This is a triangle
        % (i,j,k), where j < k < i.  Since these three indices are strictly
        % ordered, the triangle is counted only once, and C(i,j) is the number
        % of such triangles for all k.  If A(i,j)=0 then the result is not
        % needed since (i,j,k) can't be a triangle, and thus the final ().*L
        % step.
        %
        % The method in Wolf, Deveci, et al, cited above, also sorts the rows
        % of L by decreasing node degree, but the MATLAB statement below uses U
        % as-is.
        %
        % Note that L need not be strictly lower triangular.  It can also be a
        % symmetric permutation of a strictly lower triangular matrix.  Proof:
        % Let P be a permutation matrix.  Let B = P*L*P'.  Then (B*B).*B =
        % (P*L*P'*P*L*P').*P*L*P' = (P*L*L*P').*P*L*P'.  The matrix L*L and L
        % are permuted identically.  Any entry in (P*L*L*P') has the same
        % position in mask, P*L*P', since it will have been permuted to the
        % same position.  The result is a permutation of (L*L).*L.  That is
        % (B*B).*B = P*( (L*L)*.L )*P'.  The final computation for ntri is
        % simply the reduction, sum(sum(...)), so the result is the same.
        %
        % The method of Wolf, Deveci, et al exploits this property.  They find
        % an inverse permutation vector invp such that invp(i)=k if row/column
        % i of A is the kth row/column of P*A*P'.  Then "L" is computed as a
        % subset of the entries of A, but in the same positions as they are in
        % A.  An entry (i,j) is kept if invp(i) > invp(j).  The resulting "L"
        % is a permutation of tril(A).
        %
        % Their method stores L in row-oriented form, which is the same as the
        % matrix U in column-oriented form.  Thus, comparing MATLAB with the
        % Sandia method, the MATLAB statement is sum(sum(U*U).*U).  See also
        % the case 'Sandia2' below.

        %-----------------------------------------------------------------------
        % Sandia method:
        t.prep_time = triu_time ;   % needs U only
        tic ;
        ntri = sum (sum ((U * U) .* U)) ;
        t.triangle_count_time = toc ;
        %-----------------------------------------------------------------------

    %===========================================================================
    case 'Sandia2'  % sum (sum ((L*L).*L))
    %===========================================================================

        % Note that for all these methods, MATLAB computes all of U*U or L*L
        % above, and only then applies the mask via the (.*) operation, whereas
        % the masked sparse matrix-matrix multiply in Kokkos, and in GraphBLAS,
        % exploit the mask during the multiplication.  This means MATLAB can
        % easily run out of memory when computing U*U or L*L.  In contrast,
        % Kokkos and GraphBLAS only require O(nnz(L)) memory to compute
        % sum(sum(L*L).*L).

        %-----------------------------------------------------------------------
        % Sandia2 method:
        t.prep_time = tril_time ;   % needs L only
        tic ;
        ntri = sum (sum ((L * L) .* L)) ;
        t.triangle_count_time = toc ;
        %-----------------------------------------------------------------------

    %===========================================================================
    case 'SandiaDot'  % sum (sum ((L'*U).*U))
    %===========================================================================

        % same as Sandia method, but with L' instead of U.  The matrices are
        % the same but MATLAB might treat the L' differently.

        %-----------------------------------------------------------------------
        % Sandia method:
        t.prep_time = triu_time + tril_time ;
        tic ;
        ntri = sum (sum ((L' * U) .* U)) ;
        t.triangle_count_time = toc ;
        %-----------------------------------------------------------------------


    otherwise
        error ('unrecognized method') ;

end

% sum(sum(...)) returns its result as a sparse scalar, so make it full.
ntri = full (ntri) ;


function s = tricount (A, check)
%GRB.TRICOUNT count triangles in a matrix.
% s = GrB.tricount (A) counts the number of triangles in the matrix A.
% spones (A) must be symmetric; results are undefined if spones (A) is
% unsymmetric.  Diagonal entries are ignored.
%
% To check the input matrix A, use GrB.tricount (A, 'check').  This check
% takes additional time so by default the input is not checked.
%
% See also GrB.ktruss.

[m, n] = size (A) ;
if (m ~= n)
    gb_error ('A must be square') ;
end
if (nargin < 2)
    check = false ;
else
    check = isequal (check, 'check') ;
end

if (check && ~issymmetric (spones (A)))
    gb_error ('pattern of A must be symmetric') ;
end

% C, L, and U will have the same format as A
C = GrB (n, n, 'int64', GrB.format (A)) ;
L = tril (A, -1) ;
U = triu (A, 1) ;

% Inside GraphBLAS, the methods below are identical.  For example, L stored by
% row is the same data structure as U stored by column.  Both use the
% SandiaDot2 method as defined in LAGraph (case 6), which is typically the
% fastest of the methods in LAGraph_tricount.

desc.mask = 'structural' ;

if (GrB.isbyrow (A))
    % C<U> = U*L': SandiaDot2 method
    desc.in1 = 'transpose' ;
    C = GrB.mxm (C, U, '+.pair.int64', U, L, desc) ;
else
    % C<U> = L'*U: SandiaDot2 method
    desc.in0 = 'transpose' ;
    C = GrB.mxm (C, U, '+.pair.int64', L, U, desc) ;
end

s = full (double (GrB.reduce ('+.int64', C))) ;


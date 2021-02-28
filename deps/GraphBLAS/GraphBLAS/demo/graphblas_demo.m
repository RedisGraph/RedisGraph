%% GraphBLAS: graph algorithms in the language of linear algebra
% GraphBLAS is a library for creating graph algorithms based on sparse
% linear algebraic operations over semirings.  Visit http://graphblas.org
% for more details and resources.  See also the SuiteSparse:GraphBLAS
% User Guide in this package.
%
% SuiteSparse:GraphBLAS, (c) 2017-2020, Tim Davis, Texas A&M University,
% http://faculty.cse.tamu.edu/davis

%% GraphBLAS: faster and more general sparse matrices for MATLAB
% GraphBLAS is not only useful for creating graph algorithms; it also
% supports a wide range of sparse matrix data types and operations.
% MATLAB can compute C=A*B with just two semirings: 'plus.times.double'
% and 'plus.times.complex' for complex matrices.  GraphBLAS has 1,040
% unique built-in semirings, such as 'max.plus'
% (https://en.wikipedia.org/wiki/Tropical_semiring).  These semirings can
% be used to construct a wide variety of graph algorithms, based on
% operations on sparse adjacency matrices.
%
% GraphBLAS supports sparse double and single precision matrices,
% logical, and sparse integer matrices: int8, int16, int32, int64, uint8,
% uint16, uint32, and uint64.  Complex matrices will be added in the
% future.

clear all
format compact
rng ('default') ;
X = 100 * rand (2) ;
G = GrB (X)              % GraphBLAS copy of a matrix X, same type

%% Sparse integer matrices
% Here's an int8 version of the same matrix:

S = int8 (G)            % convert G to a full MATLAB int8 matrix
G = GrB (X, 'int8')      % a GraphBLAS sparse int8 matrix

%% Sparse single-precision matrices
% Matrix operations in GraphBLAS are typically as fast, or faster than
% MATLAB.  Here's an unfair comparison: computing X^2 with MATLAB in
% double precision and with GraphBLAS in single precision.  You would
% naturally expect GraphBLAS to be faster. 
%
% Please wait ...

n = 1e5 ;
X = spdiags (rand (n, 201), -100:100, n, n) ;
G = GrB (X, 'single') ;
tic
G2 = G^2 ;
gb_time = toc ;
tic
X2 = X^2 ;
matlab_time = toc ;
fprintf ('\nGraphBLAS time: %g sec (in single)\n', gb_time) ;
fprintf ('MATLAB time:    %g sec (in double)\n', matlab_time) ;
fprintf ('Speedup of GraphBLAS over MATLAB: %g\n', ...
    matlab_time / gb_time) ;

%% Mixing MATLAB and GraphBLAS matrices
% The error in the last computation is about eps('single') since
% GraphBLAS did its computation in single precision, while MATLAB used
% double precision.  MATLAB and GraphBLAS matrices can be easily
% combined, as in X2-G2.  The sparse single precision matrices take less
% memory space.

err = norm (X2 - G2, 1) / norm (X2,1)
eps ('single')
whos G G2 X X2

%% Faster matrix operations
% But even with standard double precision sparse matrices, GraphBLAS is
% typically faster than the built-in MATLAB methods.  Here's a fair
% comparison:

G = GrB (X) ;
tic
G2 = G^2 ;
gb_time = toc ;
err = norm (X2 - G2, 1) / norm (X2,1)
fprintf ('\nGraphBLAS time: %g sec (in double)\n', gb_time) ;
fprintf ('MATLAB time:    %g sec (in double)\n', matlab_time) ;
fprintf ('Speedup of GraphBLAS over MATLAB: %g\n', ...
    matlab_time / gb_time) ;

%% A wide range of semirings
% MATLAB can only compute C=A*B using the standard '+.*.double' and
% '+.*.complex' semirings.  A semiring is defined in terms of a string,
% 'add.mult.type', where 'add' is a monoid that takes the place of the
% additive operator, 'mult' is the multiplicative operator, and 'type' is
% the data type for the two inputs to the mult operator (the type
% defaults to the type of A for C=A*B).
%
% In the standard semiring, C=A*B is defined as:
%
%   C(i,j) = sum (A(i,:).' .* B(:,j))
%
% using 'plus' as the monoid and 'times' as the multiplicative operator.
% But in a more general semiring, 'sum' can be any monoid, which is an
% associative and commutative operator that has an identity value.  For
% example, in the 'max.plus' tropical algebra, C(i,j) for C=A*B is
% defined as:
%
%   C(i,j) = max (A(i,:).' + B(:,j))

%%
% This can be computed in GraphBLAS with:
%
%   C = GrB.mxm ('max.+', A, B)

n = 3 ;
A = rand (n) ;
B = rand (n) ;
C = zeros (n) ;
for i = 1:n
    for j = 1:n
        C(i,j) = max (A (i,:).' + B (:,j)) ;
    end
end
C2 = GrB.mxm ('max.+', A, B) ;
fprintf ('\nerr = norm (C-C2,1) = %g\n', norm (C-C2,1)) ;

%% The max.plus tropical semiring
% Here are details of the "max.plus" tropical semiring.  The identity
% value is -inf since max(x,-inf) = max (-inf,x) = -inf for any x.

GrB.semiringinfo ('max.+.double') ;

%% A boolean semiring
% MATLAB cannot multiply two logical matrices.  MATLAB R2019a converts
% them to double and uses the conventional +.*.double semiring instead.
% In GraphBLAS, this is the common Boolean 'or.and.logical' semiring,
% which is widely used in linear algebraic graph algorithms.

GrB.semiringinfo ('|.&.logical') ;

%%
clear
A = sparse (rand (3) > 0.5)
B = sparse (rand (3) > 0.2)

%%
try
    % MATLAB R2019a does this by casting A and B to double
    C1 = A*B
catch
    % MATLAB R2018a throws an error
    fprintf ('MATLAB R2019a required for C=A*B with logical\n') ;
    fprintf ('matrices.  Explicitly converting to double:\n') ;
    C1 = double (A) * double (B)
end
C2 = GrB (A) * GrB (B)

%%
% Note that C1 is a MATLAB sparse double matrix, and contains non-binary
% values.  C2 is a GraphBLAS logical matrix.
whos
GrB.type (C2)

%% GraphBLAS operators, monoids, and semirings
% The C interface for SuiteSparse:GraphBLAS allows for arbitrary types
% and operators to be constructed.  However, the MATLAB interface to
% SuiteSparse:GraphBLAS is restricted to pre-defined types and operators:
% a mere 11 types, 66 unary operators, 275 binary operators, 44 monoids,
% 16 select operators, and 1,865 semirings (1,040 of which are unique,
% since some binary operators are equivalent: 'min.logical' and
% '&.logical' are the same thing, for example).  The complex type and
% its binary operators, monoids, and semirings will be added in the
% near future.
%
% That gives you a lot of tools to create all kinds of interesting
% graph algorithms.  For example:
%
%   GrB.bfs    % breadth-first search
%   GrB.dnn    % sparse deep neural network (http://graphchallenge.org)
%   GrB.mis    % maximal independent set
%
% See 'help GrB.binopinfo' for a list of the binary operators, and
% 'help GrB.monoidinfo' for the ones that can be used as the additive
% monoid in a semiring.

%% 
help GrB.binopinfo

%% 
help GrB.monoidinfo

%% Element-wise operations
% Binary operators can be used in element-wise matrix operations, like
% C=A+B and C=A.*B.  For the matrix addition C=A+B, the pattern of C is
% the set union of A and B, and the '+' operator is applied for entries
% in the intersection.  Entries in A but not B, or in B but not A, are
% assigned to C without using the operator.  The '+' operator is used for
% C=A+B but any operator can be used with GrB.eadd.

%%
A = GrB (sprand (3, 3, 0.5)) ;
B = GrB (sprand (3, 3, 0.5)) ;
C1 = A + B
C2 = GrB.eadd ('+', A, B)
err = norm (C1-C2,1)

%% Subtracting two matrices
% A-B and GrB.eadd ('-', A, B) are not the same thing, since the '-'
% operator is not applied to an entry that is in B but not A.

C1 = A-B 
C2 = GrB.eadd ('-', A, B)

%% 
% But these give the same result

C1 = A-B 
C2 = GrB.eadd ('+', A, GrB.apply ('-', B))
err = norm (C1-C2,1)

%% Element-wise 'multiplication'
% For C = A.*B, the result C is the set intersection of the pattern of A
% and B.  The operator is applied to entries in both A and B.  Entries in
% A but not B, or B but not A, do not appear in the result C.

C1 = A.*B
C2 = GrB.emult ('*', A, B) 
C3 = double (A) .* double (B)

%%
% Just as in GrB.eadd, any operator can be used in GrB.emult:

A
B
C2 = GrB.emult ('max', A, B) 

%% Overloaded operators
% The following operators all work as you would expect for any matrix.
% The matrices A and B can be GraphBLAS matrices, or MATLAB sparse or
% dense matrices, in any combination, or scalars where appropriate:
%
%    A+B   A-B  A*B   A.*B  A./B  A.\B  A.^b   A/b   C=A(I,J)
%    -A    +A   ~A    A'    A.'   A&B   A|B    b\A   C(I,J)=A
%    A~=B  A>B  A==B  A<=B  A>=B  A<B   [A,B]  [A;B]
%    A(1:end,1:end)
%
% For A^b, b must be a non-negative integer.

C1 = [A B] ;
C2 = [double(A) double(B)] ;
assert (isequal (double (C1), C2))

%%
C1 = A^2
C2 = double (A)^2 ;
err = norm (C1 - C2, 1)
assert (err < 1e-12)

%%
C1 = A (1:2,2:end)
A = double (A) ;
C2 = A (1:2,2:end) ;
assert (isequal (double (C1), C2))

%% Overloaded functions
% Many MATLAB built-in functions can be used with GraphBLAS matrices:
%
% A few differences with the built-in functions:
%
%   S = sparse (G)        % makes a copy of a GrB matrix
%   F = full (G)          % adds explicit zeros, so numel(F)==nnz(F)
%   F = full (G,type,id)  % adds explicit identity values to a GrB matrix
%   disp (G, level)       % display a GrB matrix G; level=2 is the default.
%
% In the list below, the first set of Methods are overloaded built-in
% methods.  They are used as-is on GraphBLAS matrices, such as C=abs(G).
% The Static methods are prefixed with "GrB.", as in C = GrB.apply ( ... ).

%%

methods GrB

%% Zeros are handled differently
% Explicit zeros cannot be automatically dropped from a GraphBLAS matrix,
% like they are in MATLAB sparse matrices.  In a shortest-path problem,
% for example, an edge A(i,j) that is missing has an infinite weight,
% (the monoid identity of min(x,y) is +inf).  A zero edge weight A(i,j)=0
% is very different from an entry that is not present in A.  However, if
% a GraphBLAS matrix is converted into a MATLAB sparse matrix, explicit
% zeros are dropped, which is the convention for a MATLAB sparse matrix.
% They can also be dropped from a GraphBLAS matrix using the GrB.select
% method.

%%

G = GrB (magic (2)) ;
G (1,1) = 0      % G(1,1) still appears as an explicit entry
A = double (G)   % but it's dropped when converted to MATLAB sparse
H = GrB.select ('nonzero', G)  % drops the explicit zeros from G
fprintf ('nnz (G): %d  nnz (A): %g nnz (H): %g\n', ...
    nnz (G), nnz (A), nnz (H)) ;
fprintf ('num entries in G: %d\n', GrB.entries (G)) ;

%% Displaying contents of a GraphBLAS matrix
% Unlike MATLAB, the default is to display just a few entries of a GrB matrix.
% Here are all 100 entries of a 10-by-10 matrix, using a non-default disp(G,3):

%%
G = GrB (rand (10)) ;
% display everything:
disp (G,3)

%%
% That was disp(G,3), so every entry was printed.  It's a little long, so
% the default is not to print everything.

%%
% With the default display (level = 2):
G

%%
% That was disp(G,2) or just display(G), which is what is printed by a
% MATLAB statement that doesn't have a trailing semicolon.  With
% level = 1, disp(G,1) gives just a terse summary:
disp (G,1)

%% Storing a matrix by row or by column
% MATLAB stores its sparse matrices by column, refered to as 'standard
% CSC' in SuiteSparse:GraphBLAS.  In the CSC (compressed sparse column)
% format, each column of the matrix is stored as a list of entries, with
% their value and row index.  In the CSR (compressed sparse row) format,
% each row is stored as a list of values and their column indices.
% GraphBLAS uses both CSC and CSR, and the two formats can be intermixed
% arbitrarily.  In its C interface, the default format is CSR.  However,
% for better compatibility with MATLAB, this MATLAB interface for
% SuiteSparse:GraphBLAS uses CSC by default instead. 

%%
rng ('default') ;
GrB.clear ;                      % clear all prior GraphBLAS settings
fprintf ('the default format is: %s\n', GrB.format) ;
C = sparse (rand (2))
G = GrB (C)
GrB.format (G)

%%
% Many graph algorithms work better in CSR format, with matrices stored
% by row.  For example, it is common to use A(i,j) for the edge (i,j),
% and many graph algorithms need to access the out-adjacencies of nodes,
% which is the row A(i,;) for node i.  If the CSR format is desired,
% GrB.format ('by row') tells GraphBLAS to create all subsequent matrices
% in the CSR format.  Converting from a MATLAB sparse matrix (in standard
% CSC format) takes a little more time (requiring a transpose), but
% subsequent graph algorithms can be faster.

%%
G = GrB (C, 'by row')
fprintf ('the format of G is:    %s\n', GrB.format (G)) ;
H = GrB (C)
fprintf ('the format of H is:    %s\n', GrB.format (H)) ;
err = norm (H-G,1)

%% Hypersparse matrices
% SuiteSparse:GraphBLAS can use two kinds of sparse matrix data
% structures: standard and hypersparse, for both CSC and CSR formats.  In
% the standard CSC format used in MATLAB, an m-by-n matrix A takes
% O(n+nnz(A)) space.  MATLAB can create huge column vectors, but not huge
% matrices (when n is huge).

clear all
[c, huge] = computer ;
C = sparse (huge, 1)    % MATLAB can create a huge-by-1 sparse column
try
    C = sparse (huge, huge)     % but this fails
catch me
    error_expected = me
end

%%
% In a GraphBLAS hypersparse matrix, an m-by-n matrix A takes only
% O(nnz(A)) space.  The difference can be huge if nnz (A) << n.

clear
[c, huge] = computer ;
G = GrB (huge, 1)            % no problem for GraphBLAS
H = GrB (huge, huge)         % this works in GraphBLAS too

%%
% Operations on huge hypersparse matrices are very fast; no component of
% the time or space complexity is Omega(n).

I = randperm (huge, 2) ;
J = randperm (huge, 2) ;
H (I,J) = magic (2) ;        % add 4 nonzeros to random locations in H
H (I,I) = 10 * [1 2 ; 3 4] ; % so H^2 is not all zero
H = H^2 ;                    % square H
H = (H' * 2) ;               % transpose H and double the entries
K = pi * spones (H) ;
H = H + K                    % add pi to each entry in H

%% numel uses vpa if the matrix is really huge
e1 = numel (G)               % this is huge, but still a flint
e2 = numel (H)               % this is huge^2, which needs vpa
whos e1 e2

%%
% All of these matrices take very little memory space:
whos C G H K

%% The mask and accumulator
% When not used in overloaded operators or built-in functions, many
% GraphBLAS methods of the form GrB.method ( ... ) can optionally use a
% mask and/or an accumulator operator.  If the accumulator is '+' in
% GrB.mxm, for example, then C = C + A*B is computed.  The mask acts much
% like logical indexing in MATLAB.  With a logical mask matrix M,
% C<M>=A*B allows only part of C to be assigned.  If M(i,j) is true, then
% C(i,j) can be modified.  If false, then C(i,j) is not modified.
%
% For example, to set all values in C that are greater than 0.5 to 3:

%%
A = rand (3) 
C = GrB.assign (A, A > 0.5, 3) ;     % in GraphBLAS
C1 = GrB (A) ; C1 (A > .5) = 3       % also in GraphBLAS
C2 = A      ; C2 (A > .5) = 3       % in MATLAB
err = norm (C - C1, 1)
err = norm (C - C2, 1)

%% The descriptor
% Most GraphBLAS functions of the form GrB.method ( ... ) take an optional
% last argument, called the descriptor.  It is a MATLAB struct that can
% modify the computations performed by the method.  'help
% GrB.descriptorinfo' gives all the details.  The following is a short
% summary of the primary settings:
%
% d.out  = 'default' or 'replace', clears C after the accum op is used.
%
% d.mask = 'default' or 'complement', to use M or ~M as the mask matrix;
%          'structural', or 'structural complement', to use the pattern
%           of M or ~M.
%
% d.in0  = 'default' or 'transpose', to transpose A for C=A*B, C=A+B, etc.
%
% d.in1  = 'default' or 'transpose', to transpose B for C=A*B, C=A+B, etc.
%
% d.kind = 'default', 'GrB', 'sparse', or 'full'; the output of GrB.method.

A = sparse (rand (2)) ;
B = sparse (rand (2)) ;
C1 = A'*B ;
C2 = GrB.mxm ('+.*', A, B, struct ('in0', 'transpose')) ;
err = norm (C1-C2,1)

%% Integer arithmetic is different in GraphBLAS
% MATLAB supports integer arithmetic on its full matrices, using int8,
% int16, int32, int64, uint8, uint16, uint32, or uint64 data types.  None
% of these integer data types can be used to construct a MATLAB sparse
% matrix, which can only be double, double complex, or logical.
% Furthermore, C=A*B is not defined for integer types in MATLAB, except
% when A and/or B are scalars.
%
% GraphBLAS supports all of those types for its sparse matrices (except
% for complex, which will be added in the future).  All operations are
% supported, including C=A*B when A or B are any integer type, for all
% 1,865 semirings (1,040 of which are unique).
%
% However, integer arithmetic differs in GraphBLAS and MATLAB.  In
% MATLAB, integer values saturate if they exceed their maximum value.  In
% GraphBLAS, integer operators act in a modular fashion.  The latter is
% essential when computing C=A*B over a semiring.  A saturating integer
% operator cannot be used as a monoid since it is not associative.
%
% The C API for GraphBLAS allows for the creation of arbitrary
% user-defined types, so it would be possible to create different binary
% operators to allow element-wise integer operations to saturate,
% perhaps:
%
%   C = GrB.eadd('+saturate',A,B)
%
% This would require an extension to this MATLAB interface.

%%
C = uint8 (magic (3)) ;
G = GrB (C) ;
C1 = C * 40
C2 = G * 40
C3 = double (G) * 40 ;
S = double (C1 < 255) ;
assert (isequal (double (C1).*S, double (C2).*S))
assert (isequal (nonzeros (C2), double (mod (nonzeros (C3), 256))))

%% An example graph algorithm: breadth-first search
% The breadth-first search of a graph finds all nodes reachable from the
% source node, and their level, v.  v=GrB.bfs(A,s) or v=bfs_matlab(A,s)
% compute the same thing, but GrB.bfs uses GraphBLAS matrices and
% operations, while bfs_matlab uses pure MATLAB operations.  v is defined
% as v(s) = 1 for the source node, v(i) = 2 for nodes adjacent to the
% source, and so on.

clear all
rng ('default') ;
n = 1e5 ;
A = logical (sprandn (n, n, 1e-3)) ;

tic
v1 = GrB.bfs (A, 1) ;
gb_time = toc ;

tic
v2 = bfs_matlab (A, 1) ;
matlab_time = toc ;

assert (isequal (double (v1'), v2))
fprintf ('\nnodes reached: %d of %d\n', nnz (v2), n) ;
fprintf ('GraphBLAS time: %g sec\n', gb_time) ;
fprintf ('MATLAB time:    %g sec\n', matlab_time) ;
fprintf ('Speedup of GraphBLAS over MATLAB: %g\n', ...
    matlab_time / gb_time) ;

%% Example graph algorithm: Luby's method in GraphBLAS
% The GrB.mis.m function is variant of Luby's randomized algorithm [Luby
% 1985].  It is a parallel method for finding an maximal independent set
% of nodes, where no two nodes are adjacent.  See the
% GraphBLAS/@GrB/GrB.mis.m function for details.  The graph must be
% symmetric with a zero-free diagonal, so A is symmetrized first and any
% diagonal entries are removed.

A = GrB (A) ;
A = GrB.offdiag (A|A') ;

tic
s = GrB.mis (A) ;
toc
fprintf ('# nodes in the graph: %g\n', size (A,1)) ;
fprintf ('# edges: : %g\n', GrB.entries (A) / 2) ;
fprintf ('size of maximal independent set found: %g\n', ...
    full (double (sum (s)))) ;

% make sure it's independent
p = find (s) ;
S = A (p,p) ;
assert (GrB.entries (S) == 0)

% make sure it's maximal
notp = find (s == 0) ;
S = A (notp, p) ;
deg = GrB.vreduce ('+.int64', S) ;
assert (logical (all (deg > 0)))

%% Sparse deep neural network
% The 2019 MIT GraphChallenge (see http://graphchallenge.org) is to solve
% a set of large sparse deep neural network problems.  In this demo, the
% MATLAB reference solution is compared with a solution using GraphBLAS,
% for a randomly constructed neural network.  See the GrB.dnn and
% dnn_matlab.m functions for details.

clear all
rng ('default') ;
nlayers = 16 ;
nneurons = 4096 ;
nfeatures = 30000 ;
fprintf ('# layers:   %d\n', nlayers) ;
fprintf ('# neurons:  %d\n', nneurons) ;
fprintf ('# features: %d\n', nfeatures) ;

tic
Y0 = sprand (nfeatures, nneurons, 0.1) ;
for layer = 1:nlayers
    W {layer} = sprand (nneurons, nneurons, 0.01) * 0.2 ;
    bias {layer} = -0.2 * ones (1, nneurons) ;
end
t_setup = toc ;
fprintf ('construct problem time: %g sec\n', t_setup) ;

% convert the problem from MATLAB to GraphBLAS
t = tic ;
[W_gb, bias_gb, Y0_gb] = dnn_mat2gb (W, bias, Y0) ;
t = toc (t) ;
fprintf ('setup time: %g sec\n', t) ;

%% Solving the sparse deep neural network problem with GraphbLAS
% Please wait ...

tic
Y1 = GrB.dnn (W_gb, bias_gb, Y0_gb) ;
gb_time = toc ;
fprintf ('total time in GraphBLAS: %g sec\n', gb_time) ;

%% Solving the sparse deep neural network problem with MATLAB
% Please wait ...

tic
Y2 = dnn_matlab (W, bias, Y0) ;
matlab_time = toc ;
fprintf ('total time in MATLAB:    %g sec\n', matlab_time) ;
fprintf ('Speedup of GraphBLAS over MATLAB: %g\n', ...
    matlab_time / gb_time) ;

err = norm (Y1-Y2,1)

%% For objects, GraphBLAS has better colon notation than MATLAB
% The MATLAB notation C = A (start:inc:fini) is very handy, and
% it works great if A is a MATLAB matrix.  But for objects like
% the GraphBLAS matrix, MATLAB starts by creating the explicit
% index vector I = start:inc:fini.  That's fine if the matrix is
% modest in size, but GraphBLAS can construct huge matrices.
% The problem is that 1:n cannot be explicitly constructed when n
% is huge.
%
% The C API for GraphBLAS can represent the colon notation 
% start:inc:fini in an implicit manner, so it can do the indexing
% without actually forming the explicit list I = start:inc:fini.
% But there is no access to this method using the MATLAB notation
% start:inc:fini.
%
% Thus, to compute C = A (start:inc:fini) for very huge matrices,
% you need to use use a cell array to represent the colon notation,
% as { start, inc, fini }, instead of start:inc:fini. See
% 'help GrB.extract' and 'help.gbsubassign' for, for C(I,J)=A.  The
% syntax isn't conventional, but it is far faster than the MATLAB
% colon notation for objects, and takes far less memory when I is huge.

%%
n = 1e14 ;
H = GrB (n, n) ;            % a huge empty matrix
I = [1 1e9 1e12 1e14] ;
M = magic (4)
H (I,I) = M ;
J = {1, 1e13} ;            % represents 1:1e13 colon notation
C1 = H (J, J)              % computes C1 = H (1:e13,1:1e13)
c = nonzeros (C1) ;
m = nonzeros (M (1:3, 1:3)) ;
assert (isequal (c, m)) ;

%%
try
    % try to compute the same thing with colon
    % notation (1:1e13), but this fails:
    C2 = H (1:1e13, 1:1e13)
catch me
    error_expected = me
end

%% Iterative solvers work as-is
% Many built-in functions work with GraphBLAS matrices unmodified.

A = sparse (rand (4)) ;
b = sparse (rand (4,1)) ;
x = gmres (A,b)
norm (A*x-b)
x = gmres (GrB(A), GrB(b))
norm (A*x-b)

%% ... even in single precision
x = gmres (GrB(A,'single'), GrB(b,'single'))
norm (A*x-b)

%%
% Both of the following uses of minres (A,b) fail to converge because A
% is not symmetric, as the method requires.  Both failures are correctly
% reported, and both the MATLAB version and the GraphBLAS version return
% the same incorrect vector x.

x = minres (A, b)
x = minres (GrB(A), GrB(b))

%%
% With a proper symmetric matrix

A = A+A' ;
x = minres (A, b)
norm (A*x-b)
x = minres (GrB(A), GrB(b))
norm (A*x-b)

%% Extreme performance differences between GraphBLAS and MATLAB.
% The GraphBLAS operations used so far are perhaps 2x to 50x faster than
% the corresponding MATLAB operations, depending on how many cores your
% computer has.  To run a demo illustrating a 500x or more speedup versus
% MATLAB, run this demo:
%
%    gbdemo2
%
% It will illustrate an assignment C(I,J)=A that can take under a second
% in GraphBLAS but several minutes in MATLAB.  To make the comparsion
% even more dramatic, try:
%
%    gbdemo2 (20000)
%
% assuming you have enough memory.  The gbdemo2 is not part of this demo
% since it can take a long time; it tries a range of problem sizes,
% and each one takes several minutes in MATLAB.

%% Sparse logical indexing is much, much faster in GraphBLAS
% The mask in GraphBLAS acts much like logical indexing in MATLAB, but it
% is not quite the same.  MATLAB logical indexing takes the form:
%
%       C (M) = A (M)
%
% which computes the same thing as the GraphBLAS statement:
%
%       C = GrB.assign (C, M, A)
%
% The GrB.assign statement computes C(M)=A(M), and it is vastly faster
% than C(M)=A(M), even if the time to convert the GrB matrix back to a
% MATLAB sparse matrix is included.
%
% GraphBLAS can also compute C (M) = A (M) using overloaded operators
% for subsref and subsasgn, but C = GrB.assign (C, M, A) is a bit faster.
%
% First, both methods in GraphBLAS (both are very fast):

clear
n = 4000 ;
tic
C = sprand (n, n, 0.1) ;
A = 100 * sprand (n, n, 0.1) ;
M = (C > 0.5) ;
t_setup = toc ;
fprintf ('nnz(C): %g, nnz(M): %g, nnz(A): %g\n', ...
    nnz(C), nnz(M), nnz(A)) ;
fprintf ('\nsetup time:     %g sec\n', t_setup) ;

% include the time to convert C1 from a GraphBLAS
% matrix to a MATLAB sparse matrix:
tic
C1 = GrB.assign (C, M, A) ;
C1 = double (C1) ;
gb_time = toc ;
fprintf ('\nGraphBLAS time: %g sec for GrB.assign\n', gb_time) ;

% now using overloaded operators, also include the time to
% convert back to a MATLAB sparse matrix, for good measure:
A2 = GrB (A) ;
C2 = GrB (C) ;
tic
C2 (M) = A2 (M) ;
C2 = double (C2) ;
gb_time2 = toc ;
fprintf ('\nGraphBLAS time: %g sec for C(M)=A(M)\n', gb_time2) ;

%%
% Please wait, this will take about 10 minutes or so ...

tic
C (M) = A (M) ;
matlab_time = toc ;

fprintf ('\nGraphBLAS time: %g sec (GrB.assign)\n', gb_time) ;
fprintf ('\nGraphBLAS time: %g sec (overloading)\n', gb_time2) ;
fprintf ('MATLAB time:    %g sec\n', matlab_time) ;
fprintf ('Speedup of GraphBLAS over MATLAB: %g\n', ...
    matlab_time / gb_time2) ;

% GraphBLAS computes the exact same result with both methods:
assert (isequal (C1, C))
assert (isequal (C2, C))
C1 - C
C2 - C

%% Limitations and their future solutions
% The MATLAB interface for SuiteSparse:GraphBLAS is a work-in-progress.
% It has some limitations, most of which will be resolved over time.
%
% (1) Nonblocking mode:
%
% GraphBLAS has a 'non-blocking' mode, in which operations can be left
% pending and completed later.  SuiteSparse:GraphBLAS uses the
% non-blocking mode to speed up a sequence of assignment operations, such
% as C(I,J)=A.  However, in its MATLAB interface, this would require a
% MATLAB mexFunction to modify its inputs.  That breaks the MATLAB API
% standard, so it cannot be safely done.  As a result, using GraphBLAS
% via its MATLAB interface can be slower than when using its C API.  This
% restriction would not be a limitation if GraphBLAS were to be
% incorporated into MATLAB itself, but there is likely no way to do this
% in a mexFunction interface to GraphBLAS.

%%
% (2) Complex matrices:
%
% GraphBLAS can operate on matrices with arbitrary user-defined types and
% operators.  The only constraint is that the type be a fixed sized
% typedef that can be copied with the ANSI C memcpy; variable-sized types
% are not yet supported.  However, in this MATLAB interface,
% SuiteSparse:GraphBLAS has access to only predefined types, operators,
% and semirings.  Complex types and operators will be added to this
% MATLAB interface in the future.  They already appear in the C version
% of GraphBLAS, with user-defined operators in
% GraphBLAS/Demo/Source/usercomplex.c.

%%
% (3) Integer element-wise operations:
%
% Integer operations in MATLAB saturate, so that uint8(255)+1 is 255.  To
% allow for integer monoids, GraphBLAS uses modular arithmetic instead.
% This is the only way that C=A*B can be defined for integer semirings.
% However, saturating integer operators could be added in the future, so
% that element- wise integer operations on GraphBLAS sparse integer
% matrices could work just the same as their MATLAB counterparts.
%
% So in the future, you could perhaps write this, for both sparse and
% dense integer matrices A and B:
%
%       C = GrB.eadd ('+saturate.int8', A, B)
%
% to compute the same thing as C=A+B in MATLAB for its full int8
% matrices.  Note that MATLAB can do this only for dense integer
% matrices, since it doesn't support sparse integer matrices.

%%
% (4) Faster methods:
%
% Most methods in this MATLAB interface are based on efficient parallel C
% functions in GraphBLAS itself, and are typically as fast or faster than
% the equivalent built-in operators and functions in MATLAB.
%
% There are few notable exceptions; these will be addressed in the future.
% Dense matrices and vectors held as GraphBLAS objects are slower than
% their MATLAB counterparts.  horzcat and vertcat, for [A B] and [A;B]
% when either A or B are GraphBLAS matrices, are also slow, as
% illustrated below in the next example.
%
% Other methods that will be faster in the future include bandwidth,
% istriu, istril, eps, ceil, floor, round, fix, isfinite, isinf, isnan,
% spfun, and A.^B.  These methods are currently implemented in
% m-files, not in efficient parallel C functions.

%%
% Here is an example that illustrates the performance of C = [A B]
clear
A = sparse (rand (2000)) ;
B = sparse (rand (2000)) ;
tic
C1 = [A B] ;
matlab_time = toc ;

A = GrB (A) ;
B = GrB (B) ;
tic
C2 = [A B] ;
gb_time = toc ;

err = norm (C1-C2,1)
fprintf ('\nMATLAB: %g sec, GraphBLAS: %g sec\n', ...
    matlab_time, gb_time) ;
if (gb_time > matlab_time)
    fprintf ('GraphBLAS is slower by a factor of %g\n', ...
        gb_time / matlab_time) ;
end

%%
% (5) Linear indexing:
%
% If A is an m-by-n 2D MATLAB matrix, with n > 1, A(:) is a column vector
% of length m*n.  The index operation A(i) accesses the ith entry in the
% vector A(:).  This is called linear indexing in MATLAB.  It is not yet
% available for GraphBLAS matrices in this MATLAB interface to GraphBLAS,
% but it could be added in the future.

%%
% (6) Implicit singleton dimension expansion 
%
% In MATLAB C=A+B where A is m-by-n and B is a 1-by-n row vector
% implicitly expands B to a matrix, computing C(i,j)=A(i,j)+B(j).  This
% implicit expansion is not yet suported in GraphBLAS with C=A+B.
% However, it can be done with C = GrB.mxm ('+.+', A, diag(GrB(B))).
% That's an nice example of the power of semirings, but it's not
% immediately obvious, and not as clear a syntax as C=A+B.  The
% GraphBLAS/@GrB/dnn.m function uses this 'plus.plus' semiring to
% apply the bias to each neuron.

A = magic (3)
B = 1000:1000:3000
C1 = A + B
C2 = GrB.mxm ('+.+', A, diag (GrB (B)))
err = norm (C1-C2,1)

%% GraphBLAS operations
% In addition to the overloaded operators (such as C=A*B) and overloaded
% functions (such as L=tril(A)), GraphBLAS also has methods of the form
% GrB.method, listed on the next page.  Most of them take an optional
% input matrix Cin, which is the initial value of the matrix C for the
% expression below, an optional mask matrix M, and an optional
% accumulator operator.
%
%      C<#M,replace> = accum (C, T)
%
% In the above expression, #M is either empty (no mask), M (with a mask
% matrix) or ~M (with a complemented mask matrix), as determined by the
% descriptor.  'replace' can be used to clear C after it is used in
% accum(C,T) but before it is assigned with C<...> = Z, where
% Z=accum(C,T).  The matrix T is the result of some operation, such as
% T=A*B for GrB.mxm, or T=op(A,B) for GrB.eadd.
%
% A summary of these GrB.methods is on the next pages.

%% Methods for the GrB class:
%
%   These methods operate on GraphBLAS matrices only, and they overload
%   the existing MATLAB functions of the same name.
%
%   C = GrB (...)           construct a GraphBLAS matrix
%   C = sparse (G)          makes a copy of a GrB matrix
%   C = full (G, ...)       adds explicit zeros or id values to a GrB matrix
%   C = double (G)          cast GrB matrix to MATLAB sparse double matrix
%   C = logical (G)         cast GrB matrix to MATLAB sparse logical matrix
%   C = complex (G)         cast GrB matrix to MATLAB sparse complex
%   C = single (G)          cast GrB matrix to MATLAB full single matrix
%   C = int8 (G)            cast GrB matrix to MATLAB full int8 matrix
%   C = int16 (G)           cast GrB matrix to MATLAB full int16 matrix
%   C = int32 (G)           cast GrB matrix to MATLAB full int32 matrix
%   C = int64 (G)           cast GrB matrix to MATLAB full int64 matrix
%   C = uint8 (G)           cast GrB matrix to MATLAB full uint8 matrix
%   C = uint16 (G)          cast GrB matrix to MATLAB full uint16 matrix
%   C = uint32 (G)          cast GrB matrix to MATLAB full uint32 matrix
%   C = uint64 (G)          cast GrB matrix to MATLAB full uint64 matrix
%   C = cast (G,...)        cast GrB matrix to MATLAB matrix (as above)

%%
%   X = nonzeros (G)        extract all entries from a GrB matrix
%   [I,J,X] = find (G)      extract all entries from a GrB matrix
%   C = spones (G)          return pattern of GrB matrix
%   disp (G, level)         display a GrB matrix G
%   display (G)             display a GrB matrix G; same as disp(G,2)
%   mn = numel (G)          m*n for an m-by-n GrB matrix G
%   e = nnz (G)             number of entries in a GrB matrix G
%   e = nzmax (G)           number of entries in a GrB matrix G
%   [m n] = size (G)        size of a GrB matrix G
%   n = length (G)          length of a GrB vector
%   s = isempty (G)         true if any dimension of G is zero
%   s = issparse (G)        true for any GrB matrix G
%   s = ismatrix (G)        true for any GrB matrix G
%   s = isvector (G)        true if m=1 or n=1, for an m-by-n GrB matrix G
%   s = iscolumn (G)        true if n=1, for an m-by-n GrB matrix G
%   s = isrow (G)           true if m=1, for an m-by-n GrB matrix G
%   s = isscalar (G)        true if G is a 1-by-1 GrB matrix
%   s = isnumeric (G)       true for any GrB matrix G (even logical)
%   s = isfloat (G)         true if GrB matrix is double, single, complex
%   s = isreal (G)          true if GrB matrix is not complex
%   s = isinteger (G)       true if GrB matrix is int8, int16, ..., uint64
%   s = islogical (G)       true if GrB matrix is logical
%   s = isa (G, classname)  check if a GrB matrix is of a specific class

%%
%   C = diag (G,k)          diagonal matrices and diagonals of GrB matrix G
%   L = tril (G,k)          lower triangular part of GrB matrix G
%   U = triu (G,k)          upper triangular part of GrB matrix G
%   C = kron (A,B)          Kronecker product
%   C = repmat (G, ...)     replicate and tile a GraphBLAS matrix
%   C = reshape (G, ...)    reshape a GraphBLAS matrix
%   C = abs (G)             absolute value
%   C = sign (G)            signum function
%   s = istril (G)          true if G is lower triangular
%   s = istriu (G)          true if G is upper triangular
%   s = isbanded (G,...)    true if G is banded
%   s = isdiag (G)          true if G is diagonal
%   s = ishermitian (G)     true if G is Hermitian
%   s = issymmetric (G)     true if G is symmetric
%   [lo,hi] = bandwidth (G) determine the lower & upper bandwidth of G
%   C = sum (G, option)     reduce via sum, to vector or scalar
%   C = prod (G, option)    reduce via product, to vector or scalar
%   s = norm (G, kind)      1-norm or inf-norm of a GrB matrix
%   C = max (G, ...)        reduce via max, to vector or scalar
%   C = min (G, ...)        reduce via min, to vector or scalar
%   C = any (G, ...)        reduce via '|', to vector or scalar
%   C = all (G, ...)        reduce via '&', to vector or scalar

%%
%   C = sqrt (G)            element-wise square root
%   C = eps (G)             floating-point spacing
%   C = ceil (G)            round towards infinity
%   C = floor (G)           round towards -infinity
%   C = round (G)           round towards nearest
%   C = fix (G)             round towards zero
%   C = isfinite (G)        test if finite
%   C = isinf (G)           test if infinite
%   C = isnan (G)           test if NaN
%   C = spfun (fun, G)      evaluate a function on the entries of G
%   p = amd (G)             approximate minimum degree ordering
%   p = colamd (G)          column approximate minimum degree ordering
%   p = symamd (G)          approximate minimum degree ordering
%   p = symrcm (G)          reverse Cuthill-McKee ordering
%   [...] = dmperm (G)      Dulmage-Mendelsohn permutation
%   parent = etree (G)      elimination tree
%   C = conj (G)            complex conjugate
%   C = real (G)            real part of a complex GraphBLAS matrix
%   [V, ...] = eig (G,...)  eigenvalues and eigenvectors
%   assert (G)              generate an error if G is false
%   C = zeros (...,'like',G)   all-zero matrix, same type as G
%   C = false (...,'like',G)   all-false logical matrix
%   C = ones (...,'like',G)    matrix with all ones, same type as G

%% Operator overloading:
%
%   C = plus (A, B)         C = A + B
%   C = minus (A, B)        C = A - B
%   C = uminus (G)          C = -G
%   C = uplus (G)           C = +G
%   C = times (A, B)        C = A .* B
%   C = mtimes (A, B)       C = A * B
%   C = rdivide (A, B)      C = A ./ B
%   C = ldivide (A, B)      C = A .\ B
%   C = mrdivide (A, B)     C = A / B
%   C = mldivide (A, B)     C = A \ B
%   C = power (A, B)        C = A .^ B
%   C = mpower (A, B)       C = A ^ B
%   C = lt (A, B)           C = A < B
%   C = gt (A, B)           C = A > B
%   C = le (A, B)           C = A <= B
%   C = ge (A, B)           C = A >= B
%   C = ne (A, B)           C = A ~= B
%   C = eq (A, B)           C = A == B
%   C = and (A, B)          C = A & B
%   C = or (A, B)           C = A | B
%   C = not (G)             C = ~G
%   C = ctranspose (G)      C = G'
%   C = transpose (G)       C = G.'
%   C = horzcat (A, B)      C = [A , B]
%   C = vertcat (A, B)      C = [A ; B]
%   C = subsref (A, I, J)   C = A (I,J) or C = A (M)
%   C = subsasgn (A, I, J)  C (I,J) = A
%   index = end (A, k, n)   for object indexing, A(1:end,1:end)

%% Static Methods:
%
%   The Static Methods for the GrB class can be used on input matrices of
%   any kind: GraphBLAS sparse matrices, MATLAB sparse matrices, or
%   MATLAB dense matrices, in any combination.  The output matrix Cout is
%   a GraphBLAS matrix, by default, but can be optionally returned as a
%   MATLAB sparse or dense matrix.  The static methods divide into two
%   categories: those that perform basic functions, and the GraphBLAS
%   operations that use the mask/accum.

%% GraphBLAS basic functions:
%
%   GrB.clear                    clear GraphBLAS workspace and settings
%   GrB.descriptorinfo (d)       list properties of a descriptor
%   GrB.unopinfo (op, type)      list properties of a unary operator
%   GrB.binopinfo (op, type)     list properties of a binary operator
%   GrB.monoidinfo (op, type)    list properties of a monoid
%   GrB.semiringinfo (s, type)   list properties of a semiring
%   t = GrB.threads (t)          set/get # of threads to use in GraphBLAS
%   c = GrB.chunk (c)            set/get chunk size to use in GraphBLAS
%   b = GrB.burble (b)           set/get burble (diagnostic output)
%   result = GrB.entries (G,...) count or query entries in a matrix
%   result = GrB.nonz (G,...)    count or query nonzeros in a matrix
%   C = GrB.prune (A, id)        prune entries equal to id
%   C = GrB.offdiag (A)          prune diagonal entries
%   s = GrB.isfull (A)           true if all entries present
%   [C,I,J] = GrB.compact (A,id) remove empty rows and columns
%   G = GrB.empty (m, n)         return an empty GraphBLAS matrix
%   s = GrB.type (A)             get the type of a MATLAB or GrB matrix A
%   s = GrB.issigned (type)      true if type is signed
%   f = GrB.format (f)           set/get matrix format to use in GraphBLAS
%   s = GrB.isbyrow (A)          true if format f A is 'by row'
%   s = GrB.isbycol (A)          true if format f A is 'by col'
%   C = GrB.expand (scalar, A)   expand a scalar (C = scalar*spones(A))
%   C = GrB.eye                  identity matrix of any type
%   C = GrB.speye                identity matrix (of type 'double')
%   C = GrB.build (I, J, X, m, n, dup, type, desc)
%                                build a GrB matrix from list of entries
%   [I,J,X] = GrB.extracttuples (A, desc)
%                                extract all entries from a matrix
%   s = GrB.normdiff (A, B, kind)   norm (A-B,kind)

%% GraphBLAS operations with Cout, mask M, and accum.
%
%   Cout = GrB.mxm (Cin, M, accum, semiring, A, B, desc)
%                   sparse matrix-matrix multiplication over a semiring
%   Cout = GrB.select (Cin, M, accum, op, A, b, desc)
%                   select a subset of entries from a matrix
%   Cout = GrB.assign (Cin, M, accum, A, I, J, desc)
%                   sparse matrix assignment, such as C(I,J)=A
%   Cout = GrB.subassign (Cin, M, accum, A, I, J, desc)
%                   sparse matrix assignment, such as C(I,J)=A
%   Cout = GrB.vreduce (Cin, M, accum, op, A, desc)
%                   reduce a matrix to a vector
%   Cout = GrB.reduce (Cin, accum, op, A, desc)
%                   reduce a matrix to a scalar
%   Cout = GrB.kronecker (Cin, M, accum, op, A, B, desc)
%                   Kronecker product
%   Cout = GrB.trans (Cin, M, accum, A, desc)
%                   transpose a matrix
%   Cout = GrB.eadd (Cin, M, accum, op, A, B, desc)
%                   element-wise addition
%   Cout = GrB.emult (Cin, M, accum, op, A, B, desc)
%                   element-wise multiplication
%   Cout = GrB.apply (Cin, M, accum, op, A, desc)
%                   apply a unary operator
%   Cout = GrB.extract (Cin, M, accum, A, I, J, desc)
%                   extract submatrix, like C=A(I,J) in MATLAB
%
%%
% GraphBLAS operations (with Cout, Cin arguments) take the following form:
%
%   C<#M,replace> = accum (C, operation (A or A', B or B'))
%
%   C is both an input and output matrix.  In this MATLAB interface to
%   GraphBLAS, it is split into Cin (the value of C on input) and Cout
%   (the value of C on output).  M is the optional mask matrix, and #M is
%   either M or !M depending on whether or not the mask is complemented
%   via the desc.mask option.  The replace option is determined by
%   desc.out; if present, C is cleared after it is used in the accum
%   operation but before the final assignment.  A and/or B may optionally
%   be transposed via the descriptor fields desc.in0 and desc.in1,
%   respectively.  To select the format of Cout, use desc.format.  See
%   GrB.descriptorinfo for more details.
%
%   accum is optional; if not is not present, then the operation becomes
%   C<...> = operation(A,B).  Otherwise, C = C + operation(A,B) is
%   computed where '+' is the accum operator.  It acts like a sparse
%   matrix addition (see GrB.eadd), in terms of the structure of the
%   result C, but any binary operator can be used.
%
%   The mask M acts like MATLAB logical indexing.  If M(i,j)=1 then
%   C(i,j) can be modified; if zero, it cannot be modified by the
%   operation.

%% Static Methods for graph algorithms:
%
%   r = GrB.pagerank (A, opts) ;            % PageRank of a matrix
%   C = GrB.ktruss (A, k, check) ;          % k-truss
%   s = GrB.tricount (A, check) ;           % triangle count
%   L = GrB.laplacian (A, type, check) ;    % Laplacian graph
%   C = GrB.incidence (A, ...) ;            % incidence matrix
%   [v, parent] = GrB.bfs (A, s, ...) ;     % breadth-first search
%   iset = GrB.mis (A, check) ;             % maximal independent set
%   Y = GrB.dnn (W, bias, Y0) ;             % deep neural network
%
%   More graph algorithms will be added in the future.
%
% Thanks for watching!
%
% Tim Davis, Texas A&M University, http://faculty.cse.tamu.edu/davis
% See also sparse, doc sparse, and https://twitter.com/DocSparse


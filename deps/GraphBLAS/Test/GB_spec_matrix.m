function Cout = GB_spec_matrix (Cin, identity)
%GB_SPEC_MATRIX a MATLAB mimic that conforms a matrix to the GraphBLAS spec
%
% Cout = GB_spec_matrix (Cin)
% Cout = GB_spec_matrix (Cin, identity)
%
% Conforms an input matrix to the GraphBLAS spec, for use in GB_spec_* only.
%
% The identity argument is the addititive identity of the semiring that will be
% used on the matrix (default is zero).  It represents the value of the
% implicit 'zeros' that are not present in a sparse matrix.
%
% The input Cin is either a matrix or a struct.
%
% If Cin is a struct, it must have a field Cin.matrix, and it can have two
% optional fields, Cin.pattern and Cin.class.  Let X = Cin.matrix.  The class
% of X is given by Cin.class, if present, or class(X) otherwise.  For a dense
% X, class(X) and Cin.class, if present, must match.  If present, the pattern
% of X is given by Cin.pattern; otherwise the pattern for a sparse X is
% GB_spones_mex(X) and entries outside the pattern are assumed to be equal to
% identity.  For a dense X, with no Cin.pattern present the pattern of X is
% just X ~= identity.
%
% Cin is a matrix, then its class is given by class (Cin).  If the matrix is
% sparse, its pattern is GB_spones_mex(Cin) and entries not in the pattern are
% assumed equal to identity.  Otherwise the pattern of Cin is given by Cin ~=
% identity.
%
% The output Cout is a struct with all three fields present (matrix, pattern,
% and class).  Cout.matrix is dense, and it has been typecast into the class
% given by Cout.class.  Entries not in the pattern of X are explicitly set to
% identity.
%
% The matrix is typecast to Cout.class using the typecasting GraphBLAS
% typecasting rules, which differ from MATLAB's rules, particular for integer
% types.  Since MATLAB can only represent 'logical' and 'double' sparse
% matries, the matrix is converted to full.
%
% Converting a matrix from sparse to full is not part of the GraphBLAS spec, of
% course.  Neither is the identity value a part of the matrix structure in
% GraphBLAS.  A matrix in GraphBLAS is always sparse, and entries not in the
% pattern are always implicitly equal to whatever semiring is used on the
% sparse matrix.  As a result, GraphBLAS never needs to know what the identity
% value is until it operates on the matrix, and the identity is not part of the
% matrix itself.  Using a different semiring immediately changes the value of
% the implicit zero, with a change to the GraphBLAS data structure for the
% matrix.
%
% However, MATLAB only supports logical and double sparse matrices, so to
% preserve the class behavior, the MATLAB mimic functions GB_spec_* operate
% only on dense matrices.  When a MATLAB sparse matrix is converted into a
% dense matri , the entries not in the pattern must be set to an explicit
% value: the addititive identity.
%
% The MATLAB mimic routines, GB_spec_* are written almost purely in M, and
% do not call GraphBLAS functions (with the exception of typecasting and
% operators).  They always return a stuct C with a dense C.matrix, a
% pattern C.pattern and class C.class.
%
% GraphBLAS GB_mex_* functions are direct interfaces to the C functions in
% GraphBLAS, and they always return a sparse struct; otherwise large problems
% could not be solved.  The struct contains just C.matrix and C.class.  The
% pattern of C.matrix is GB_spones_mex(C.matrix).  To compare the output C0 of
% a GB_mex_* function with C1 of a GrapBLAS_spec_* function, the struct C0
% must first be passed to this function, C0=GB_spec_matrix(C0,identity) and
% then C0 and C1 should be identical.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% get the semiring addititive identity, if present
if (nargin < 2)
    identity = 0 ;      % default is zero
end

% get the matrix
if (isstruct (Cin))
    X = Cin.matrix ;
else
    X = Cin ;
end

% get the class
if (isstruct (Cin))
    if (isfield (Cin, 'class'))
        % get the class of X from Cin.class
        xclass = Cin.class ;
        if (~issparse (X) && ~isequal (xclass, class (X)))
            % for a dense X, class (X) and Cin.class must match
            error ('class(X) and Cin.class must match for dense X') ;
        end
    else
        % no Cin.class present, so get the class from X itself
        xclass = class (X) ;
    end
else
    % Cin is a matrix, so get the class from X itself
    xclass = class (X) ;
end

% get the pattern
if (isstruct (Cin) && isfield (Cin, 'pattern'))
    xpattern = Cin.pattern ;
else
    % must be done before typecasting since it can introduce zeros
    if (issparse (X))
        % For a sparse matrix, use the actual pattern.  Entries not in the
        % the pattern are assumed to be equal to the addititve identity.
        xpattern = cast (full (GB_spones_mex (X)), 'logical') ;
    else
        xpattern = (X ~= identity) ;
    end
end

% get the uncasted values, if present
if (isstruct (Cin) && isfield (Cin, 'values') &&  ...
    (isequal (xclass, 'int64') || isequal (xclass, 'uint64')))
    % Cin.matrix is sparse, either double or logical.  But it was typecasted
    % from a GraphBLAS matrix that was not double or logical.  Typecasting
    % to int64 or uint64 can lead to loss of precision.  So in this case,
    % use Cin.values instead.
    [m, n] = size (Cin.matrix) ;
    X = zeros (m, n, xclass) ;
    [I J ~] = find (xpattern) ;
    Cx = Cin.values ;
    assert (length (I) == length (Cx)) ;
    for k = 1:length (Cin.values)
        X (I (k), J (k)) = Cx (k) ;
    end
else
    % typecast X to the requested type and make it dense
    X = GB_mex_cast (full (X), xclass) ;
end

% in the dense X, entries not in the xpattern must be set to the identity
X (~xpattern) = GB_mex_cast (identity, xclass) ;

% return the output struct
Cout.matrix = X ;
Cout.pattern = xpattern ;
Cout.class = xclass ;

% The output is now a struct with all 3 fields present, and Cout.matrix
% is always dense.  Cout.class always matches class(Cout.matrix).
assert (isstruct (Cout)) ;
assert (isequal (Cout.class, class (Cout.matrix))) ;
assert (~issparse (Cout.matrix)) ;
% 

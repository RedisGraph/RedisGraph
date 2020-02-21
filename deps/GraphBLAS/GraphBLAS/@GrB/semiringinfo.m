function semiringinfo (s, type)
%GRB.SEMIRINGINFO list the details of a GraphBLAS semiring.
%
% Usage
%
%   GrB.semiringinfo
%   GrB.semiringinfo (semiring)
%   GrB.semiringinfo (semiring, type)
%
% For GrB.semiring(semiring), the semiring must be a string of the form
% 'add.mult.type', where 'add' and 'mult' are binary operators.  The
% second usage allows the type to be omitted from the first argument, as
% just 'add.mult'.  This is valid for all GraphBLAS operations, since the
% type defaults to the type of the input matrices.  However,
% GrB.semiringinfo does not have a default type and thus one must be
% provided, either in the semiring as GrB.semiringinfo ('+.*.double'), or
% in the second argument, GrB.semiringinfo ('+.*', 'double').
%
% The add operator must be a valid monoid: plus, times, min, max, and the
% boolean operators or.logical, and.logical, ne.logical, and xor.logical.
% The binary operator z=f(x,y) of a monoid must be associative and
% commutative, with an identity value id such that f(x,id) = f(id,x) = x.
% Furthermore, the types of x, y, and z for the monoid operator f must
% all be the same.  Thus, the '<.double' is not a valid monoid operator,
% since its 'logical' output type does not match its 'double' inputs, and
% since it is neither associative nor commutative.  Thus, <.*.double is
% not a valid semiring.
%
% Example:
%
%   % valid semirings
%   GrB.semiringinfo ('+.*.double') ;
%   GrB.semiringinfo ('min.1st.int32') ;
%
%   % invalid semiring (generates an error; since '<' is not a monoid)
%   GrB.semiringinfo ('<.*.double') ;
%
% See also GrB.binopinfo, GrB.descriptorinfo, GrB.monoidinfo,
% GrB.selectopinfo, GrB.unopinfo.

% FUTURE: add complex semirings

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    help GrB.semiringinfo
elseif (nargin == 1)
    gbsemiringinfo (s) ;
else
    gbsemiringinfo (s, type) ;
end


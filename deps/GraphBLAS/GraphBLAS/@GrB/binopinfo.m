function binopinfo (op, type)
%GRB.BINOPINFO list the details of a GraphBLAS binary operator.
%
% Usage
%
%   GrB.binopinfo
%   GrB.binopinfo (op)
%   GrB.binopinfo (op, type)
%
% For GrB.binopinfo(op), the op must be a string of the form
% 'op.type', where 'op' is listed below.  The second usage allows the
% type to be omitted from the first argument, as just 'op'.  This is
% valid for all GraphBLAS operations, since the type defaults to the
% type of the input matrices.  However, GrB.binopinfo does not have a
% default type and thus one must be provided, either in the op as
% GrB.binopinfo ('+.double'), or in the second argument, GrB.binopinfo
% ('+', 'double').
%
% The MATLAB interface to GraphBLAS provides for 27 different binary
% operators, each of which may be used with any of the 11 types, for
% a total of 27*11 = 297 valid binary operators.  Binary operators
% are defined by a string of the form 'op.type', or just 'op'.  In
% the latter case, the type defaults to the type of the matrix inputs
% to the GraphBLAS operation.
%
% The 6 comparator operators come in two flavors.  For the is*
% operators, the result has the same type as the inputs, x and y,
% with 1 for true and 0 for false.  For example isgt.double (pi, 3.0)
% is the double value 1.0.  For the second set of 6 operators (eq,
% ne, gt, lt, ge, le), the result is always logical (true or false).
% In a semiring, the type of the add monoid must exactly match the
% type of the output of the multiply operator, and thus
% 'plus.iseq.double' is valid (counting how many terms are equal).
% The 'plus.eq.double' semiring is valid, but not the same semiring
% since the 'plus' of 'plus.eq.double' has a logical type and is thus
% equivalent to 'or.eq.double'.   The 'or.eq' is true if any terms
% are equal and false otherwise (it does not count the number of
% terms that are equal).
%
% The following binary operators are available.  Many have equivalent
% synonyms, so that '1st' and 'first' both define the first(x,y) = x
% operator.
%
%   operator name(s) f(x,y)         |   operator names(s) f(x,y)
%   ---------------- ------         |   ----------------- ------
%   1st first        x              |   iseq             x == y
%   2nd second       y              |   isne             x ~= y
%   min              min(x,y)       |   isgt             x > y
%   max              max(x,y)       |   islt             x < y
%   +   plus         x+y            |   isge             x >= y
%   -   minus        x-y            |   isle             x <= y
%   rminus           y-x            |   ==  eq           x == y
%   *   times        x*y            |   ~=  ne           x ~= y
%   /   div          x/y            |   >   gt           x > y
%   \   rdiv         y/x            |   <   lt           x < y
%   |   || or  lor   x | y          |   >=  ge           x >= y
%   &   && and land  x & y          |   <=  le           x <= y
%   xor lxor         xor(x,y)       |
%   pair             1              |   any              x, or y
%
% The three logical operators, lor, land, and lxor, also come in 11
% types.  z = lor.double (x,y) tests the condition (x~=0) || (y~=0),
% and returns the double value 1.0 if true, or 0.0 if false.
%
% Example:
%
%   % valid binary operators
%   GrB.binopinfo ('+.double') ;
%   GrB.binopinfo ('1st.int32') ;
%
%   % invalid binary operator (an error; this is a unary op):
%   GrB.binopinfo ('abs.double') ;
%
% See also GrB.descriptorinfo, GrB.monoidinfo, GrB.selectopinfo,
% GrB.semiringinfo, GrB.unopinfo.

% FUTURE: add complex binary operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

if (nargin == 0)
    help GrB.binopinfo
elseif (nargin == 1)
    gbbinopinfo (op) ;
else
    gbbinopinfo (op, type) ;
end


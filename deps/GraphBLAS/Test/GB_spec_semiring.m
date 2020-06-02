function [multiply_op add_op identity zclass] = GB_spec_semiring (semiring)
%GB_SPEC_SEMIRING create a semiring
%
% [multiply_op add_op identity zclass] = GB_spec_semiring (semiring)
%
% Given a semiring, extract the multiply operator, additive operator, additive
% identity, and the class of z for z=multiply(...) and the monoid z=add(z,z).
%
% A semiring is a struct with 3 fields, each a string, with defaults used if
% fields are not present.  None of the content of the semiring should be
% a struct.
%
%   multiply    a string with the name of the 'multiply' binary operator
%               (default is 'times').
%
%   add         a string with the name of the 'add' operator (default is 'plus')
%
%   class       the MATLAB class of the operators (default is 'double', unless
%               the multiply operator is 'or', 'and, or 'xor').  Any logical or
%               numeric class is supported, which are the same as the 11
%               built-in GraphBLAS types:
%               'logical' (boolean in GraphBLAS), 'int8', 'uint8', 'int16',
%               'uint16', 'int32', 'uint32', 'int64', 'uint64', 'single' (FP43
%               in GraphBLAS), 'double' (FP64 in GraphBLAS).

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% set the default semiring
if (isempty (semiring))
    semiring = struct ;
end
if (~isfield (semiring, 'multiply'))
    semiring.multiply = 'times' ;
end
if (~isfield (semiring, 'add'))
    semiring.add = 'plus' ;
end
if (~isfield (semiring, 'class'))
    semiring.class = 'double' ;
end

% create the multiply operator.  No error checks; it will be checked later
% and can be any valid GraphBLAS binary operator.
[mult multclass zclass] = GB_spec_operator (semiring.multiply, semiring.class);
multiply_op.opname =  mult ;
multiply_op.opclass = multclass ;

% create the add operator
[add_opname add_opclass] = GB_spec_operator (semiring.add, zclass) ;
add_op.opname = add_opname ;
add_op.opclass = add_opclass ;

% get the identity of the add operator
identity = GB_spec_identity (add_op) ;

switch mult

    % 11, the monoid has the same type as x, y, and z, all semiring.class
    case 'first'      % z = x
         ;
    case 'second'     % z = y
         ;
    case 'pair'       % z = 1
         ;
    case 'min'        % z = min(x,y)
         ;
    case 'max'        % z = max(x,y)
         ;
    case 'plus'       % z = x + y
         ;
    case 'minus'      % z = x - y
         ;
    case 'rminus'     % z = y - x
         ;
    case 'times'      % z = x * y
         ;
    case 'rdiv'       % z = y / x
         ;
    case 'div'        % z = x / y
         ;

    % 6, the monoid has the same type as x, y, and z, all semiring.class
    case 'iseq'         % z = (x == y)
         ;
    case 'isne'         % z = (x != y)
         ;
    case 'isgt'         % z = (x >  y)
         ;
    case 'islt'         % z = (x <  y)
         ;
    case 'isge'         % z = (x >= y)
         ;
    case 'isle'         % z = (x <= y)
         ;

    % 6 ops, the class of x and y are semiring.class, but z logical
    case 'eq'         % z = (x == y)
        ;
    case 'ne'         % z = (x != y)
        ;
    case 'gt'         % z = (x >  y)
        ;
    case 'lt'         % z = (x <  y)
        ;
    case 'ge'         % z = (x >= y)
        ;
    case 'le'         % z = (x <= y)
        ;

    % 3 boolean ops, class of x, y, z are semiring.class (not just logical)
    case 'or'         % z = x || y
        ;
    case 'and'        % z = x && y
        ;
    case 'xor'        % z = x != y
        ;

    otherwise
        error ('invalid multiply op for semiring') ;
end

zbool = isequal (zclass, 'logical') ;

% min, max, plus, times, any monoids: valid for all 11 classes
% or, and, xor, eq monoids:  valid only for logical
switch add_opname
    case 'min'
        ok = 1 ;
    case 'max'
        ok = 1 ;
    case 'plus'
        ok = 1 ;
    case 'times'
        ok = 1 ;
    case 'any'
        ok = 1 ;
    case 'or'
        ok = zbool ;
    case 'and'
        ok = zbool ;
    case 'xor'
        ok = zbool ;
    case 'eq'
        ok = zbool ;
    case 'iseq'
        ok = zbool ;
    otherwise
        ok = false ;
        error ('invalid add monoid for semiring') ;
end

if (~ok)
    error ([add_opname ' operator a valid monoid only for logical case']) ;
end


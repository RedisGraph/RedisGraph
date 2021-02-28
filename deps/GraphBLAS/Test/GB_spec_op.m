function C = GB_spec_op (op, A, B)
%GB_SPEC_OP apply a unary or binary operator
%
% Apply a binary operator z = f (x,y) element-wise to x and y, or a unary
% operator z = f(x) just x.  The operator op
% is any built-in GraphBLAS operator.
%
% op or op.opname is a string with just the operator name.  Valid names of
% binary operators are 'first', 'second', 'min', 'max', 'plus', 'minus',
% 'rminus', 'times', 'div', 'rdiv', 'eq', 'ne', 'gt', 'lt', 'ge', 'le', 'or',
% 'and', 'xor'.  'iseq', 'isne', 'isgt', 'islt', 'isge', 'le', 'pair', 'any'
%
% Unary operators are 'one', 'identity', 'ainv', 'abs', 'minv', and 'not'
%
% op.opclass: 'logical', 'int8', 'uint8', 'int16', 'uint16', 'int32',
%  'uint32', 'int64', 'uint64', 'single', or 'double'
%
% The class of Z is the same as the class of the output of the operator,
% which is op.opclass except for 'eq', 'ne', 'gt', 'lt', 'ge', 'le',
% in which case Z is logical.
%
% Intrinsic MATLAB operators are used as much as possible, so as to test
% GraphBLAS operators.  Some must be done in GraphBLAS because the
% divide-by-zero and overflow rules for integers differs between MATLAB and C.
% Also, typecasting in MATLAB and GraphBLAS differs under underflow and
% overflow conditions.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% get the operator name and class
[opname opclass] = GB_spec_operator (op, class (A)) ;

% cast the inputs A and B to the inputs of the operator
if (~isequal (class (A), opclass))
    x = GB_mex_cast (A, opclass) ;
else
    x = A ;
end

% use MATLAB for integer and logical plus, minus, times, and div
use_matlab = (isa (x, 'float') && ...
    (isequal (opclass, 'single') || isequal (opclass, 'double'))) ;

if (nargin > 2)
    if (~isequal (class (B), opclass))
        y = GB_mex_cast (B, opclass) ;
    else
        y = B ;
    end
    use_matlab = use_matlab && isa (y, 'float') ;
end

switch opname

    % 12 binary operators, result is opclass
    case 'first'
        z = x ;
    case 'second'
        z = y ;
    case 'any'
        z = y ;
    case 'pair'
        z = ones (size (x), opclass) ;
    case 'min'
        % min(x,y) in SuiteSparse:GraphBLAS is min(x,y,'omitnan') in MATLAB.
        % see discussion in SuiteSparse/GraphBLAS/Source/GB.h
        % z = min (x,y,'omitnan') ;
        z = GB_mex_op (op, x, y) ;
    case 'max'
        % z = max (x,y,'omitnan') ;
        z = GB_mex_op (op, x, y) ;
    case 'plus'
        if (use_matlab)
            z = x + y ;
        else
            z = GB_mex_op (op, x, y) ;
        end
    case 'minus'
        if (use_matlab)
            z = x - y ;
        else
            z = GB_mex_op (op, x, y) ;
        end
    case 'rminus'
        if (use_matlab)
            z = y - x ;
        else
            z = GB_mex_op (op, x, y) ;
        end
    case 'times'
        if (use_matlab)
            z = x .* y ;
        else
            z = GB_mex_op (op, x, y) ;
        end
    case 'div'
        if (use_matlab)
            z = x ./ y ;
        else
            z = GB_mex_op (op, x, y) ;
        end
    case 'rdiv'
        if (use_matlab)
            z = y ./ x ;
        else
            z = GB_mex_op (op, x, y) ;
        end

    % 6 binary comparison operators (result is same as opclass)
    case 'iseq'
        z = cast (x == y, opclass) ;
    case 'isne'
        z = cast (x ~= y, opclass) ;
    case 'isgt'
        z = cast (x >  y, opclass) ;
    case 'islt'
        z = cast (x <  y, opclass) ;
    case 'isge'
        z = cast (x >= y, opclass) ;
    case 'isle'
        z = cast (x <= y, opclass) ;

    % 6 binary comparison operators (result is boolean)
    case 'eq'
        z = (x == y) ;
    case 'ne'
        z = (x ~= y) ;
    case 'gt'
        z = (x >  y) ;
    case 'lt'
        z = (x <  y) ;
    case 'ge'
        z = (x >= y) ;
    case 'le'
        z = (x <= y) ;

    % 3 binary logical operators (result is opclass)
    case 'or'
        z = cast ((x ~= 0) | (y ~= 0), opclass) ;
    case 'and'
        z = cast ((x ~= 0) & (y ~= 0), opclass) ;
    case 'xor'
        z = cast ((x ~= 0) ~= (y ~= 0), opclass) ;

    % unary operators (result is opclass)
    case 'one'
        z = cast (1, opclass) ;
    case 'identity'
        z = x ;
    case 'ainv'
        if (use_matlab)
            z = -x ;
        else
            z = GB_mex_op (op, x) ;
        end
    case 'abs'
        if (use_matlab)
            z = abs (x) ;
        else
            z = GB_mex_op (op, x) ;
        end
    case 'minv'
        if (use_matlab)
            z = 1 ./ x ;
        else
            z = GB_mex_op (op, x) ;
        end
    case 'not'
        z = cast (~(x ~= 0), opclass) ;

    otherwise
        opname
        error ('unknown op') ;
end

C = z ;



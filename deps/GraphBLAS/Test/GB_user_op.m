function [z tol] = GB_user_op (op, x, y)
%
%GB_USER_OP apply a complex binary and unary operator
%
% MATLAB equivalents of the GraphBLAS user-defined Complex operators.
% See ../Demo/usercomplex.[ch] and the GB_mex_op mexFunction
%
% [z tol] = GB_user_op (op,x,y) returns tol true if GB_mex_op(op,x,y) is
% allowed to have roundoff error when compared with GB_user_op(op,x,y).  tol is
% false if the result in MATLAB and GraphBLAS should match exactly.
%
% No typecasting is done for user-defined operators.  x,y,z are either
% double complex or double

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

tol = false ;

switch op

    %---------------------------------------------------------------------------
    % binary operators
    %---------------------------------------------------------------------------

    % x,y,z all complex:
    case 'first'
        z = x ;
    case 'second'
        z = y ;
    case 'min'
        z = min (x,y,'includenan') ;
    case 'max'
        z = max (x,y,'includenan') ;
    case 'plus'
        z = x+y ;
    case 'minus'
        z = x-y ;
    case 'times'
        z = x.*y ;
    case 'div'
        % MATLAB doesn't return a complex NaN (Nan+1i*Nan), but the GraphBLAS
        % GB_mex_op mexFunction does.  So if z has any of them, replace them
        % with a complex Nan, just to make sure the tests pass...
        z = x./y ;
        if (any (isnan (z)))
            z (isnan (z)) = complex (nan,nan) ;
        end
        tol = true ;

    % x,y,z all complex:
    case 'iseq'
        z = complex (double (x == y), 0) ;
    case 'isne'
        z = complex (double (x ~= y), 0) ;
    case 'isgt'
        z = complex (double (x >  y), 0) ;
    case 'islt'
        z = complex (double (x <  y), 0) ;
    case 'isge'
        z = complex (double (x >= y), 0) ;
    case 'isle'
        z = complex (double (x <= y), 0) ;

    % x,y,z all complex:
    case 'or'
        z = complex (double ((x ~= 0) | (y ~= 0)), 0) ;
    case 'and'
        z = complex (double ((x ~= 0) & (y ~= 0)), 0) ;
    case 'xor'
        z = complex (double ((x ~= 0) ~= (y ~= 0)), 0) ;

    % x,y complex, z logical:
    case 'eq'
        z = x == y ;
    case 'ne'
        z = x ~= y ;
    case 'gt'
        z = x >  y ;
    case 'lt'
        z = x <  y ;
    case 'ge'
        z = x >= y ;
    case 'le'
        z = x <= y ;

    % x,y double, z comlex:
    case 'complex'
        z = complex (x,y) ;

    %---------------------------------------------------------------------------
    % unary operators
    %---------------------------------------------------------------------------

    % x,z complex
    case 'one'
        [m n] = size (x) ;
        z = complex (ones (m,n),0) ;
    case 'identity'
        z = x ;
    case 'ainv'
        z = -x ;
    case 'abs'
        z = complex (abs (x), 0) ;
        tol = true ;
    case 'minv'
        z = 1./x ;
        if (any (isnan (z)))
            z (isnan (z)) = complex (nan,nan) ;
        end
        tol = true ;
    case 'not'
        z = complex (double (~(x ~= 0)), 0) ;
    case 'conj'
        z = conj (x) ;

    % x complex, z real
    case 'real'
        z = real (x) ;
    case 'imag'
        z = imag (x) ;
    case 'cabs'
        z = abs (x) ;
        tol = true ;
    case 'angle'
        z = angle (x) ;
        tol = true ;

    % x real, z complex
    case 'complex_real'
        z = complex (x,0) ;
    case 'complex_imag'
        z = complex (0,x) ;

    otherwise
        error ('unrecognized complex operator')
end



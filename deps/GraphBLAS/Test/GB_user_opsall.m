function [complex_binaryops complex_unaryops ] = GB_user_opsall
%GB_USER_OPSALL return list of complex operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

[mult_ops unary_ops add_ops classes semirings] = GB_spec_opsall ;

% complex binary operators
complex_binaryops = mult_ops ;
complex_binaryops {end+1} = 'complex' ;

complex_unaryops = {
% 7 where x,z are complex
'one',         % z = 1
'identity',    % z = x
'ainv',        % z = -x
'abs',         % z = complex(abs(x),0)
'minv'         % z = 1/x
'not'          % z = ~x
'conj'         % z = conj(x)
%----------------------------
% 4 where x is complex, z is double
'real'         % z = real(x)
'imag'         % z = imag(x)
'cabs'         % z = cabs(x)
'angle'        % z = angle(x)
%----------------------------
% 2 where x is double, z is complex
'complex_real' % z = complex(x,0)
'complex_imag' % z = complex(0,x)
 } ;


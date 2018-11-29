function [mult_ops unary_ops add_ops classes semirings selops] = GB_spec_opsall
%GB_SPEC_OPSALL return a list of all operators, classes, and semirings
%
% [mult_ops unary_ops add_ops classes semirings select_ops] = GB_spec_opsall

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

mult_ops = {
% 8 operators where x,y,z are all the same class
'first',     % z = x
'second',    % z = y
'min',       % z = min(x,y)
'max',       % z = max(x,y)
'plus',      % z = x + y
'minus',     % z = x - y
'times',     % z = x * y
'div',       % z = x / y
% 6 comparison operators where x,y,z are all the same class
'iseq',      % z = (x == y)
'isne',      % z = (x != y)
'isgt',      % z = (x >  y)
'islt',      % z = (x <  y)
'isge',      % z = (x >= y)
'isle',      % z = (x <= y)
% 3 boolean operators where x,y,z are all the same class
'or',        % z = x || y
'and',       % z = x && y
'xor'        % z = x != y
%----------------------------
% 6 comparison operators where x,y are all the same class, z is logical
'eq',        % z = (x == y)
'ne',        % z = (x != y)
'gt',        % z = (x >  y)
'lt',        % z = (x <  y)
'ge',        % z = (x >= y)
'le',        % z = (x <= y)
} ;


% 6 unary ops for all types, z and x have the same class
unary_ops = {
'one',       % z = 1
'identity',  % z = x
'ainv',      % z = -x
'abs',       % z = abs(x)
'minv',      % z = 1/x
'not'        % z = ~x
} ;

add_ops = {
% 4 monoids for any of 11 classes (including logical)
'min',       % z = min(x,y) : identity is +inf
'max',       % z = max(x,y) : identity is -inf
'plus',      % z = x + y    : identity is 0
'times',     % z = x * y    : identity is 1
%----------------------------
% 4 monoids for just boolean
'or',        % z = x || y   : identity is 0 (false)
'and'        % z = x && y   : identity is 1 (true)
'xor'        % z = x != y   : identity is 0 (false)
'eq'         % z = (x==y)   ; identity is 1 (true)
} ;

% MATLAB classes that correspond to GraphBLAS built-in types
classes = {
'logical'       % GrB_BOOL
'int8'          % GrB_INT8
'uint8'         % GrB_UINT8
'int16'         % GrB_INT16
'uint16'        % GrB_UINT16
'int32'         % GrB_INT32
'uint32'        % GrB_UINT32
'int64'         % GrB_INT64
'uint64'        % GrB_UINT64
'single'        % GrB_FP32
'double' } ;    % GrB_FP64

nonbool = {
'int8' 
'uint8' 
'int16' 
'uint16'
'int32'
'uint32'
'int64'
'uint64'
'single'
'double' } ;

% create all 960 unique semirings using built-in operators

n = 0 ;

% 680: x,y,z all nonboolean:  (8+6+3)*4*10
for mult = {'first', 'second', 'min', 'max', 'plus', 'minus', 'times', 'div',...
            'iseq', 'isne', 'isgt', 'islt', 'isge', 'isle', ...
            'or', 'and', 'xor', }
    for add = { 'min', 'max', 'plus', 'times' }
        for c = nonbool'
            n = n + 1 ;
            s = struct ('multiply', mult{1}, 'add', add{1}, 'class', c{1}) ;
            semirings {n} = s ;
            % fprintf ('%3d %s-%s-%s\n', n, add{1}, mult{1}, c{1}) ;
        end
    end
end

% 240: x,y nonboolean, z boolean: 6 * 4 * 10
for mult = { 'eq', 'ne', 'gt', 'lt', 'ge', 'le' }
    for add = { 'or', 'and', 'xor', 'eq' }
        for c = nonbool'
            n = n + 1 ;
            s = struct ('multiply', mult{1}, 'add', add{1}, 'class', c{1}) ;
            semirings {n} = s ;
            % fprintf ('%3d %s-%s-%s\n', n, add{1}, mult{1}, c{1}) ;
        end
    end
end

% 40: x,y,z all boolean: 10 * 4
for mult = { 'first', 'second', 'or', 'and', 'xor', 
             'eq', 'gt', 'lt', 'ge', 'le' }
    for add = { 'or', 'and', 'xor', 'eq' }
        n = n + 1 ;
        s = struct ('multiply', mult{1}, 'add', add{1}, 'class', c{1}) ;
        semirings {n} = s ;
        % fprintf ('%3d %s-%s-logical\n', n, add{1}, mult{1}) ;
    end
end

selops = { 'tril', 'triu', 'diag', 'offdiag', 'nonzero' } ;

% fprintf ('semirings: %d\n', n) ;


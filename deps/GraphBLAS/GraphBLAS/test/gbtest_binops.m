function [binops, synonyms] = gbtest_binops
%GBTEST_BINOPS return a cell array of strings, listing all binary operators
% Types are not included; see gbtest_types.
%
% [binops synonyms] = gbtest_binops ;
%
% returns a list of the names of the operators in binops, and a list of
% their synonyms in the 2nd output.
%
% See also GrB.binopinfo.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

binops = {
    '1st'
    '2nd'
    'pair'
    'any'
    'min'
    'max'
    '+'
    '-'
    'rminus'
    '*'
    '/'
    '\'
    'iseq'
    'isne'
    'isgt'
    'islt'
    'isge'
    'isle'
    '=='
    '~='
    '>'
    '<'
    '>='
    '<='
    '|'
    '&'
    'xor'
    } ;

synonyms = {
    'first'
    'second'
    'plus'
    'times'
    'rdiv'
    'div'
    'minus'
    'or'
    'lor'
    'and'
    'land'
    'lxor'
    '||'
    '&&'
    'eq'
    'ne'
    'ge'
    'le'
    'lt'
    'gt' } ;


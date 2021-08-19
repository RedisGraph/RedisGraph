function [binops, synonyms] = gbtest_binops
%GBTEST_BINOPS list of all binary operators
% Types are not included; see gbtest_types.
%
% [binops synonyms] = gbtest_binops ;
%
% returns a list of the names of the operators in binops, and a list of
% their synonyms in the 2nd output.
%
% See also GrB.binopinfo.

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: GPL-3.0-or-later

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
    'atan2'
    'hypot'
    'fmod'
    'remainder'
    'copysign'
    'cmplx'
    'pow2'
    'xnor'
    'pow'
    'bitor'
    'bitand'
    'bitxor'
    'bitxnor'
    'firsti0'
    'firsti1'
    'firstj0'
    'firstj1'
    'secondi0'
    'secondi1'
    'secondj0'
    'secondj1'
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
    'gt'
    'lxnor' } ;


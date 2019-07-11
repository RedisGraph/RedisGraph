% SuiteSparse/GraphBLAS/Source
%
% These files are used to create the files in Source/Generated, from the
% input files in Source/Generator.  These functions do not need to be used by
% the user.
%
%   codegen                      - generate all code for Generated/*
%   codegen_axb                  - create all C=A*B functions for all semirings
%   codegen_axb_method           - create a function to compute C=A*B over a semiring
%   codegen_axb_template         - create a function for a semiring with a TxT->T multiplier
%   codegen_axb_compare_template - create a function for a semiring with a TxT->bool multiplier

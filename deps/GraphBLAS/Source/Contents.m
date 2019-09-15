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
%   codegen_binop                - create functions for all binary operators
%   codegen_binop_method         - create a function to compute C=binop(A,B)
%   codegen_binop_template       - create binop functions
%   codegen_red                  - create functions for all reduction operators
%   codegen_red_method           - create a reduction function, C = reduce (A)
%   codegen_sel                  - create functions for all selection operators
%   codegen_sel_method           - create a selection function, C = select (A,thunk)
%   codegen_type                 - determine C type name, signed, and # bits a type
%   codegen_unop                 - create functions for all unary operators
%   codegen_unop_method          - create a function to compute C=unop(cast(A))
%   codegen_unop_template        - create unop functions

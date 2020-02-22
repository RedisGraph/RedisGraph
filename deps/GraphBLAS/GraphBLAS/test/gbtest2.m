function gbtest2
%GBTEST2 list all binary operators

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

optype = gbtest_types ;
opnames = gbtest_binops ;

for k1 = 1:length(opnames)

    opname = opnames {k1} ;
    fprintf ('\n=================================== %s\n', opname) ;

    for k2 = 0:length(optype)

        op = opname ;
        if (k2 > 0)
            op = [op '.' optype{k2}] ; %#ok<*AGROW>
        end
        fprintf ('\nop: [%s]\n', op) ;
        if (k2 > 0)
            GrB.binopinfo (op)
        else
            GrB.binopinfo (op, 'double')
        end
    end
end

fprintf ('\nhelp GrB.binopinfo:\n') ;
GrB.binopinfo ;

fprintf ('gbtest2: all tests passed\n') ;


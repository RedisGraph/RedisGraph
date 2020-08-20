function [C err] = kron_demo (A,B)
%KRON_DEMO test Program/kron_demo.c and compare with MATLAB kron
% Usage:
% [C err] = kron_demo (A,B)

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

% write A to a file
Afile = fopen ('A.tsv', 'w') ;
[i j x] = find (A) ;
fprintf (Afile, '%d\t%d\t%0.17g\n', [i j x]') ;
fclose (Afile) ;

% write B to a file
Bfile = fopen ('B.tsv', 'w') ;
[i j x] = find (B) ;
fprintf (Bfile, '%d\t%d\t%0.17g\n', [i j x]') ;
fclose (Bfile) ;

% run kron_demo
system ('../../build/kron_demo A.tsv B.tsv C.tsv') ;

% read C from file
load C.tsv ;
C = spconvert (C) ;

% compare with MATLAB
C2 = kron (A, B) ;

err = norm (C-C2, 1) / (norm (A,1) + norm (B,1)) ;
if (err > 1e-14)
    error ('invalid Kronecker product') ;
end

delete A.tsv
delete B.tsv
delete C.tsv

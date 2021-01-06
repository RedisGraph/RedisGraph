function gbtest51
%GBTEST51 test GrB.tricount

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
% SPDX-License-Identifier: Apache-2.0

files =  {
'../../Demo/Matrix/2blocks'
'../../Demo/Matrix/ash219'
'../../Demo/Matrix/bcsstk01'
'../../Demo/Matrix/bcsstk16'
'../../Demo/Matrix/eye3'
'../../Demo/Matrix/fs_183_1'
'../../Demo/Matrix/ibm32a'
'../../Demo/Matrix/ibm32b'
'../../Demo/Matrix/lp_afiro'
'../../Demo/Matrix/mbeacxc'
'../../Demo/Matrix/t1'
'../../Demo/Matrix/t2'
'../../Demo/Matrix/west0067' } ;
nfiles = length (files) ;

% the files in ../../Demo/Matrix that do not have a .mtx filename
% are zero-based.
desc.base = 'zero-based' ;

valid_count = [
           0
           0
         160
     1512964
           0
         863
           0
           0
           0
           0
           2
           0
         120 ] ;

[filepath, name, ext] = fileparts (mfilename ('fullpath')) ; %#ok<*ASGLU>

for k = 1:nfiles
    % fprintf ('--------------------------load file:\n') ;
    filename = files {k} ;
    T = load (fullfile (filepath, filename)) ;
    nz = size (T, 1) ;
    X = ones (nz,1) ;
    G = GrB.build (int64 (T (:,1)), int64 (T (:,2)), X, desc) ;
    A = sparse (T (:,1)+1, T (:,2)+1, X) ;
    assert (isequal (A,G))

    % fprintf ('--------------------------construct G:\n') ;
    [m, n] = size (G) ;
    if (m ~= n)
        G = [GrB(m,m) G ; G' GrB(n,n)] ; %#ok<*AGROW>
    elseif (~issymmetric (G))
        G = G + G' ;
    end

    % fprintf ('--------------------------tricount (G):\n') ;
    c = GrB.tricount (G) ;
    % fprintf ('triangle count: %-30s : # triangles %d\n', filename, c) ;
    assert (c == valid_count (k)) ;

    % fprintf ('--------------------------convert G to by-row:\n') ;
    G = GrB (G, 'by row') ;

    % fprintf ('--------------------------tricount (G):\n') ;
    c = GrB.tricount (G) ;
    assert (c == valid_count (k)) ;
end

% fprintf ('--------------------------tricount (G, ''check''):\n') ;
c = GrB.tricount (G, 'check') ;
assert (c == valid_count (end)) ;

fprintf ('\ngbtest51: all tests passed\n') ;


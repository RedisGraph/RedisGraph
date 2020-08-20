function gbtest51
%GBTEST51 test GrB.tricount

% SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
% http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
    filename = files {k} ;
    T = load (fullfile (filepath, files {k})) ;
    G = GrB.build (int64 (T (:,1)), int64 (T (:,2)), T (:,3), desc) ;
    [m, n] = size (G) ;
    if (m ~= n)
        G = [GrB(m,m) G ; G' GrB(n,n)] ; %#ok<*AGROW>
    elseif (~issymmetric (G))
        G = G + G' ;
    end
    c = GrB.tricount (G) ;
    fprintf ('triangle count: %-30s : # triangles %d\n', filename, c) ;
    assert (c == valid_count (k)) ;

    G = GrB (G, 'by row') ;
    c = GrB.tricount (G) ;
    assert (c == valid_count (k)) ;
end

c = GrB.tricount (G, 'check') ;
assert (c == valid_count (end)) ;

fprintf ('\ngbtest51: all tests passed\n') ;


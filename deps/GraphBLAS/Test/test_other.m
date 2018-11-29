function test_other
%TEST_OTHER installs all packages needed for extensive tests

here = pwd ;
fprintf ('\n------------------installing ssget:\n') ;
try
    index = ssget ;
catch
    cd ../../ssget
    addpath (pwd) ;
end
cd (here) ;

fprintf ('\n------------------installing GraphBLAS/Demo/MATLAB:\n') ;
addpath ../Demo/MATLAB

fprintf ('\n------------------installing spok:\n') ;
cd spok
addpath (pwd) ;
try
    spok (sparse (1)) ;
catch
    spok_install ;
end
cd (here) ;

fprintf ('\n------------------installing SSMULT:\n') ;
cd ../../MATLAB_Tools/SSMULT
addpath (pwd) ;
try
    L = sparse (1) ;
    ssmultsym (L, L) ;
catch
    ssmult_install
end
cd (here) ;

fprintf ('\n------------------installing CXSparse:\n') ;
cd ../../CXSparse/MATLAB/Csparse
addpath (pwd) ;
try
    cs_sparse (1, 1, 1) ;
catch
    cs_make (1) ;
end
cd (here) ;

fprintf ('\n------------------installing CHOLMOD:\n') ;
cd ../../CHOLMOD/MATLAB
addpath (pwd) ;
try
    sparse2 (1, 1, 1) ;
catch
    cholmod_make ;
end
cd (here) ;


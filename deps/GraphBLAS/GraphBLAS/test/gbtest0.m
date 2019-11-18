function gbtest0
%GBTEST0 test GrB.clear

GrB.clear

assert (isequal (GrB.format, 'by col')) ;
assert (isequal (GrB.chunk, 4096)) ;

fprintf ('default # of threads: %d\n', GrB.threads) ;

fprintf ('gbtest0: all tests passed\n') ;


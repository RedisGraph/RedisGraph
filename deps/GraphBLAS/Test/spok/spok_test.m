function spok_test
%SPOK_TEST installs and tests SPOK
%
% Example:
%   spok_install
%
% See also sparse, spok, spok_install

% Copyright 2008-2011, Timothy A. Davis, http://www.suitesparse.com

% compile and install spok
help spok ;
spok_install ;
c = pwd ;
cd private ;

% mex spok_invalid.c ;
is64 = ~isempty (strfind (computer, '64')) ;
if (is64)
    mex -largeArrayDims spok_invalid.c
else
    mex spok.c spok_invalid.c
end

cd (c) ;

% test with valid matrices
fprintf ('\nTesting spok, please wait ...\n') ;
lastwarn ('') ;
test_spok (sparse ([ ]), 1, '') ;
test_spok (sparse (logical ([ ])), 1, '') ;
test_spok (sparse (0,4), 1, '') ;
test_spok (sparse (4,4), 1, '') ;

for trials = 1:2
    for m = 0:10
        for n = 0:10
            for d = 0:.1:1
                A = sprand (m,n,d) ;
                B = sprand (m,n,d) ;
                test_spok (A, 1, '') ;
                test_spok (A + 1i*B, 1, '') ;
                test_spok (A > 0, 1, '') ;
            end
        end
    end
end

% test with non-sparse matrices
fprintf ('\nTesting on non-sparse matrices; 7 warnings should appear:\n') ;
test_spok ('hi',          1, 'SPOK:NotSparse') ;
test_spok (cell (42),     1, 'SPOK:NotSparse') ;
test_spok ([ ],           1, 'SPOK:NotSparse') ;
test_spok (ones (10),     1, 'SPOK:NotSparse') ;
test_spok (ones (10) > 0, 1, 'SPOK:NotSparse') ;
test_spok (ones (0,10),   1, 'SPOK:NotSparse') ;
test_spok (ones (10,0),   1, 'SPOK:NotSparse') ;

% test with an invalid matrix
fprintf ('\nTesting on invalid sparse matrices; 2 warnings should appear:\n') ;
test_spok (spok_invalid (0), 0, 'SPOK:QuestionableMatrix') ;
test_spok (spok_invalid (1), 0, 'SPOK:QuestionableMatrix') ;

fprintf ('\nAll tests passed.\n') ;

%-------------------------------------------------------------------------------
function test_spok (A, ok_expected, id_expected)
%TEST_SPOK tests spok and checks its result
lastwarn ('') ;
ok = spok (A) ;
[msg id] = lastwarn ;
if (ok ~= ok_expected || ~strcmp (id, id_expected))
    lastwarn
    error ('test failure') ;
end

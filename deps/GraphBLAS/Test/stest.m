% test GxB_select
clear all
make

for k = [1 2 4 8 ] % 16 32]
    nthreads_set (k)

    debug_on
    gb
    stat

    test129 %GxB_select (tril and nonzero, hypersparse)
    test134 % shorter version of test25
    test27  % band, for user-defined
    test26  % performance test
    test76  % resize
    test25  % built-in, exhaustive test

    debug_off
    gb
    stat

    test129 %GxB_select (tril and nonzero, hypersparse)
    test134 % shorter version of test25
    test27  % band, for user-defined
    test26  % performance test
    test76  % resize
    test25  % built-in, exhaustive test

end


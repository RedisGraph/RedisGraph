% test GxB_select
clear all
make

for k = [1 2 4 8 ] % 16 32]
    nthreads_set (k)

    debug_off
    gb
    stat
    test25  % built-in, exhaustive test
    test27  % band, for user-defined
    test26  % performance test
    test76  % resize

    debug_on
    gb
    stat
    test25
    test27
    test26
    test76
end


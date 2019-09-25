SuiteSparse/GraphBLAS/alternative/README.txt

CMake is prefered for compiling SuiteSparse/GraphBLAS, but this folder provides
a simple alternative, if you don't have CMake or if you prefer a simple
Makefile build process.  The Makefile does not install any user-defined objects
in ../User/*.m4, and it will typically require modifications before it will
work on your system.  It assumes you have the Intel compiler (icc), but this
can easily be changed by editting the Makefile.  See the ifeq (UNAME...)
section in the Makefile to customize the settings for Linux or the Mac.

Files:

    Makefile
    README.txt
    altdemo

to compile the dynamic library:       make
to compile the static library:        make static
to compile in parallel with 4 cores:  make -j4
to install in /usr/local/*:           make install
to cleanup:                           make distclean
to compile and run the demos:         make run


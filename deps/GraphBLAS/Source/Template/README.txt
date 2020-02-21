SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

This is the GraphBLAS/Source/Template folder.

These files are not stand-alone, but are #include'd in files in Source/*.c,
Source/GB.h, Source/Generator/*.c, and Source/Generated/*.c, to generate code.

Files with the name GB_*_factory.c are "switch factories", where the
#include'ing file #define's a GB_WORKER macro that is then specialized by the
*factory.c code to generate workers for different semirings, types, and
operators.  Some of the *factory.c files are two-level, where one *factory.c
file #include's another one from this same folder.


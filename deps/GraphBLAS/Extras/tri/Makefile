# Makefile for tri package (triangle counting)

# CC = gcc-6        # Mac, with OpenMP
  CC = xlc          # IBM Minsky
# CC = gcc          # linux
# CC = icc          # Intel compiler

CF = -O3 -fexceptions -fPIC -std=c11 -fopenmp
LIB = -lm -fopenmp

ifeq ($(UNAME),Linux)
    # add the realtime library, librt
    # this might not be needed anymore
    LIB += -lrt
endif

ifeq ($(UNAME), Darwin)
    # To compile on the Mac with clang. First install Xcode.  Then do this at
    # the command line in the Terminal, before doing 'make':
    # xcode-select --install
    # This is needed for clang for some of my codes but perhaps not this one...
    CF += -fno-common
endif

INC = tri_def.h Makefile

run: tri_main
	./tri_main < bcsstk01

tri_main: tri_functions.o tri_main.o tri_prep.o tri_read.o tri_simple.o
	$(CC) $(CF) $^ -o $@ $(LIB)

tri_functions.o: tri_functions.c tri_template.c tri_dot_template.c $(INC)
	$(CC) $(CF) -c $<

tri_main.o: tri_main.c $(INC)
	$(CC) $(CF) -c $<

tri_prep.o: tri_prep.c $(INC)
	$(CC) $(CF) -c $<

tri_read.o: tri_read.c $(INC)
	$(CC) $(CF) -c $<

tri_simple.o: tri_simple.c $(INC)
	$(CC) $(CF) -c $<

clean: distclean

purge: distclean

distclean:
	- $(RM) tri_main *.o


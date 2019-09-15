# compile 4 main programs: *_main

# select your compiler
# CC = gcc-6    # Mac
  CC = xlc      # IBM Minsky system
# CC = gcc
# CC = cc

CF = -O3 -fexceptions -fPIC -std=c11 -fopenmp

LIB = -L/usr/local/lib -lgraphblas ../../build/libgraphblasdemo_static.a -lm
  I = -I../../Include -I../../Demo/Include 

%.o: %.c $(INC)
	$(CC) $(CF) $(I) -c $<

# default: compiler all 4 main programs and run each of them
all: ktruss_main ktruss_graphblas_main allktruss_main allktruss_graphblas_main
	./ktruss_main < bcsstk01
	./allktruss_main < bcsstk01
	./ktruss_graphblas_main 1 < bcsstk01
	./allktruss_graphblas_main 1 < bcsstk01

# construct a k-truss (pure C, with OpenMP)
ktruss_main: ktruss_main.o ktruss_read.o ktruss.o ktruss_ntriangles.o
	$(CC) $(CF) $^ -o $@ -lm

# construct all k-trusses (pure C, with OpenMP)
allktruss_main: allktruss_main.o ktruss_read.o allktruss.o ktruss_ntriangles.o
	$(CC) $(CF) $^ -o $@ -lm

# construct a k-truss (GraphBLAS)
ktruss_graphblas_main: ktruss_graphblas_main.c ktruss_graphblas.c
	$(CC) $(CF) $(I) $^ -o $@ $(LIB)

# construct all k-trusses (GraphBLAS)
allktruss_graphblas_main: allktruss_graphblas_main.c allktruss_graphblas.c
	$(CC) $(CF) $(I) $^ -o $@ $(LIB)

prog: ktruss_main ktruss_graphblas_main allktruss_main allktruss_graphblas_main

clean:
	- $(RM) -r *.o

purge: distclean

distclean: clean
	- $(RM) -r *_main *.mex* 


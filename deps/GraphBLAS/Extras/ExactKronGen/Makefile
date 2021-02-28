test:  kron
	./kron a.tsv b.tsv c.tsv
	./kron a.tsv b.tsv c_btype1.tsv 1 0 1
	./kron a.tsv b.tsv c_btype2.tsv 1 0 2

kron: kron.c kron_submatrix.c read_tuples.c kron.h simple_timer.c simple_timer.h
	$(CC) -o kron kron.c kron_submatrix.c read_tuples.c simple_timer.c \
		-lgraphblas

kron_mpi: kron_mpi.c kron_submatrix.c read_tuples.c kron.h simple_timer.c \
	simple_timer.h
	mpicc -o kron_mpi kron_mpi.c kron_submatrix.c read_tuples.c \
		simple_timer.c -lgraphblas

mpi: kron_mpi
	mpirun -np 4 ./kron_mpi a.tsv b.tsv e.tsv
	cat *_e.tsv | sort -n > e_all.tsv
	sort -n corig.tsv > c_all.tsv
	diff e_all.tsv c_all.tsv

distclean: purge

purge: clean
	$(RM) *_C.tsv Csort?.tsv *_e.tsv e_all.tsv c_btype?.tsv C.tsv c_all.tsv e.tsv c.tsv

clean:
	$(RM) *.o kron kron_mpi


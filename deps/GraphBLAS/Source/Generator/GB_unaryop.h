GrB_Info GB_unop
(
    GB_ctype *restrict Cx,
    const GB_atype *restrict Ax,
    int64_t anz,
    int nthreads
) ;

GrB_Info GB_tran
(
    GrB_Matrix C,
    const GrB_Matrix A,
    int64_t **Rowcounts,
    GBI_single_iterator Iter,
    const int64_t *restrict A_slice,
    int naslice
) ;


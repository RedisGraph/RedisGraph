/** \file headers_for_doxygen.h
\brief auto-generated headers for doxygen
constructed by dox_headers.m
*/


/** \file GB.h
\brief  GB.h: definitions visible only inside GraphBLAS

\par
 These defintions are not visible to the user.  They are used only inside
 GraphBLAS itself.
*/


/** \file GB_AxB_Gustavson.c
\brief  GB_AxB_Gustavson: C=A*B or C\<M\>=A*B, gather/scatter-based saxpy method.

\par
 This method is agnostic to the CSR/CSC format.  The format of C is set
 to CSC but this is a placeholder that will be changed in GB_AxB_meta.
\par
 parallel: this work is done by a single thread.  parallelism will be done in
 GB_AxB_parallel.
\par
 Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.
*/


/** \file GB_AxB_Gustavson_builtin.c
\brief  GB_AxB_Gustavson_builtin:  hard-coded C=A*B for built-in types

\par
 This function computes C=A*B with hard-coded versions for all 960 unique
 built-in semirings that can be constructed with built-in operators.  It also
 handles all non-unique built-in semirings, by renaming operators to
 equivalent ones; if these are included, this function computes C=A*B for all
 1712 valid semirings that can be constructed from built-in operators.
\par
 parallel: this work is done by a single thread; parallelism will be done in
 GB_AxB_parallel.
*/


/** \file GB_AxB_alloc.c
\brief  GB_AxB_alloc: estimate nnz(C) and allocate C for C=A*B or C=A'*B

\par
 parallel: this function will remain sequential.
 parallelism will be done in GB_AxB_parallel.
\par
 Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.
*/


/** \file GB_AxB_dot.c
\brief  GB_AxB_dot: compute C\<M\> = A'*B without forming A' via dot products

\par
 GB_AxB_dot computes the matrix multiplication C\<M\>=A'*B without forming
 A' explicitly.  It is useful when A is very tall and thin (n-by-1 in
 particular).  In that case A' is costly to transpose, but A'*B is very
 easy if B is also tall and thin (say also n-by-1).
\par
 If M is NULL, the method computes C=A'*B by considering each entry C(i,j),
 taking O(m*n) time if C is m-by-n.  This is suitable only when C is small
 (such as a scalar, a small matrix, or a vector).  If M is present, the upper
 bound on the number of entries in C is the same as nnz(M), so that space is
 allocated for C, and C(i,j) is computed only where M(i,j)=1.  This function
 assumes the mask M is not complemented.
\par
 Compare this function with GB_AxB_Gustavson, which computes C=A*B and
 C\<M\>=A*B.  The computation of C=A*B requires C-\>p and C-\>i to be constructed
 first, in a symbolic phase.  Otherwise they are very similar.  The dot
 product in this algorithm is very much like the merge-add in GB_add,
 except that the merge in GB_add produces a column (a(:,j)+b(:,j)),
 whereas the merge in this function produces a scalar (a(:,j)'*b(:,j)).
\par
 parallel: this function will remain sequential.
 parallelism will be done in GB_AxB_parallel.
\par
 Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.
*/


/** \file GB_AxB_flopcount.c
\brief  GB_AxB_flopcount:  compute flops for C\<M\>=A*B or C=A*B

\par
 On input, A and B are two matrices for C\<M\>=A*B or C=A*B.  The flop count
 for each B(:,j) is computed, and returned as a cumulative sum.  This
 function is CSR/CSC agnostic, but for simplicity of this description, assume
 A and B are both CSC matrices, so that ncols(A) == nrows(B).  For both CSR
 and CSC, A-\>vdim == B-\>vlen holds.  A and/or B may be hypersparse, in any
 combination.
\par
 The complemented mask is not handled, so the flops for C\<!M\>=A*B is not
 computed.
\par
 If present, Bflops has size (B-\>nvec)+1, for both standard and hypersparse
 B.  Let n = B-\>vdim be the column dimension of B (that is, B is m-by-n).
\par
 If B is a standard CSC matrix then Bflops has size n+1 == B-\>nvec+1, and on
 output, Bflops [j] is the \# of flops required to compute C (:, 0:j-1).  B-\>h
 is NULL, and is implicitly the vector 0:(n-1).
\par
 If B is hypersparse, then let Bh = B-\>h.  Its size is B-\>nvec, and j = Bh
 [kk] is the (kk)th column in the data structure for B.  C will also be
 hypersparse, and only C(:,Bh) will be computed (C may have fewer non-empty
 columns than B).  On output, Bflops [kk] is the number of needed flops to
 compute C (:, Bh [0:kk-1]).
\par
 In both cases, Bflops [0] = 0, and Bflops [B-\>nvec] = total number of flops.
 The size of Bflops is B-\>nvec+1 so that it has the same size as B-\>p.  The
 first entry of B-\>p and Bflops are both zero.  This allows B to be sliced
 either by \# of entries in B (by slicing B-\>p) or by the flop count required
 (by slicing Bflops).
\par
 This algorithm does not look at the values of M, A, or B, just their
 patterns.  If the mask is present, it is assumed to not be complemented.
 The flop count of C=A*B or C\<M\>=A*B is computed for a saxpy-based method;
 the work for A'*B for the dot product method is not computed.
\par
 The algorithm scans all nonzeros in B.  It only scans at most the min and
 max (first and last) row indices in A and M (if M is present).  If A and M
 are not hypersparse, the time taken is O(nnz(B)+n).  If all matrices are
 hypersparse, the time is O(nnz(B)*log(h)) where h = max \# of vectors present
 in A and M.  In pseudo-MATLAB, and assuming B is in standard (not
 hypersparse) form:
*/


/** \file GB_AxB_heap.c
\brief  GB_AxB_heap: compute C\<M\> = A*B using a heap-based method

\par
 parallel: this function will remain sequential.
 parallelism will be done in GB_AxB_parallel.
\par
 FUTURE: reduce compiled code size by changing GB_heap.h to GB_heap.c;
 check any performance impact
\par
 Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.
*/


/** \file GB_AxB_meta.c
\brief  GB_AxB_meta: C\<M\>=A*B meta algorithm

\par
 C or C\<M\> = A*B, A'*B, A*B', or A'*B' : both symbolic and numeric, with the
 optional mask matrix.  This function is called by GB_mxm only.  If the mask
 matrix is present, it is not complemented, since this function can only
 handle a non-complemented mask matrix.  A complemented mask is handled in
 GB_accum_mask, after this matrix C is computed, in GB_mxm.  The result of
 this matrix is the T matrix in GB_mxm.
\par
 The method is chosen automatically:  a gather/scatter saxpy method
 (Gustavson), a heap-based saxpy method, or a dot product method.
\par
 FUTURE: an outer-product method for C=A*B'
 FUTURE: a hash-based method for C=A*B
\par
 parallel: this function will remain sequential.
 parallelism will be done in GB_AxB_parallel and GB_transpose.
*/


/** \file GB_AxB_parallel.c
\brief  GB_AxB_parallel: C\<M\>=A*B, C\<M\>=A'*B, C=A*B, or C=A'*B

\par
 Parallel matrix-matrix multiply, A*B or A'*B, with optional mask M.  This
 method is used by GrB_mxm, GrB_vxm, and GrB_mxv.  For both of the latter two
 methods, B on input will be an nrows-by-1 column vxector.

 If do_adotb is true, then A'*B is being computed.  In this case, A has
 not been transposed yet (and will not be).  A and B must have the same
 vector length, vlen (as if both A and B are CSC matrices with the same
 number of rows, for example).  The GB_AxB_sequential method when applied
 to A and B (or a slice of B) will operate on A' without forming it.

 If do_adotb is false, then A*B is being computed, and the vector dimension
 of A must be identical to the vector length of B (as if both A and B are
 CSC matrices, and the number of columns of A is the same as the number of
 rows of B).

 The output matrix C = *Chandle has not been allocated, so C is NULL on
 input.  The mask M is optional.  If present, it is not complemented.

 The semiring defines C=A*B, and must be valid.  flipxy modifies how the
 semiring multiply operator is applied.  If false, then fmult(aik,bkj)
 is computed.  If true, then the operands are swapped, and fmult(bkj,aij)
 is done instead.

 AxB_method selects the method to use:
      GxB_DEFAULT:        the method is selected automatically
      GxB_AxB_GUSTAVSON:  Gustavson's method for A*B
      GxB_AxB_HEAP:       heap method for A*B
      GxB_AxB_DOT:        dot method for A'*B
      GxB_AxB_HASH:       hash method for A*B (FUTURE)

 AxB_method_used reports the method actually chosen.  This is for
 informational purposes only, so if a parallel C=A*B splits the work into
 multiple submatrix multiplications, and uses different methods on each
 submatrix, then AxB_method_used is the method chosen by thread zero.

 Context: the GB_Context containing the \# of threads to use, a string of the
 user-callable function that is calling this function (GrB_mxm, GrB_mxv, or
 GxB_vxm) and detailed error reports.

 C, M, A, and B are CSR/CSC agnostic.  For this discussion, suppose they
 are CSC, with vlen = \# of rows, and vdim = \# of columns.
\par
 PARALLEL: A' or B is sliced, and the slices of C are concatenated.
\par
 FUTURE: The result of this function is the T matrix in GB_mxm.  It may be
 transplanted directly into the user's matrix, or it may be passed through
 GB_accum_mask.  See GB_mxm.  For the latter case, it would be better to
 delay the contatenation of the output matrix T = [C0 ... C(t-t)].
 GB_accum_mask could do the accum/mask using the sliced T matrix, to
 update the user's C matrix (which is not sliced), and then T is freed.
*/


/** \file GB_AxB_select.c
\brief  GB_AxB_select: select method for C\<M\>=A*B or C=A*B

\par
 Select a saxpy method for each thread: gather/scatter or heap
\par
 parallel: this function can be called in parallel for each thread.
 parallelism will be done in GB_AxB_parallel.
*/


/** \file GB_AxB_sequential.c
\brief  GB_AxB_sequential: C\<M\>=A*B, C\<M\>=A'*B, C=A*B, or C=A'*B

\par
 sequential matrix-matrix multiply.  See GB_AxB_parallel for a description
 of the arguments.
\par
 Does not log an error; returns GrB_SUCCESS, GrB_OUT_OF_MEMORY, or GrB_PANIC.
\par
 parallel: this function will remain sequential.
 parallelism will be done in GB_AxB_parallel.
*/


/** \file GB_BinaryOp_check.c
\brief  GB_BinaryOp_check: check and print a binary operator

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_BinaryOp_compatible.c
\brief  GB_BinaryOp_compatible: check binary operator for type compatibility

\par
 check type compatibilty for C = op (A,B).  With typecasting: A is cast to
 op-\>xtype, B is cast to op-\>ytype, the operator is computed, and then the
 result of op-\>ztype is cast to C-\>type.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_BinaryOp_new.c
\brief  GB_BinaryOp_new: create a new binary operator

\par
 Create a new a binary operator: z = f (x,y).  The binary function signature
 must be void f (void *z, const void *x, const void *y), and then it must
 recast its input and output arguments internally as needed.
\par
 This function is not directly user-callable.  Use GrB_BinaryOp_new instead.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Descriptor_check.c
\brief  GB_Descriptor_check: check and print a Descriptor

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Descriptor_get.c
\brief  GB_Descriptor_get: get the status of a descriptor

\par
 A descriptor modifies the behavoir of a GraphBLAS operation.
\par
 This function is called via the GB_GET_DESCRIPTOR(...) macro.
\par
  Descriptor field:           Descriptor value:
\par
  desc-\>out                   GxB_DEFAULT or GrB_REPLACE
\par
      GrB_REPLACE means that the output matrix C is cleared just
      prior to writing results back into it, via C\<M\> = results.  This
      descriptor does not affect how C is used to compute the results.  If
      GxB_DEFAULT, then C is not cleared before doing C\<M\>=results.
\par
  desc-\>mask                  GxB_DEFAULT or GrB_SCMP
\par
      An optional 'write mask' defines how the results are to be written back
      into C.  The boolean mask matrix M has the same size as C (M is
      typecasted to boolean if it has another type).  If the M input to
      the GraphBLAS method is NULL, then implicitly M(i,j)=1 for all i and
      j.  Let Z be the results to be written into C (the same dimension as
      C).  If desc-\>mask is GxB_DEFAULT, and M(i,j)=1, then C(i,j) is
      over-written with Z(i,j).  Otherwise, if M(i,j)=0 C(i,j) is left
      unmodified (it remains an implicit zero if it is so, or its value is
      unchanged if it has one).  If desc-\>mask is GrB_SCMP, then the use of
      M is negated: M(i,j)=0 means that C(i,j) is overwritten with
      Z(i,j), and M(i,j)=1 means that C(i,j) is left unchanged.
\par
      Writing results Z into C via the mask M is written as C\<M\>=Z in
      GraphBLAS notation.
\par
      Note that it is the value of M(i,j) that determines how C(i,j) is
      overwritten.  If the (i,j) entry is present in the M matrix data
      structure but has a numerical value of zero, then it is the same as if
      (i,j) is not present and thus implicitly zero.  Both mean 'M(i,j)=0'
      in the description above of how the mask M works.
\par
  desc-\>in0 and desc-\>in1     GxB_DEFAULT or GrB_TRAN
\par
      A GrB_Matrix passed as an input parameter to GraphBLAS methods can
      optionally transpose them prior to using them.  desc-\>in0 always refers
      to the first input to the method, and desc-\>in1 always refers to the
      second one.
\par
      If the value of this descriptor is GxB_DEFAULT, then the matrix is used
      as-is.  Otherwise, it is transposed first.  That is, the results are
      the same as if the transpose of the matrix was passed to the method.
\par
  desc-\>axb                   see GraphBLAS.h; can be:
\par
      GxB_DEFAULT = 0         automatic selection
\par
      GxB_AxB_GUSTAVSON       gather-scatter saxpy method
\par
      GxB_AxB_HEAP            heap-based saxpy method
\par
      GxB_AxB_DOT             dot product
\par
  desc-\>nthreads              number of threads to use (auto select if \<= 0)
\par
      This is copied from the GrB_Descriptor into the Context.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Global.c
\brief  GB_Global: global values in GraphBLAS

*/


/** \file GB_I_inverse.c
\brief  GB_I_inverse: invert an index list

\par
 I is a large list relative to the vector length, avlen, and it is not
 contiguous.  Scatter I into the I inverse buckets (Mark and Inext) for quick
 lookup.
\par
 PARALLEL: constructing the I inverse buckets in parallel would require
 synchronization (a critical section for each bucket).  A more parallel
 approach would use qsort first, to find duplicates in I, and then construct
 the buckets in parallel after the qsort.
*/


/** \file GB_Index_multiply.c
\brief  GB_Index_multiply:  multiply two integers and guard against overflow

\par
 c = a*b where c is GrB_Index (uint64_t), and a and b are int64_t.
 Check for overflow.  Requires a \>= 0 and b \>= 0.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Mask_compatible.c
\brief  GB_Mask_compatible: check input and operators for type compatibility

\par
 check the type and dimenions of the mask
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Matrix_check.c
\brief  GB_Matrix_check: print a GraphBLAS matrix and check if it is valid

\par
 parallel: this function does O(nnz(A)) work in GB_matvec_check, not here.
*/


/** \file GB_Monoid_check.c
\brief  GB_Monoid_check: check and print a monoid

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Monoid_new.c
\brief  GB_Monoid_new: create a GrB_Monoid

\par
 Create a user-defined monoid with an operator, identity value, and
 (optionally) a terminal value.  If using a built-in operator, a duplicate
 boolean operators is first replaced with its unique equivalent.  If the
 operator is built-in and corresponds to a known monoid, then the identity
 value and terminal value provided on input are ignored, and the known values
 are used instead.  This is to allow the use of the hard-coded functions for
 built-in monoids.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Sauna_acquire.c
\brief  GB_Sauna_acquire: acquire a set of Saunas

\par
 If the user only calls GraphBLAS from a single user thread, then all
 internal threads will always find their native Sauna.
\par
 FUTURE: time spent in the critical section is typically O(nthreads) or
 O(GB_Global.nthreads_max), unless the user calls GraphBLAS simultaneously
 from multiple user threads.  In that case, the time could be
 O(GxB_NTHREADS_MAX), which is 2048 by default.  If it becomes a performance
 bottleneck, doubly-linked list of available Sauna ids could be kept,
*/


/** \file GB_Sauna_alloc.c
\brief  GB_Sauna_alloc: create a new Sauna

\par
 Does not use error reporting; returns GrB_SUCCESS or GrB_OUT_OF_MEMORY.
*/


/** \file GB_Sauna_free.c
\brief  GB_Sauna_free: free a Sauna

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Sauna_release.c
\brief  GB_Sauna_release: release a set of Saunas

*/


/** \file GB_SelectOp_check.c
\brief  GB_SelectOp_check: check and print a select operator

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_SelectOp_new.c
\brief  GB_SelectOp_new: create a new select operator

\par
 The select function signature must be:
\par
      bool f (GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols,
              const void *x, const void *k) ;
\par
 This function is not directly user-callable.  Use GxB_SelectOp_new instead.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Semiring_check.c
\brief  GB_Semiring_check: check and print a semiring

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Type_check.c
\brief  GB_Type_check: print a built-in type

\par
 There are two character strings: name passed in from the caller, and
 type-\>name.  The caller can use the name argument to print \"the type of
 matrix A:\", for example.  The internal name is the C typedef with which the
 GraphBLAS GrB_Type was created.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Type_compatible.c
\brief  GB_Type_compatible: return true if domains are compatible

\par
 Two domains are compatible for typecasting between them if both are built-in
 types (of any kind) or if both are the same user-defined type.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Type_new.c
\brief  GB_Type_new: create a new user-defined type

\par
 This is not used for built-in types.  Those are created statically.
 Users should not call this function directly; use GrB_Type_new instead.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_UnaryOp_check.c
\brief  GB_UnaryOp_check: check and print a unary operator

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_UnaryOp_new.c
\brief  GB_UnaryOp_new: create a new unary operator

\par
 a unary operator: z = f (x).  The unary function signature must be
 void f (void *z, const void *x), and then it must recast its input and
 output arguments internally as needed.
\par
 This function is not directly user-callable.  Use GrB_UnaryOp_new instead.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_Vector_check.c
\brief  GB_Vector_check: print a GraphBLAS GrB_Vector and check if it is valid

\par
 GrB_Vector: same as GrB_Matrix, except it has exactly one column
\par
 parallel: this function does O(nnz(A)) work in GB_matvec_check, not here.
*/


/** \file GB_accum_mask.c
\brief  GB_accum_mask: accumulate results via the mask and accum operator

\par
 parallel: not here, but in GB_mask, GB_add, GB_transpose
\par
 C\<M\> = accum (C,T)
\par
 The primary computation of a GraphBLAS operations is done, and the results
 are in the T matrix.  The T matrix is then used to modify C, via the accum
 operator and the mask matrix M.
\par
 The results are first accumulated into Z via the accum operator.
\par
 Let Z = accum (C,T) if accum is present, or Z = T otherwise.
 In either case, the type of Z is the same as the C-\>type defined on input.
 If accum is present, T is typecast into the type of the y input to accum.
 If accum is not present, T is typecast into the same type as C.
\par
 If the function z = accum(x,y) is present, then it defines how the existing
 values of C are used to accumulate T into Z.  If both T(i,j) and C(i,j) are
 present in the pattern, then Z(i,j) = accum (C(i,j), T(i,j)).  Otherwise,
 accum is not used: If C(i,j) is present but not T(i,j), then
 Z(i,j)=C(i,j).  If C(i,j) is not present but T(i,j) is present, then
 Z(i,j)=T(i,j).  The pattern of Z = accum(C,T) is the union of C and T.
\par
 The Z = accum (C,T) phase is mimiced by the GB_spec_accum.m MATLAB script.
\par
 The next step is C\<M\> = Z.
\par
 This denotes how the matrix Z is written into C, under the control of the
 mask (or ~M if Mask_complement is true), and the C_replace flag (which
 indicates that C should be set to zero first.  This is C\<M\>=Z in
 GraphBLAS notation.  See GB_mask.c, or GB_spec_mask.m for a MATLAB script
 that describes this step.
\par
 If M is not present, C = Z is returned. Otherwise, M defines what
 values of C are modified. If M(i,j) is present and nonzero, then
 C(i,j)=Z(i,j) is done.  Otherwise, C(i,j) is left unchanged.
\par
 The descriptor affects how C and M are handled.  If the descriptor is
 NULL, defaults are used.
\par
 desc [GB_MASK] = GxB_DEFAULT means to use M as-is
\par
 desc [GB_MASK] = GrB_SCMP means to use the logical negation of M
\par
 desc [GB_OUTP] = GxB_DEFAULT means to use C as-is.
\par
 desc [GB_OUTP] = GrB_REPLACE means to clear C before writing Z into C.
*/


/** \file GB_add.c
\brief  GB_add: 'add' two matrices using an operator

\par
 GB_add (C, A, B, op), 'adds' C = op (A,B), using the given operator
 element-wise on the matrices A and B.  The result is typecasted as needed.
 The pattern of C is the union of the pattern of A and B.
\par
 Let the op be z=f(x,y) where x, y, and z have type xtype, ytype, and ztype.
 If both A(i,j) and B(i,j) are present, then:
\par
      C(i,j) = (ctype) op ((xtype) A(i,j), (ytype) B(i,j))
\par
 If just A(i,j) is present but not B(i,j), then:
\par
      C(i,j) = (ctype) A (i,j)
\par
 If just B(i,j) is present but not A(i,j), then:
\par
      C(i,j) = (ctype) B (i,j)
\par
 ctype is the type of matrix C.  The pattern of C is the union of A and B.
\par
 This function should not be called by the end user.  It is a helper function
 for user-callable routines.  No error checking is performed except for
 out-of-memory conditions.
\par
 This function does not transpose or reformat its inputs or outputs.  C, A,
 and B must have the same number of vectors and vector lengths.  However,
 suppose A is CSR, and B and C are CSC, but the caller wants to compute C =
 A'+B.  Then no transpose of A is needed; just interpret the CSR of A' as a
 CSC format.  The work is the same with C=A'+B if B and C are CSR and A is
 CSC.  Then the output C is CSR, and the CSC of A' is already effectively in
 CSR format.
\par
 As a result, the input formats of A and B are not relevant, and neither is
 the output format of C.  This function can be completely agnostic as to the
 CSR / CSC formats of A, B, and C.  The format of C is determined by the
 caller and assigned to C-\>is_csc, but is otherwise unused here.
\par
 The output C is hypersparse if both A and B are hypersparse; otherwise
 C is not hypersparse.
\par
 FUTURE: this could be faster with built-in operators and types.
\par
 PARALLEL: use 1D parallelism here.  Either do the work in symbolic/numeric
 phases (one to compute nnz in each column, one to fill the output), or
 compute submatrices and then concatenate them.  Probably do the former.
 See also GB_emult.
*/


/** \file GB_aliased.c
\brief  GB_aliased: determine if two matrices are aliased

\par
 Returns true if A == B (and not NULL), or if any component A and B are
 aliased to each other.  In the latter case, that component of A and B will
 always be shallow, in either A or B, or both.  NULL pointers are not
 aliased.
\par
 not parallel: takes O(1) time
*/


/** \file GB_apply.c
\brief  GB_apply: apply a unary operator; optionally transpose a matrix

\par
 C\<M\> = accum (C, op(A)) or accum (C, op(A)')
\par
 GB_apply does the work for GrB_*_apply.  Compare this with GrB_transpose.
\par
 parallel: not here, but in GB_transpose and GB_shallow_op
*/


/** \file GB_apply_op.c
\brief  GB_apply_op:  apply a unary operator to an array

\par
 Cx = op ((xtype) Ax)
\par
 Compare with GB_transpose_op.c
\par
 PARALLEL: do it here, but it is easy.  Might want to split into separate
 files like Generated/GB_AxB*, so worker is not in a macro but in a function.
*/


/** \file GB_assign.c
\brief  GB_assign: submatrix assignment: C\<M\>(Rows,Cols) = accum (C(Rows,Cols),A)

\par
 submatrix assignment: C\<M\>(Rows,Cols) = accum (C(Rows,Cols),A)
\par
 All GrB_*_assign operations rely on this function.
\par
 Only one of the bool parameters: scalar_expansion, col_assign, and
 row_assign can be true.  If all are false, this function does the work for
 GrB_Matrix_assign and GrB_Vector_assign.
\par
 If scalar_expansion is true, this function performs scalar assignment (the
 GrB_Matrix_assign_TYPE and GrB_Vector_assign_TYPE functions) in which case
 the input matrix A is ignored (it is NULL), and the scalar is used instead.
\par
 If col_assign is true, this function does the work for GrB_Col_assign.
 If row_assign is true, this function does the work for GrB_Row_assign.
\par
 Compare with GB_subassign, which uses M and C_replace differently
\par
 PARALLEL: some C_replace_phase here, mainly in GB_subassign_kernel and
 GB_subref_numeric.
*/


/** \file GB_assign_scalar.c
\brief  GB_assign_scalar:    C\<M\>(Rows,Cols) = accum (C(Rows,Cols),x)

\par
 Assigns a single scalar to a submatrix:
\par
 C\<M\>(Rows,Cols) = accum (C(Rows,Cols),x)
\par
 This function does the work for GrB_Matrix_assign_TYPE and
 GrB_Vector_assign_[type], where [type] is one of the 11 types, or the
 type-generic macro suffix, \"_UDT\".
\par
 Compare with GB_subassign_scalar, which uses M and C_replace differently
\par
 parallel: not here; see GB_assign
*/


/** \file GB_block.c
\brief  GB_block: apply all pending computations if blocking mode enabled

\par
 parallel: not here.  See GB_wait instead.
*/


/** \file GB_boolean_rename.c
\brief  GB_boolean_rename: rename a boolean opcode

\par
 Returns the equivalent opcode when an operator's x and y arguments are
 boolean.  13 of the 23 binary opcodes are redundant when applied to
 boolean inputs, leaving 10 unique binary opcodes z=f(x,y) when all three
 operands x,y,z are boolean.
\par
 Another 3 boolean operators are not considered here since they share
 the same opcode:
\par
 GrB_LOR  == GxB_LOR_BOOL,
 GrB_LAND == GxB_LAND_BOOL,
 GrB_LXOR == GxB_LXOR_BOOL
\par
 Those 6 names are in GraphBLAS but the pairs of names are equivalent.
\par
 See discussion on Source/GB.h on boolean and integer division
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_build.c
\brief  GB_build: build a matrix

\par
 Construct a matrix C from a list of indices and values.  Any duplicate
 entries with identical indices are assembled using the binary dup operator
 provided on input.  All three types (x,y,z for z=dup(x,y)) must be
 identical.  The types of dup, S, and C must all be compatible.
\par
 Duplicates are assembled using T(i,j) = dup (T (i,j), S (k)) into a
 temporary matrix T that has the same type as the dup operator.  The
 GraphBLAS spec requires dup to be associative so that entries can be
 assembled in any order.  There is no way to check this condition if dup is a
 user-defined operator.  It could be checked for built-in operators, but the
 GraphBLAS spec does not require this condition to cause an error so that is
 not done here.  If dup is not associative, the GraphBLAS spec states that
 the results are not defined.
\par
 SuiteSparse:GraphBLAS provides a well-defined order of assembly, however.
 Entries in [I,J,S] are first sorted in increasing order of row and column
 index via a stable sort, with ties broken by the position of the tuple in
 the [I,J,S] list.  If duplicates appear, they are assembled in the order
 they appear in the [I,J,S] input.  That is, if the same indices i and j
 appear in positions k1, k2, k3, and k4 in [I,J,S], where k1 \< k2 \< k3 \< k4,
 then the following operations will occur in order:
\par
      T (i,j) = S (k1) ;
\par
      T (i,j) = dup (T (i,j), S (k2)) ;
\par
      T (i,j) = dup (T (i,j), S (k3)) ;
\par
      T (i,j) = dup (T (i,j), S (k4)) ;
\par
 This is a well-defined order but the user should not depend upon it since
 the GraphBLAS spec does not require this ordering.  Results may differ in
 different implementations of GraphBLAS.
\par
 However, with this well-defined order, the SECOND operator will result in
 the last tuple overwriting the earlier ones.  This is relied upon internally
 by GB_wait.
\par
 After the matrix T is assembled, it is typecasted into the type of C, the
 final output matrix.  No typecasting is done during assembly of duplicates,
 since mixing the two can break associativity and lead to unpredictable
 results.  Note that this is not the case for GB_wait, which must typecast
 each tuple into its output matrix in the same order they are seen in
 the [I,J,S] pending tuples.
\par
 On input, C must not be NULL.  C-\>type, C-\>vlen, C-\>vdim and C-\>is_csc must
 be valid on input and are unchanged on output.  C must not have any existing
 entries on input (GrB_*_nvals (C) must return zero, per the specification).
 However, all existing content in C is freed.
\par
 The list of numerical values is given by the void * S array and a type code,
 scode.  The latter is defined by the actual C type of the S parameter in
 the user-callable functions.  However, for user-defined types, there is no
 way of knowing that the S array has the same type as dup or C, since in that
 case S is just a void * pointer.  Behavior is undefined if the user breaks
 this condition.
\par
 C is returned as hypersparse or non-hypersparse, depending on the number of
 non-empty vectors of C.  If C has very few non-empty vectors, then it is
 returned as hypersparse.  Only if the number of non-empty vectors is
 Omega(n) is C returned as non-hypersparse, which implies nvals is Omega(n),
 where n = \# of columns of C if CSC, or \# of rows if CSR.  As a result, the
 time taken by this function is just O(nvals*log(nvals)), regardless of what
 format C is returned in.
\par
 If nvals == 0, I_in, J_in, and S may be NULL.
\par
 PARALLEL: this function passes over the tuples to see if they are sorted,
 which could be done as a parallel reduction.  The copy into jwork is fully
 parallel.  The check for indices out-of-bounds is also like a parallel
 reduction, but it could stop as soon as a single thread finds one index
 out of bounds.  This function then calls GB_builder, which does the sort
 of the tuples and constructs the hypersparse CSR/CSC matrix C.
*/


/** \file GB_build_factory.c
\brief  GB_build_factory: build a matrix from sorted tuples

\par
 The tuples have been sorted and duplicates marked by GB_builder.  Assemble
 them with a switch factory of built-in workers, or two generic workers.  The
 vector pointers T-\>p and hyperlist T-\>h (if hypersparse) have already been
 computed.  This function is agnostic regarding the CSR/CSR format, and even
 hypersparsity.  The rest of the build process here needs to know nothing at
 all about the vectors.  kwork is NULL on input if the original input tuples
 did not need sorting; in this case, kwork [k] == k is implicitly true.
\par
 iwork holds the row indices of the tuple, and kwork holds the positions in
 the array S.  The tuples are sorted so that duplicates are adjacent to each
 other and they appear in the order they appeared in the original tuples.
 This method assembles the duplicates and creates T-\>x from iwork, kwork, and
 S.  iwork is then transplanted into T, becoming T-\>i.
\par
 On input, the (i,k,S[k]) tuples are held in two integer arrays, iwork and
 kwork, and an array S of numerical values.  S has not been sorted, nor even
 accessed yet.  It is identical to the original unsorted tuples.  The
 (i,k,S[k]) tuple holds the row index i, the position k, and the value S [k].
 This entry becomes T(i,j) = S [k] in the matrix T, and duplicates are
 assembled via the dup operator.
\par
 The row indices on input are in iwork, and after duplicates are removed,
 iwork is compacted (duplicates removed) and then transplanted directly in
 the T, becoming the row indices T-\>i.  The symbolic analysis is thus
 consumed by this function, and incorporated into the output matrix T, in
 place.  If this method is split into user-callable symbolic analysis and
 numerical phases, then a copy of iwork should be made, which would then be
 consumed and transplanted into T-\>i.  Also, kwork (which is read-only by
 thus function) should not be freed.  If these changes were made, then iwork
 and kwork could be used for subsequent builds of T with the same pattern and
 ordering of tuples but with different numerical values.
\par
 On output, kwork is freed and iwork becomes T-\>i.   Thus iwork_handle and
 kwork_handle in the caller are both set to NULL.
\par
 The time and memory taken by this function is O(t) if t=len is the number
 of tuples.
\par
 PARALLEL: the tuples have already been sorted, and duplicates tagged.  need
 to parallelize the summation of duplicate tuples.  Each unique tuple could
 be done only by the thread the owns it.  It is unlikely that there will be
 many duplicates, but possible.  So consider a parallel reduction.
*/


/** \file GB_builder.c
\brief  GB_builder: build a matrix from tuples

\par
 The input arguments \&iwork and \&jwork are always freed by this function.
 This function is agnostic regarding the CSR/CSC format.  It decides whether
 T is standard sparse or hypersparse, as determined by the default rules
 for GrB_Matrix_new.
\par
 This function is called by GB_build to build a matrix T for GrB_Matrix_build
 or GrB_Vector_build, and by GB_wait to build a matrix T from the list of
 pending tuples.
\par
 PARALLEL: first does qsort, so need to parallelize GB_qsort_*.  Then passes
 over the tuples to find duplicates, which has some dependencies but could be
 done in bulk parallel.  After sorting, a thread owns a chunk of tuples.  It
 can mark all its own duplicates, fully in parallel, but not across to tuples
 owned by another thread.  When done with this first phase, a 2nd pass could
 find any duplicates across the thread boundaries.
*/


/** \file GB_calloc_memory.c
\brief  GB_calloc_memory: wrapper for calloc (used via the GB_CALLOC_MEMORY macro)

\par
 A wrapper for calloc.  Space is set to zero.
\par
 This function is called via the GB_CALLOC_MEMORY(p,n,s,Context) macro.
\par
 Parameters are the same as the POSIX calloc, except that asking to allocate
 a block of zero size causes a block of size 1 to be allocated instead.  This
 allows the return pointer p to be checked for the out-of-memory condition,
 even when allocating an object of size zero.
\par
 PARALLEL: clear the array in parallel?
*/


/** \file GB_cast_array.c
\brief  GB_cast_array: typecast an array

\par
 Casts an input array A to an output array C with a different built-in type.
 Does not handle user-defined types.
\par
 PARALLEL: easy.  May want to put the workers in functions, like
 Generated/GB_AxB*, instead of in a macro.
*/


/** \file GB_cast_factory.c
\brief  GB_cast_factory: return a pointer to a typecasting function

\par
 Returns a pointer to a function f(z,x,s) that copies its input x into its
 output z, casting as needed.  That is, it computes z = (type of z) x.
 s is the size for user-defined types, which can only be copied.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_clear.c
\brief  GB_clear: clears the content of a matrix

\par
 All content of A is freed (or removed if shallow) and new A-\>p and A-\>h
 content is created.  This puts the matrix A in the same initialized state it
 had after GrB_Matrix_new (\&A, ...), with A-\>magic == GB_MAGIC to denote a
 valid, initialized matrix, with nnz(A) equal to zero.  The dimensions, type,
 and CSR/CSC format are unchanged.  The hypersparsity of the newly empty
 matrix A is determined by the A-\>hyper_ratio for the matrix.  The matrix is
 valid.
\par
 However, if this method runs out of memory, and the A-\>p and A-\>h structure
 cannot be recreated, then all content of the matrix is freed or removed, and
 the matrix A is left in an invalid state (A-\>magic == GB_MAGIC2).  Only the
 header is left.
\par
 parallel: not here, but perhaps in GB_calloc_memory.
*/


/** \file GB_code_check.c
\brief  GB_code_check: print an entry using a type code

\par
 Only prints entries of built-in types; user-defined types can't be printed.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_code_compatible.c
\brief  GB_code_compatible: return true if domains are compatible

\par
 Two domains are compatible for typecasting between them if both are built-in
 types (of any kind) or if both are the same user-defined type.  This
 function does not have the type itself, but just the code.  If the types are
 available, GB_Type_compatible should be called instead.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_code_size.c
\brief  GB_code_size: given a type code, return sizeof (type)

\par
 The user-defined type has no known size, so this must be provided on input.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_code_string.c
\brief  GB_code_string: convert a type code into a string

\par
 Given GB_Type_code, return a string with the name of the type
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_code_type.c
\brief  GB_code_type: convert a type code to a GrB_Type

\par
 The GrB_assign, GxB_subassign, and GrB_setElement operations all accept
 scalar inputs.  The scalar code is converted to an appropriate GrB_Type
 here.  For user-defined types, the scalar is required to have the same type
 as the matrix being operated on.  This cannot be checked; results are
 undefined if the user passes in a void * pointer to a different user-defined
 type.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_compatible.c
\brief  GB_compatible: check input and operators for type compatibility

\par
 Check if the types for C\<M\> = accum (C,T) are all compatible,
 and (if present) make sure the size of C and M match.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_create.c
\brief  GB_create: create a matrix and allocate space

\par
 Creates a matrix (with GB_new), then allocates a given space for indices and
 values.
\par
 This function is called via the GB_CREATE(...) macro.
\par
 Ahandle must be non-NULL on input.
\par
 If *Ahandle is NULL on input:
\par
      A new header for the matrix A is allocated.  If successful, *Ahandle
      points to the new handle, and its contents, on output.  If an
      out-of-memory condition occurs, the header is freed and *Ahandle is
      NULL on output.
\par
 If *Ahandle is not NULL on input:
\par
      The existing header for A is used.  The pointer *Ahandle itself is not
      modified on output, either on success or failure.  If successful, the
      content of A has been created.  If an out-of-memory condition occurs,
      the preexisting header is not freed and *Ahandle is unmodified on
      output.
\par
 To see where these options are used in SuiteSparse:GraphBLAS:
 grep \"allocate a new header\"
 which shows all uses of GB_new and GB_create
\par
 parallel: not here but perhaps in GB_new
*/


/** \file GB_cumsum.c
\brief  GB_cumsum: cumlative sum of an array

\par
 Compute the cumulative sum of an array count[0:n], of size n+1
 in pseudo-MATLAB notation:
\par
      k = sum (count [0:n] != 0) ;
\par
      count = cumsum ([0 count[0:n-1]]) ;
\par
 That is, count [j] on input is overwritten with the value of
 sum (count [0..j-1]).
\par
 GB_transpose_bucket uses this function to compute row and column pointers.
 On input, count [j] is the number of nonzeros in column j of a matrix, and
 count [n] should be zero (it only affects the result k, however).  On
 output, count [0..n] contains the column pointers of the matrix, and kresult
 is the number of nonzeros in count [0:n], which becomes nvec_nonempty for
 the resulting matrix.
\par
 GB_AxB_flopcount uses this to compute the cumulative sum of the flop
 counts for C\<M\>=A*B, for each column of B.  In this case, n is the
 number of vectors in B.  It does not need the value k, so it is not
 computed.
\par
 PARALLEL: a parallel cumsum
*/


/** \file GB_dup.c
\brief  GB_dup: make a deep copy of a sparse matrix

\par
 C = A, making a deep copy.  Not user-callable; this function does the work
 for user-callable functions GrB_*_dup.
\par
 There is little use for the following feature, but (*Chandle) and A might be
 identical, with GrB_dup (\&A, A).  The input matrix A will be lost, and will
 result in a memory leak, unless the user application does the following
 (which is valid and memory-leak free):
\par
  B = A ;
\par
  GrB_dup (\&A, A) ;
\par
  GrB_free (\&A) ;
\par
  GrB_free (\&B) ;
\par
 A is the new copy and B is the old copy.  Each should be freed when done.
\par
 PARALLEL: a few large memcpy's, can be done in parallel.
*/


/** \file GB_eWise.c
\brief  GB_eWise: C\<M\> = accum (C, A+B) or A.*B

\par
 C\<M\> = accum (C,A+B), A.*B and variations.
\par
 The input matrices A and B are optionally transposed.
\par
 Not user-callable.  Does the work for all user-callable functions of
 the form GrB_eWiseAdd_* and GrB_eWiseMult_*
\par
 parallel: not here, but in GB_add, GB_emult, GB_transpose, ...
*/


/** \file GB_emult.c
\brief  GB_emult: element-wise \"multiplication\" of two matrices

\par
 GB_emult (C, A, B, op), applies an operator C = op (A,B)
 element-wise on the matrices A and B.  The result is typecasted as needed.
\par
 Let the op be z=f(x,y) where x, y, and z have type xtype, ytype, and ztype.
 If both A(i,j) and B(i,j) are present, then:
\par
      C(i,j) = (ctype) op ((xtype) A(i,j), (ytype) B(i,j))
\par
 If just A(i,j) is present but not B(i,j), then:
\par
      C(i,j) is not present, and is implicitly 'zero'
\par
 If just B(i,j) is present but not A(i,j), then:
\par
      C(i,j) is not present, and is implicitly 'zero'
\par
 ctype is the type of matrix C.  Its pattern is the intersection of A and B.
\par
 This function should not be called by the end user.  It is a helper function
 for user-callable routines.  No error checking is performed except for
 out-of-memory conditions.
\par
 FUTURE: this could be faster with built-in operators and types.
\par
 PARALLEL: use 1D parallelism.  Either do the work in symbolic/numeric phases
 (one to compute nnz in each column, one to fill the output), or compute
 submatrices and then concatenate them.  See also GB_add.
*/


/** \file GB_entry_check.c
\brief  GB_entry_check: print a single entry for a built-in type

\par
 not parallel: This function does O(1) work, but can be called for every
 entry in a matrix, to print it out in GxB_Matrix_fprint or
 GxB_Vector_fprint.  The printing to a file is fundamentally sequential, and
 also only needed for diagnostics.  For Matrix Market I/O, however, printing
 of the entire matrix could perhaps be done in parallel.
*/


/** \file GB_error.c
\brief  GB_error: log an error string

\par
 GB_error logs the details of an error to the error string in thread-local
 storage so that it is accessible to GrB_error.  A GrB_PANIC is not logged
 to the error string since the panic may mean the string is not available.
\par
 This function is called via the GB_ERROR(info,args) macro.
\par
 SuiteSparse:GraphBLAS can generate a GrB_PANIC in the following ways:
\par
  (1) a failure to create the critical section or the POSIX thread-local
      storage key in GrB_init.
\par
  (2) a failure in the critical section (see GB_CRITICAL, GB_queue_*, and
      Template/GB_critical_section).
\par
  (3) a failure to allocate thread-local storage for GrB_error
      (see GB_thread_local_access).
\par
  (4) a failure to destroy the critical section in GrB_finalize.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_extract.c
\brief  GB_extract: C\<M\> = accum(C,A(I,J))

\par
 Not user-callable.  Implements the user-callable GrB_*_extract functions.
\par
 C\<M\> = accum (C, A (Rows,Cols)) or
\par
 C\<M\> = accum (C, AT(Rows,Cols)) where AT = A'
\par
 equivalently:
\par
 C\<M\> = accum (C, A(Rows,Cols) )
\par
 C\<M\> = accum (C, A(Cols,Rows)')
\par
 parallel: not here, but in GB_subref_numeric.
*/


/** \file GB_extractElement.c
\brief  GB_extractElement: x = A(row,col)

\par
 Extract the value of single scalar, x = A(row,col), typecasting from the
 type of A to the type of x, as needed.  Not user-callable; does the work for
 all GrB_*_extractElement* functions.
\par
 Returns GrB_SUCCESS if A(row,col) is present, and sets x to its value.
 Returns GrB_NO_VALUE if A(row,col) is not present, and x is unmodified.
\par
 not parallel: this function does O(log(..)) work and is already thread-safe.
*/


/** \file GB_extractTuples.c
\brief  GB_extractTuples: extract all the tuples from a matrix

\par
 Extracts all tuples from a matrix, like [I,J,X] = find (A).  If any
 parameter I, J and/or X is NULL, then that component is not extracted.  The
 size of the I, J, and X arrays (those that are not NULL) is given by nvals,
 which must be at least as large as GrB_nvals (\&nvals, A).  The values in the
 matrix are typecasted to the type of X, as needed.
\par
 If all arrays I, J, X are NULL, this function does nothing except to force
 all pending tuples to be assembled.  This is an intended side effect.
\par
 This function is not user-callable.  It does the work for the user-callable
 GrB_*_extractTuples functions.
\par
 PARALLEL: easy.  Does large memcpy's and fully parallel loops.
*/


/** \file GB_free.c
\brief  GB_free: free a matrix

\par
 Free all the content of a matrix.  After GB_free (\&A), A is set to NULL.
\par
 This function is called via the GB_MATRIX_FREE(A) and GB_VECTOR_FREE(v)
 macros.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_free_memory.c
\brief  GB_free_memory: wrapper for free (used via the GB_FREE_MEMORY macro)

\par
 A wrapper for free.  If p is NULL on input, it is not freed.
\par
 This function is called via the GB_FREE_MEMORY(p,n,s) macro.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_hyper_realloc.c
\brief  GB_hyper_realloc: reallocate a matrix hyperlist

\par
 Change the size of the A-\>h and A-\>p hyperlist.
 No change is made if A is not hypersparse.
\par
 parallel: not here, but perhaps in GB_realloc_memory
*/


/** \file GB_ijlength.c
\brief  GB_ijlength: get the length and kind of an index list I

\par
 Determine the length of I, and process the colon notation I = begin:inc:end.
 No error checking is done.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_ijproperties.c
\brief  GB_ijproperties: check I and determine its properties

\par
 check a list of indices I and determine its properties
\par
 PARALLEL: checks the entire set of indices in the array I, if it is a list,
 to see if any entry is out of bounds, and to determine if it is sorted
 order.  Similar to the the check of I and J in GB_build.
*/


/** \file GB_ijsort.c
\brief  GB_ijsort:  sort an index array I and remove duplicates

\par
 PARALLEL: deletes duplicates, see also GB_builder.  This is only used in
 GB_assign, for scalar expansion and for the C_replace_phase, and only when I
 and/or J are lists (not GrB_ALL, nor lo:inc:hi).
*/


/** \file GB_init.c
\brief  GB_init: initialize GraphBLAS

\par
 GrB_init (or GxB_init) must called before any other GraphBLAS operation;
 both rely on this internal function.  If GraphBLAS is used by multiple user
 threads, only one can call GrB_init or GxB_init.
\par
 Result are undefined in multiple user threads simultaneously
 call GrB_init (or GxB_init).
\par
 GrB_finalize must be called as the last GraphBLAS operation.
\par
 GrB_init or GxB_init define the mode that GraphBLAS will use:  blocking or
 non-blocking.  With blocking mode, all operations finish before returning to
 the user application.  With non-blocking mode, operations can be left
 pending, and are computed only when needed.
\par
 GxB_init is the same as GrB_init except that it also defines the
 malloc/calloc/realloc/free functions to use.
\par
 not parallel: this function does O(1) work.
*/


/** \file GB_ix_alloc.c
\brief  GB_ix_alloc: allocate a matrix to hold a given number of entries

\par
 Does not modify A-\>p or A-\>h (unless an error occurs).  Frees A-\>x and A-\>i
 and reallocates them to the requested size.  Frees any pending tuples and
 deletes all entries (including zombies, if any).  If numeric is false, then
 A-\>x is freed but not reallocated.
\par
 If this method fails, all content of A is freed (including A-\>p and A-\>h).
\par
 GB_ix_alloc is only called by GB_create, so the matrix is not in the queue.
 The function never accessed the global matrix queue, and thus it
 cannot return GrB_PANIC.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_ix_free.c
\brief  GB_ix_free: free A-\>i, A-\>x, pending tuples, zombies; A-\>p, A-\>h unchanged

\par
 Since A-\>p and A-\>h are unchanged, the matrix is still valid (unless it was
 invalid on input).  nnz(A) would report zero, and so would GrB_Matrix_nvals.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_ix_realloc.c
\brief  GB_ix_realloc: reallocate a matrix to hold a given number of entries

\par
 Does not modify A-\>p.  Reallocates A-\>x and A-\>i to the requested size,
 preserving the existing content of A-\>x and A-\>i.  Preserves pending tuples
 and zombies, if any.  If numeric is false, then A-\>x is freed instead.
\par
 parallel: not here; see GB_realloc_memory instead, which might be done in
 parallel to copy from the old to new space.
*/


/** \file GB_ix_resize.c
\brief  GB_ix_resize:  reallocate a matrix with some slack for future growth

\par
 nnz(A) has, or will, change.  The \# of nonzeros may decrease significantly,
 in which case the extra space is trimmed.  If the existing space is not
 sufficient, the matrix is doubled in size to accomodate the new entries.
\par
 parallel: not here, but perhaps in GB_ix_realloc.
*/


/** \file GB_kron.c
\brief  GB_kron: C\<M\> = accum (C, kron(A,B))

\par
 C\<M\> = accum (C, kron(A,B))
\par
 The input matrices A and B are optionally transposed.
\par
 Not user-callable.  Does the work for GxB_kron
\par
 parallel: not here, but in GB_kron_kernel.
*/


/** \file GB_kron_kernel.c
\brief  GB_kron_kernel: Kronecker product, C = kron (A,B)

\par
 C = kron(A,B) where op determines the binary multiplier to use.  The type of
 A and B are compatible with the x and y inputs of z=op(x,y), but can be
 different.  The type of C is the type of z.  C is hypersparse if either A
 or B are hypersparse.
\par
 PARALLEL: simple parallelism, but need to handle combinations of
 hyper/non-hyper cases in doubly-nested loops.
*/


/** \file GB_malloc_memory.c
\brief  GB_malloc_memory: wrapper for malloc_function

\par
 A wrapper for malloc_function.  Space is not initialized.
\par
 This function is called via the GB_MALLOC_MEMORY(p,n,s) macro.
\par
 Asking to allocate a block of zero size causes a block of size 1 to be
 allocated instead.  This allows the return pointer p to be checked for the
 out-of-memory condition, even when allocating an object of size zero.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_mask.c
\brief  GB_mask: apply a mask: C\<M\> = Z

\par
 C\<M\> = Z
\par
 Nearly all GraphBLAS operations take a mask, which controls how the result
 of the computations, Z, are copied into the result matrix C.  The following
 working MATLAB script, GB_spec_mask, defines how this is done.
\par
 This function can only handle the case when C, M, and Z all have the same
 format (all CSC and CSR transposed, or all CSR or CSC transposed).  The
 caller (GB_accum_mask) must transpose as needed, before calling this
 function.  This function can handle any combination of hypersparsity of C,
 M, and/or Z, as needed.  In the comments, C(i,j) is shorthand for the index
 i in the jth vector, and likewise for M, Z, and R.  If the matrices are all
 CSC, then this is row i and column j.  If the matrices are all CSR, then it
 is row j and column i.
\par
 PARALLEL: similar method as GB_add
*/


/** \file GB_matvec_check.c
\brief  GB_matvec_check: print a GraphBLAS matrix and check if it is valid

\par
 parallel: could be parallelized, but this is meant primarily for testing
 and debugging, so performance is not critical.
*/


/** \file GB_mxm.c
\brief  GB_mxm: matrix-matrix multiply for GrB_mxm, GrB_mxv, and GrB_vxm

\par
 C\<M\> = accum (C,A*B) and variations.
\par
 This function is not user-callable.  It does the work for user-callable
 functions GrB_mxm, GrB_mxv, and GrB_vxm.
\par
 parallel: not here; see GB_AxB_parallel instead.
*/


/** \file GB_new.c
\brief  GB_new: create a new GraphBLAS matrix

\par
 Creates a new matrix but does not allocate space for A-\>i and A-\>x.
 See GB_create instead.
\par
 This function is called via the GB_NEW(...) macro.
\par
 If the Ap_option is GB_Ap_calloc, the A-\>p and A-\>h are allocated and
 initialized, and A-\>magic is set to GB_MAGIC to denote a valid matrix.
 Otherwise, the matrix has not yet been fully initialized, and A-\>magic is
 set to GB_MAGIC2 to denote this.  This case only occurs internally in
 GraphBLAS.  The internal function that calls GB_new must then allocate or
 initialize A-\>p itself, and then set A-\>magic = GB_MAGIC when it does so.
\par
 Only GrB_SUCCESS and GrB_OUT_OF_MEMORY can be returned by this function.
\par
 The GrB_Matrix object holds both a sparse vector and a sparse matrix.  A
 vector is represented as an vlen-by-1 matrix, but it is sometimes treated
 differently in various methods.  Vectors are never transposed via a
 descriptor, for example.
\par
 The matrix may be created in an existing header, which case *Ahandle is
 non-NULL on input.  If an out-of-memory condition occurs, (*Ahandle) is
 returned as NULL, and the existing header is freed as well, if non-NULL on
 input.
\par
 To see where these options are used in SuiteSparse:GraphBLAS:
 grep \"allocate a new header\"
 which shows all uses of GB_new and GB_create
\par
 parallel: not here; see GB_calloc_memory
*/


/** \file GB_nvals.c
\brief  GB_nvals: number of entries in a sparse matrix

\par
 parallel: not here; see GB_wait
*/


/** \file GB_nvec_nonempty.c
\brief  GB_nvec_nonempty: count the number of non-empty vectors

\par
 Pending tuples are ignored.  If a vector has all zombies it is still
 counted as non-empty.
\par
 PARALLEL: simple parallel reduction
*/


/** \file GB_op_is_second.c
\brief  GB_op_is_second: return true if op is the SECOND operator of the right type

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_ops.c
\brief  GB_builtin.c: built-in types, functions, operators, and other externs

\par
 This file defines the predefined built-in objects: 11 types, 45 unary
 operators, 256 binary operators, 44 monoids, and 960 semirings.
\par
 not parallel: this function has no executable code, just definitions.
*/


/** \file GB_pending_add.c
\brief  GB_pending_add:  add an entry A(i,j) to the list of pending tuples

\par
 Compare this function with the CSparse function cs_entry, the essence of
 which is copied below.  A CSparse matrix can be held in either compressed
 sparse column format, or as a list of tuples, but never both.  A GraphBLAS
 matrix can have both components.
\par
 The cs_entry function appends a single entry to the end of the tuple list,
 and it doubles the space if no space is available.  It also augments the
 matrix dimension as needed, which GB_pending_add does not do.
\par
 This function starts with an initial list that is larger than cs_entry
 (which starts with a list of size 1), and it quadruples the size as needed
 instead of doubling it.  If A has a single column then the column index is
 not kept.  Finally, this function supports any data type whereas CSparse
 only allows for double.
\par
 Otherwise the two methods are essentially the same.  The reader is
 encouraged the compare/contrast the unique coding styles used in CSparse and
 this implementation of GraphBLAS.  CSparse is concise; the book provides the
 SIAM, Philadelphia, Sept. 2006, http://bookstore.siam.org/fa02 .  Csparse is
 at http://faculty.cse.tamu.edu/davis/publications_files/CSparse.zip .
\par
 If the function succeeds, the matrix is added to the queue if it is not
 already there.
\par
 If the function fails to add the pending tuple, the entire matrix is
 cleared of all entries, all pending tuples, and all zombies; and it is
 removed from the queue if it is already there.
\par
 This function is agnostic about the CSR/CSC format of A.  Regardless of the
 format, i refers to an index into the vectors, and j is a vector.  So for
 CSC, i is a row index and j is a column index.  For CSR, i is a column index
 and j is a row index.  This function also does not need to know if A is
 hypersparse or not.
\par
 parallel: unless reallocation occurs, this function does O(1) work and is
 already thread-safe.  The reallocation could be parallel; see
 GB_realloc_memory.
*/


/** \file GB_pending_free.c
\brief  GB_pending_free: free all pending tuples

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_ph_free.c
\brief  GB_ph_free: free the A-\>p and A-\>h content of a matrix

\par
 Free the A-\>p and A-\>h content of a matrix.  If followed by GB_ix_free(A),
 the header of A is just like GB_new with GB_Ap_null.  No content is left
 except the header.  The matrix becomes invalid, and would generate a
 GrB_INVALID_OBJECT error if passed to a user-callable GraphBLAS function.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_phix_free.c
\brief  GB_phix_free: free all content of a matrix

\par
 Frees all allocatable content of a matrix, except for the header itself.
 A-\>magic becomes GB_MAGIC2.  If this matrix is given to a user-callable
 GraphBLAS function, it will generate a GrB_INVALID_OBJECT error.
\par
 not parallel: this function does O(1) work and is already thread-safe.
\par
 This function normally returns GrB_SUCCESS. 
\par
 If A is in the queue, it is removed, which requires access to the critical
 section for the queue.  Normally this cannot fail, but In the rare event
 that the critical section does fail, a GrB_PANIC is returned.
*/


/** \file GB_qsort_1.c
\brief  GB_qsort_1: sort an n-by-1 list of integers

\par
 This sort is not stable, but it is used in GraphBLAS only on lists with
 unique integers.  So it does not need to be stable.
\par
 parallel: not here, see Template/GB_qsort_template.c
*/


/** \file GB_qsort_2a.c
\brief  GB_qsort_2a: sort a 2-by-n list of integers, using A [0][ ] as the sort key

\par
 This sort is not stable, but it is used in GraphBLAS only on lists with
 unique tuples (i,k).  So it does not need to be stable.  Just the first
 entry i in each tuple (i,k) is used as the sort key.  The second item k in
 each tuple happens to be unique in itself, but this is not part of the
 sort key.
\par
 parallel: not here, see Template/GB_qsort_template.c
*/


/** \file GB_qsort_2b.c
\brief  GB_qsort_2b: sort a 2-by-n list of integers, using A[0:1][ ] as the key

\par
 This sort is not stable, but it is used in GraphBLAS only on lists with
 unique tuples (i,k).  So it does not need to be stable.  Both entries i
 and k in the tuples (i,k) are used as the sort key.  The value i may
 appear in multiple tuples, but the value k is unique across all tuples.
\par
 parallel: not here, see Template/GB_qsort_template.c
*/


/** \file GB_qsort_3.c
\brief  GB_qsort_3: sort a 3-by-n list of integers, using A[0:2][] as the key

\par
 This sort is not stable, but it is used in GraphBLAS only on lists with
 unique tuples (j,i,k).  So it does not need to be stable.  All entries j, i
 and k in the tuples (j,i,k) are used as the sort key.  The values i and j
 may appear in multiple tuples, but the value k is unique across all tuples.
\par
 parallel: not here, see Template/GB_qsort_template.c
*/


/** \file GB_queue_insert.c
\brief  GB_queue_insert:  insert a matrix at the head of the matrix queue

\par
 check if the matrix has pending computations (either pending tuples or
 zombies, or both).  If it has any, and if it is not already in the queue,
 then insert it into the queue.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_queue_remove.c
\brief  GB_queue_remove: remove a matrix from the matrix queue

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_queue_remove_head.c
\brief  GB_queue_remove_head: remove the matrix at the head of the matrix queue

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_queue_status.c
\brief  GB_queue_status:  check the status of the queue for a particular matrix

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_realloc_memory.c
\brief  GB_realloc_memory: wrapper for realloc_function

\par
 A wrapper for realloc_function.
\par
 This function is called via the GB_REALLOC_MEMORY macro.
\par
 If p is non-NULL on input, it points to a previously allocated object of
 size nitems_old * size_of_item.  The object is reallocated to be of size
 nitems_new * size_of_item.  If p is NULL on input, then a new object of that
 size is allocated.  On success, a pointer to the new object is returned, and
 ok is returned as true.  If the allocation fails, ok is set to false and a
 pointer to the old (unmodified) object is returned.
\par
 Usage:
\par
      p = GB_realloc_memory (nnew, nold, size, p, \&ok, Context)
\par
      if (ok)
\par
          p points to a space of size at least nnew*size, and the first
          part, of size min(nnew,nold)*size, has the same content as
          the old memory space if it was present.
\par
      else
\par
          p points to the old space of size nold*size, which is left
          unchanged.  This case never occurs if nnew \< nold.
\par
 PARALLEL: move the data in parallel?
*/


/** \file GB_reduce_to_column.c
\brief  GB_reduce_to_column: reduce a matrix to a column using a binary op

\par
 C\<M\> = accum (C,reduce(A)) where C is n-by-1
\par
 PARALLEL: use a parallel reduction method
*/


/** \file GB_reduce_to_scalar.c
\brief  GB_reduce_to_scalar: reduce a matrix to a scalar

\par
 c = accum (c, reduce_to_scalar(A)), reduce entries in a matrix
 to a scalar.  Not user-callable.  Does the work for GrB_*_reduce_TYPE,
 both matrix and vector.  This funciton tolerates zombies and does not
 delete them.  It does not tolerate pending tuples, so if they are present,
 all zombies are deleted and all pending tuples are assembled.
\par
 This function does not need to know if A is hypersparse or not, and its
 result is the same if A is in CSR or CSC format.
\par
 PARALLEL: a parallel reduction method.  All entries of the matrix
 must be reduce to a single scalar.
*/


/** \file GB_resize.c
\brief  GB_resize: change the size of a matrix

\par
 PARALLEL: simple parallelism; not a lot of work to unless the vector
 length is decreasing.  See Template/GB_prune_inplace.c
*/


/** \file GB_select.c
\brief  GB_select: apply a select operator; optionally transpose a matrix

\par
 C\<M\> = accum (C, select(A,k)) or select(A,k)').  This function is not
 user-callable.  It does the work for GxB_*_select.
 Compare this function with GrB_apply.
\par
 PARALLEL: do in parallel, but using an extra move of the data.
*/


/** \file GB_semiring_builtin.c
\brief  GB_semiring_builtin:  determine if semiring is built-in

\par
 Determine if A*B uses a built-in semiring, and if so, determine the
 opcodes and type codes of the semiring.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_setElement.c
\brief  GB_setElement: C(row,col) = scalar

\par
 Sets the value of single scalar, C(row,col) = scalar, typecasting from the
 type of scalar to the type of C, as needed.  Not user-callable; does the
 work for all GrB_*_setElement* functions.
\par
 If C(row,col) is already present in the matrix, its value is overwritten
 with the scalar.  Otherwise, if the mode determined by GrB_init is
 non-blocking, the tuple (i,j,scalar) is appended to a list of pending tuples
 to C.  When calling GrB_wait, these pending tuples are assembled.  They are
 also assembled if the mode is blocking.
\par
 GrB_setElement is the same as GrB_*assign with an implied SECOND accum
 operator whose ztype, xtype, and ytype are the same as C, with I=i, J=1, a
 1-by-1 dense matrix A (where nnz (A) == 1), no mask, mask not complemented,
 C_replace effectively false (its value is ignored), and A transpose
 effectively false (since transposing a scalar has no effect).
\par
 Compare this function with GB_extractElement.
\par
 not parallel: this function does O(log(..)) work and is already thread-safe.
*/


/** \file GB_shallow_cast.c
\brief  GB_shallow_cast: create a shallow copy of a matrix, optionally typecasted

\par
 C = (ctype) A.
\par
 Create a shallow copy of a matrix, possibly typecasted.
\par
 The CSR/CSC format of C and A can differ, but they have they same vlen and
 vdim.  This function is CSR/CSC agnostic, except that C_is_csc is used to
 set the C-\>is_csc state in C.
\par
 The values are a shallow copy unless they need to be typecasted.
\par
 The pattern is always a shallow copy.  No errors are checked except for
 out-of-memory conditions.  This function is not user-callable.  Shallow
 matrices are never passed back to the user.
\par
 Compare this function with GB_shallow_op.c
\par
 parallel: not here, but in GB_cast_array
*/


/** \file GB_shallow_op.c
\brief  GB_shallow_op:  create a shallow copy and apply a unary operator to a matrix

\par
 C = op (A)
\par
 Create a shallow copy of a matrix, applying an operator to the entries.
\par
 The CSR/CSC format of C and A can differ, but they have they same vlen and
 vdim.  This function is CSR/CSC agnostic, except that C_is_csc is used to
 set the C-\>is_csc state in C.
\par
 The values are typically not a shallow copy, unless no typecasting is needed
 and the operator is an identity operator.
\par
 The pattern is always a shallow copy.  No errors are checked except for
 out-of-memory conditions.  This function is not user-callable.  Shallow
 matrices are never passed back to the user.
\par
 Compare this function with GB_shallow_cast.c
\par
 parallel: not here, but in GB_apply_op.
*/


/** \file GB_size_t_multiply.c
\brief  GB_size_t_multiply:  multiply two size_t and guard against overflow

\par
 c = a*b but check for overflow
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_status_code.c
\brief  GB_status_code: return an error string describing the last error

\par
 GB_status_code: convert GrB_Info enum into a string
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_subassign.c
\brief  GB_subassign: C(Rows,Cols)\<M\> = accum (C(Rows,Cols),A) or A'

\par
 submatrix assignment: C(Rows,Cols)\<M\> = accum (C(Rows,Cols),A)
\par
 All GxB_*_subassign operations rely on this function.
\par
 With scalar_expansion = false, this method does the work for the standard
 GxB_*_subassign operations (GxB_Matrix_subassign, GxB_Vector_subassign,
 GxB_Row_subassign, and GxB_Col_subassign).  If scalar_expansion is true, it
 performs scalar assignment (the GxB_*_subassign_TYPE functions) in which
 case the input matrix A is ignored (it is NULL), and the scalar is used
 instead.
\par
 Compare with GB_assign, which uses M and C_replace differently
\par
 parallel: not here, but in GB_subassign_kernel
*/


/** \file GB_subassign_kernel.c
\brief  GB_subassign_kernel: C(I,J)\<M\> = accum (C(I,J), A)

\par
 Submatrix assignment: C(I,J)\<M\> = A, or accum (C (I,J), A), no transpose
\par
 All assignment operations rely on this function, including the GrB_*_assign
 operations in the spec, and the GxB_*_subassign operations that are a
 SuiteSparse:GraphBLAS extension to the spec:
\par
 GrB_Matrix_assign,
 GrB_Matrix_assign_TYPE,
 GrB_Vector_assign,
 GrB_Vector_assign_TYPE,
 GrB_Row_assign,
 GrB_Col_assign
\par
 GxB_Matrix_subassign,
 GxB_Matrix_subassign_TYPE,
 GxB_Vector_subassign,
 GxB_Vector_subassign_TYPE,
 GxB_Row_subassign,
 GxB_Col_subassign
\par
 This function handles the accumulator, and the mask M, and the C_replace
 option itself, without relying on GB_accum_mask or GB_mask.  The mask M has
 the same size as C(I,J) and A.  M(0,0) governs how A(0,0) is assigned
 into C(I[0],J[0]).  This is how GxB_subassign operates.  For GrB_assign, the
 mask M in this function is the SubMask, constructed via SubMask=M(I,J).
\par
 No transposed case is handled.  This function is also agnostic about the
 CSR/CSC format of C, A, and M.  The A matrix must have A-\>vlen == nI and
 A-\>vdim == nJ (except for scalar expansion, in which case A is NULL).  The
 mask M must be the same size as A, if present.
\par
 Any or all of the C, M, and/or A matrices may be hypersparse or standard
 non-hypersparse.
\par
 C is operated on in-place and thus cannot be aliased with the inputs A or M.
\par
 Since the pattern of C does not change here, C-\>p, C-\>h, C-\>nvec, and
 C-\>nvec_nonempty are constant.  C-\>x and C-\>i can be modified, but only one
 entry at a time.  No entries are shifted.  C-\>x can be modified, and C-\>i
 can be changed by turning an entry into a zombie, or by bringing a zombie
 back to life, but no entry in C-\>i moves in position.
\par
 PARALLEL: the pattern of C is not changing, except that zombies are
 introduced.  Pending tuples are added, but they could be added in any order.
 Each thread could keep its own list of pending tuples.  To parallelize this
 function, partition the list J, and call this function for each partition.
 Assuming that pending tuples are first added to a thread's private list, and
 then merged into C when done, C can be modified safely in parallel.  Also
 relies on GB_subref_symbolic, which can be done in parallel (but if J is
 partitioned, GB_subref_symbolic would be called independently for each
 partition).
*/


/** \file GB_subassign_scalar.c
\brief  GB_subassign_scalar: C(Rows,Cols)\<M\> = accum (C(Rows,Cols),x)

\par
 Assigns a single scalar to a submatrix:
\par
 C(Rows,Cols)\<M\> = accum (C(Rows,Cols),x)
\par
 This function does the work for GxB_Matrix_subassign_TYPE and
 GxB_Vector_subassign_[type], where [type] is one of the 11 types, or the
 type-generic macro suffix, \"_UDT\".
\par
 Compare with GB_assign_scalar, which uses M and C_replace differently
\par
 parallel: not here; see GB_subassign
*/


/** \file GB_subref_numeric.c
\brief  GB_subref_numeric: C = A(I,J) or C = (A(J,I))', extract the values

\par
 parallel: not here; see Template/GB_subref_template.c
*/


/** \file GB_subref_symbolic.c
\brief  GB_subref_symbolic: C = A(I,J), extract the pattern

\par
 parallel: not here; see Template/GB_subref_template.c
*/


/** \file GB_to_hyper.c
\brief  GB_to_hyper: convert a matrix to hyperspasre

\par
 On input, the matrix may have shallow A-\>p content; it is safely removed.
 On output, the matrix is always hypersparse (even if out of memory).  If the
 input matrix is non-hypersparse, it is given new A-\>p and A-\>h that are not
 shallow.  If the input matrix is already hypersparse, nothing is changed
 (and in that case A-\>p and A-\>h remain shallow on output if shallow on
 input). The A-\>x and A-\>i content is not changed; it remains in whatever
 shallow/non-shallow state that it had on input).
\par
 If an out-of-memory condition occurs, all content of the matrix is cleared.
\par
 PARALLEL: a reduction loop
*/


/** \file GB_to_hyper_conform.c
\brief  GB_to_hyper_conform: conform a matrix to its desired hypersparse format

\par
 The input matrix can have shallow A-\>p and/or A-\>h components.  If the
 hypersparsity is changed, these components are no longer shallow.  If the
 method fails and the matrix is shallow, all content is removed or freed.
\par
 parallel: not here
*/


/** \file GB_to_hyper_test.c
\brief  GB_to_hyper_test: test if a matrix should convert to hyperspasre

\par
 Returns true if a non-hypersparse matrix should be converted to hypersparse.
 Returns false if the matrix is already hypersparse.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_to_nonhyper.c
\brief  GB_to_nonhyper: convert a matrix to non-hypersparse form

\par
 On input, the matrix may have shallow A-\>p and A-\>h content; it is safely
 removed.  On output, the matrix is always non-hypersparse (even if out of
 memory).  If the input matrix is hypersparse, it is given a new A-\>p that is
 not shallow.  If the input matrix is already non-hypersparse, nothing is
 changed (and in that case A-\>p remains shallow on output if shallow on
 input). The A-\>x and A-\>i content is not changed; it remains in whatever
 shallow/non-shallow state that it had on input).
\par
 If an out-of-memory condition occurs, all content of the matrix is cleared.
\par
 PARALLEL: a few simple loops, no synchronization or reduction needed
*/


/** \file GB_to_nonhyper_test.c
\brief  GB_to_nonhyper_test: test if a matrix should convert to non-hyperspasre

\par
 Returns true if a hypersparse matrix should be converted to non-hypersparse.
 Returns false if the matrix is already non-hypersparse.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_transplant.c
\brief  GB_transplant: replace contents of one matrix with another

\par
 Transplant A into C, and then free A.  If any part of A is shallow, or if A
 must be typecasted, a deep copy is made into C.  Prior content of C is
 ignored.  Then A is freed, except for any shallow components of A which are
 left untouched (after unlinking them from A).  The resulting matrix C is not
 shallow.  This function is not user-callable.  The new type of C (ctype)
 must be compatible with A-\>type.
\par
 Only GrB_SUCCESS and GrB_OUT_OF_MEMORY are returned by this function.
\par
 PARALLEL: several large memcpy's of the whole matrix, and GB_cast_array,
 if a deep copy needs to be made.
*/


/** \file GB_transplant_conform.c
\brief  GB_transplant_conform: transplant T into C, then conform C

\par
 C = (type) T, then conform C to its desired hypersparsity.  T is freed
\par
 parallel: not here
*/


/** \file GB_transpose.c
\brief  GB_transpose:  C=A' or C=op(A'), with typecasting

\par
 Transpose a matrix, C=A', and optionally apply a unary operator and/or
 typecast the values.  The transpose may be done in place, in which case C or
 A are modified in place.  If the matrix to be transposed has more than one
 vector, it may have jumbled indices in its vectors, which must be sorted.
 If the input matrix has a single vector, it must be already sorted on input.
 The input matrix may have shallow components (even if in place), and the
 output may also have shallow components (even in the input matrix is not
 shallow).
\par
 This function is CSR/CSC agnostic; it sets the output matrix format from
 C_is_csc but otherwise ignores the CSR/CSC type of A and C.
\par
 If A_in is NULL, then C = (*Chandle) is transposed in place.  If out of
 memory, (*Chandle) is always returned as NULL, which frees the input matrix
 C if the transpose is done in place.
\par
 If A_in is not NULL and Chandle is NULL, then A is modified in place, and
 the A_in matrix is not freed when done.
\par
 PARALLEL: two methods are used: a bucket sort, and a qsort.  For the
 parallel case, it might be hard to parallelize the bucket sort.  The qsort
 is likely better, and the bucket sort could remain mostly sequential.  Most
 of the work is done in other functions (GB_transpose_bucket, GB_builder,
 etc), not here.  There are a few large loops here, and a large memcpy, that
 should be done in parallel.
*/


/** \file GB_transpose_bucket.c
\brief  GB_transpose_bucket: transpose and optionally typecast and/or apply operator

\par
 C = A' or op(A').  Optionally typecasts from A-\>type to the new type
 ctype, and/or optionally applies a unary operator.  No error checking is
 done by this function except for out-of-memory conditions.  Returns true if
 successful, or false if out of memory.  This function is not user-callable;
 use GrB_transpose or GrB_apply instead.
\par
 If an operator z=op(x) is provided, the type of z must be the same as the
 type of C.  The type of A must be compatible with the type of of x (A is
 typecasted into the type of x).  These conditions must be checked in the
 caller.
\par
 The input matrix A may have jumbled row indices; this is OK.
 The output matrix C will always have sorted row indices.
\par
 This function is agnostic for the CSR/CSC format of C and A.  C_is_csc is
 defined by the caller and assigned to C-\>is_csc, but otherwise unused.
 A-\>is_csc is ignored.
\par
 The input can be hypersparse or non-hypersparse.  The output is
 always non-hypersparse.
\par
 The result is never shallow.
\par
 If A is m-by-n in CSC format, with k nonzeros, the time and memory taken is
 O(m+n+k) if A is non-hypersparse, or O(m+k) if hypersparse.  This is fine if
 most rows and columns of A are non-empty, but can be very costly if A or A'
 is hypersparse.  In particular, if A is a non-hypersparse column vector with
 m \>\> k, the time and memory is O(m), which can be huge.  Thus, for
 hypersparse matrices, or for very sparse matrices, the qsort method should
 be used instead (see GB_transpose).
\par
 PARALLEL: the bucket transpose will not be simple to parallelize.  The qsort
 method of transpose would be more parallel.  This method might remain mostly
 sequential.  There
*/


/** \file GB_transpose_ix.c
\brief  GB_transpose_ix: transpose the values and pattern of a matrix

\par
 The values of A are typecasted to R_type, the type of the R matrix.
 A can be sparse or hypersparse, but R is not hypersparse.
\par
 The row pointers of the output matrix have already been computed, in Rp.
 Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
 version of Rp on *input*.  On output, however, Rp has been shifted down
 by one.  Rp [0:m-1] has been over written with Rp [1:m].  They can be
 shifted back, if needed, but GraphBLAS treats this array Rp, on input
 to this function, as a throw-away copy of Rp.
\par
 Compare with GB_transpose_op.c
\par
 PARALLEL: the bucket transpose will not be simple to parallelize.  The qsort
 method of transpose would be more parallel.  This method might remain mostly
 sequential.
*/


/** \file GB_transpose_op.c
\brief  GB_transpose_op: transpose and apply an operator to a matrix

\par
 R = op ((xtype) A')
\par
 The values of A are typecasted to op-\>xtype and then passed to the unary
 operator.  The output is assigned to R, which must be of type op-\>ztype; no
 output typecasting done with the output of the operator.
\par
 The row pointers of the output matrix have already been computed, in Rp.
 Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
 version of Rp on *input*.  On output, however, Rp has been shifted down
 by one.  Rp [0:m-1] has been over written with Rp [1:m].  They can be
 shifted back, if needed, but GraphBLAS treats this array Rp, on input
 to this function, as a throw-away copy of Rp.
\par
 Compare with GB_transpose_ix.c and GB_apply_op.c
\par
 PARALLEL: the bucket transpose will not be simple to parallelize.  The qsort
 method of transpose would be more parallel.  This method might remain mostly
 sequential.
*/


/** \file GB_type.c
\brief  GB_type: return the type of a matrix

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_user_build.c
\brief  GB_user_build: check inputs and build a matrix

\par
 This function implements GrB_Matrix_build_* and GrB_Vector_build_*.  It is
 not used by GrB_wait or GB_wait (see GB_builder instead).  For details on
 the algorithm, see GB_build.
\par
 parallel: not here, but in GB_build.
*/


/** \file GB_wait.c
\brief  GB_wait:  finish all pending computations on a single matrix

\par
 This function is typically called via the GB_WAIT(A) macro, except for
 GB_assign and GB_subassign.
\par
 The matrix A has zombies and/or pending tuples placed there by
 GrB_setElement and GrB_*assign.  Zombies must now be deleted, and pending
 tuples must now be assembled together and added into the matrix.
\par
 When the function returns, the matrix has been removed from the queue
 and all pending tuples and zombies have been deleted.  This is true even
 the function fails due to lack of memory (in that case, the matrix is
 cleared as well).
\par
 GrB_wait removes the head of the queue from the queue via
 GB_queue_remove_head, and then passes the matrix to this function.  Thus is
 is possible (and safe) for this matrix to operate on a matrix not in
 the queue.
\par
 If A is hypersparse, the time taken is at most O(nnz(A) + t log t), where t
 is the number of pending tuples in A, and nnz(A) includes both zombies and
 live entries.  There is no O(m) or O(n) time component, if A is m-by-n.
 If the number of non-empty vectors of A grows too large, then A can be
 converted to non-hypersparse.
\par
 If A is non-hypersparse, then O(n) is added in the worst case, to prune
 zombies and to update the vector pointers for A.
\par
 PARALLEL: this moves lots of data inside the matrix, but it is like a single
 merge of a mergesort.  Hard to do in parallel as currently written.  A
 better approach in parallel would be to do it like a matrix add (see
 GB_add).  Some work is done in GB_builder, which can be done in parallel.
*/


/** \file GrB_BinaryOp_free.c
\brief  GrB_BinaryOp_free: free a binary operator

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_BinaryOp_new.c
\brief  GrB_BinaryOp_new: create a new user-defined binary operator

\par
 GrB_BinaryOp_new is implemented both as a macro and a function.  Both are
 user-callable.  The macro is used by default since it can capture the name
 of the binary function.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Col_assign.c
\brief  GrB_Col_assign:    C\<M\>(Rows,col) = accum (C(Rows,col),u)

\par
 Compare with GxB_Col_subassign, which uses the M and C_replace differently
\par
 parallel: not here; see GB_assign
*/


/** \file GrB_Col_extract.c
\brief  GrB_Col_extract: w\<M\> = accum (w, A(I,j)) or A(j,I)'

\par
 Extract a single row or column from a matrix.  Note that in the
 GraphBLAS spec, row and column vectors are indistinguishable.  In this
 implementation, both are the same as an n-by-1 GrB_Matrix, except with
 restrictions on the matrix operations that can be performed on them.
\par
 parallel: not here; see GB_extract.
*/


/** \file GrB_Descriptor_free.c
\brief  GrB_Descriptor_free: free a descriptor

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Descriptor_new.c
\brief  GrB_Descriptor_new: create a new descriptor

\par
 Default values are set to GxB_DEFAULT
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Descriptor_set.c
\brief  GrB_Descriptor_set: set a field in a descriptor

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Matrix_apply.c
\brief  GrB_Matrix_apply: apply a unary operator to a matrix

\par
 C\<M\> = accum(C,op(A)) or accum(C,op(A'))
\par
 parallel: not here; see GB_apply
*/


/** \file GrB_Matrix_assign.c
\brief  GrB_Matrix_assign:    C\<M\>(Rows,Cols) = accum (C(Rows,Cols),A) or A'

\par
 parallel: not here; see GB_assign
*/


/** \file GrB_Matrix_assign_scalar.c
\brief  GrB_Matrix_assign_[SCALAR]: assign a scalar to matrix, via scalar expansion

\par
 Assigns a single scalar to a matrix:
\par
 C\<M\>(Rows,Cols) = accum(C(Rows,Cols),x)
\par
 The scalar x is implicitly expanded into a matrix A of size nRows-by-nCols,
 with each entry in A equal to x.
\par
 Compare with GxB_Matrix_subassign_scalar,
 which uses M and C_Replace differently.
\par
 The actual work is done in GB_assign_scalar.c.
\par
 parallel: not here; see GB_assign
*/


/** \file GrB_Matrix_build.c
\brief  GrB_Matrix_build: build a sparse GraphBLAS matrix

\par
 parallel: not here, but in GB_build.
*/


/** \file GrB_Matrix_clear.c
\brief  GrB_Matrix_clear: clears the content of a matrix

\par
 The A-\>x and A-\>i content is freed and the vector pointers A-\>p are set to
 zero.  This puts the matrix A in the same state it had after GrB_Matrix_new
 (\&A, ...).  The dimensions and type of A are not changed.
\par
 parallel: not here but in GB_clear.  
*/


/** \file GrB_Matrix_dup.c
\brief  GrB_Matrix_dup: make a deep copy of a sparse matrix

\par
 C = A, making a deep copy
\par
 parallel: not here, but in GB_dup. 
*/


/** \file GrB_Matrix_extract.c
\brief  GrB_Matrix_extract: C\<M\> = accum (C, A(I,J)) or A(J,I)'

\par
 parallel: not here, but in GB_subref_numeric.
*/


/** \file GrB_Matrix_extractElement.c
\brief  GrB_Matrix_extractElement: extract a single entry from a matrix

\par
 Extract the value of single scalar, x = A(row,col), typecasting from the
 type of A to the type of x, as needed.
\par
 Returns GrB_SUCCESS if A(row,col) is present, and sets x to its value.
 Returns GrB_NO_VALUE if A(row,col) is not present, and x is unmodified.
\par
 not parallel: this function does O(log(..)) work and is already thread-safe.
*/


/** \file GrB_Matrix_extractTuples.c
\brief  GrB_Matrix_extractTuples: extract all tuples from a matrix

\par
 Extracts all tuples from a matrix, like [I,J,X] = find (A) in MATLAB.  If
 any parameter I, J and/or X is NULL, then that component is not extracted.
 The size of the I, J, and X arrays (those that are not NULL) is given by
 nvals, which must be at least as large as GrB_nvals (\&nvals, A).  The values
 in the matrix are typecasted to the type of X, as needed.
\par
 If any parameter I, J, and/or X is NULL, that component is not extracted.
 So to extract just the row and col indices, pass I and J as non-NULL,
 and X as NULL.  This is like [I,J,~] = find (A).
\par
 parallel: not here, but in see GB_extractTuples.
*/


/** \file GrB_Matrix_free.c
\brief  GrB_Matrix_free: free a matrix

\par
 free all the content of a matrix.  After GrB_Matrix_free (\&A), A is set
 to NULL
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Matrix_ncols.c
\brief  GrB_Matrix_ncols: number of columns of a sparse matrix

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Matrix_new.c
\brief  GrB_Matrix_new: create a new matrix

\par
 The new matrix is nrows-by-ncols, with no entries in it.  Default format for
 an empty matrix is hypersparse CSC: A-\>p is size 2 and all zero, A-\>h is
 size 1, A-\>plen is 1, and contents A-\>x and A-\>i are NULL.  If this method
 fails, *A is set to NULL.
\par
 parallel: not here; see GB_new.
*/


/** \file GrB_Matrix_nrows.c
\brief  GrB_Matrix_nrows: number of rows of a sparse matrix

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Matrix_nvals.c
\brief  GrB_Matrix_nvals: number of entries in a sparse matrix

\par
 parallel: not here, but in GB_nvals (which forces completeion).
*/


/** \file GrB_Matrix_reduce_scalar.c
\brief  GrB_Matrix_reduce_scalar: reduce a matrix to a scalar

\par
 Reduce entries in a matrix to a scalar, c = accum (c, reduce_to_scalar(A)))
\par
 All entries in the matrix are \"summed\" to a single scalar t using the reduce
 monoid, which must be associative (otherwise the results are undefined).
 The result is either assigned to the output scalar c (if accum is NULL), or
 it accumulated in the result c via c = accum(c,t).  If A has no entries, the
 result t is the identity value of the monoid.  Unlike most other GraphBLAS
 operations, this operation uses an accum operator but no mask.
\par
 The actual work is done in GB_reduce_to_scalar.c.
\par
 parallel: not here but in GB_reduce_to_scalar
*/


/** \file GrB_Matrix_setElement.c
\brief  GrB_Matrix_setElement: set an entry in a matrix, C(row,col) = x

\par
 Set a single entry in a matrix, C(row,col) = x in MATLAB notation,
 typecasting from the type of x to the type of C, as needed.
\par
 not parallel: this function does O(log(..)) work and is already thread-safe.
*/


/** \file GrB_Monoid_free.c
\brief  GrB_Monoid_free:  free a monoid

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Monoid_new.c
\brief  GrB_Monoid_new:  create a new monoid

\par
 Create a new monoid with binary operator, z=op(x.y).  The three types of x,
 y, and z must all be the same, and the identity value must also have the
 same type.  No typecasting is done for the identity value.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Row_assign.c
\brief  GrB_Row_assign:    C\<M'\>(row,Cols) = accum (C(row,Cols),u')

\par
 Compare with GxB_Row_subassign, which uses M and C_replace differently
\par
 parallel: not here, but in GB_assign
*/


/** \file GrB_Semiring_free.c
\brief  GrB_Semiring_free: free a semiring

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Semiring_new.c
\brief  GrB_Semiring_new: create a new semiring

\par
 A GraphBLAS Semiring consists of two components: \"add\" and \"multiply\".
 These components imply three domains: ztype, xtype, and ytype.
\par
 The \"add\" is an associative and commutative monoid, which is a binary
 operator that works on a single type, ztype = add(ztype,ztype).  The add
 monoid also includes an identity value, called \"zero\", so that
 add(x,zero)=add(zero,x)=x.  For most algebras, this \"zero\" is a plain zero
 in the usual sense, but this is not the case for all algebras.  For example,
 for the max-plus algebra, the \"add\" operator is the function max(a,b), and
 the \"zero\" for this operator is -infinity since max(a,-inf)=max(-inf,a)=a.
\par
 The \"multiply\" is a binary operator z = multiply(x,y).  It has no
 restrictions, except that the type of z must exactly match the ztype
 of the add monoid.  That is, the types for the multiply operator are
 ztype = multiply (xtype, ytype).  When the semiring is applied to two
 matrices A and B, where (A,B) appear in that order in the method, the
 multiply operator is always applied as z = multiply (A(i,j),B(i,j)).  The
 two input operands always appear in that order.  That is, the multiply
 operator is not assumed to be commutative.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Type_free.c
\brief  GrB_Type_free:  free a user-defined type

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Type_new.c
\brief  GrB_Type_new: create a new user-defined type

\par
 GrB_Type_new is implemented both as a macro and a function.  Both are
 user-callable.  The default is to use the macro, since this allows the name
 of the type to be saved as a string, for subsequent error reporting by
 GrB_error.  It is also provided as a function so that applications that
 require a function instead of macro can access it.  User code can simply do
 \#undef GrB_Type_new before using the function.  This approach also places
 the function GrB_Type_new in the linkable SuiteSparse:GraphBLAS library so
 that it is visible for linking with applications in languages other than
 ANSI C.  The function version does not allow the name of the ctype to be
 saved in the new GraphBLAS type, however.  It is given the generic name.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_UnaryOp_free.c
\brief  GrB_UnaryOp_free: free a unary operator

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_UnaryOp_new.c
\brief  GrB_UnaryOp_new: create a new user-defined unary operator

\par
 GrB_UnaryOp_new is implemented both as a macro and a function.  Both are
 user-callable.  The macro is used by default since it can capture the name
 of the unary function.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Vector_apply.c
\brief  GrB_Vector_apply: apply a unary operator to a vector

\par
 parallel: not here; see GB_apply
*/


/** \file GrB_Vector_assign.c
\brief  GrB_Vector_assign:    w\<M\>(Rows) = accum (w(Rows),u)

\par
 Compare with GxB_Vector_subassign, which uses M and C_replace differently
\par
 parallel: not here; see GB_assign
*/


/** \file GrB_Vector_assign_scalar.c
\brief  GrB_Vector_assign_[SCALAR]: assign scalar to vector, via scalar expansion

\par
 Assigns a single scalar to a vector, w\<M\>(Rows) = accum(w(Rows),x)
 The scalar x is implicitly expanded into a vector u of size nRows-by-1,
 with each entry in u equal to x.
\par
 The actual work is done in GB_assign_scalar.c.
\par
 parallel: not here; see GB_assign
*/


/** \file GrB_Vector_build.c
\brief  GrB_Vector_build: build a sparse GraphBLAS vector

\par
 parallel: not here, but in GB_build.
*/


/** \file GrB_Vector_clear.c
\brief  GrB_Vector_clear: clears the content of a vector

\par
 parallel: not here but in GB_clear.
*/


/** \file GrB_Vector_dup.c
\brief  GrB_Vector_dup: make a deep copy of a sparse vector

\par
 w = u, making a deep copy
\par
 parallel: not here, but in GB_dup.
*/


/** \file GrB_Vector_extract.c
\brief  GrB_Vector_extract: w\<M\> = accum (w, u(I))

\par
 parallel: not here, but in GB_subref_numeric.
*/


/** \file GrB_Vector_extractElement.c
\brief  GrB_Vector_extractElement: extract a single entry from a vector

\par
 Extract a single entry, x = v(row), typecasting from the type
 of v to the type of x, as needed.
\par
 Returns GrB_SUCCESS if v(row) is present, and sets x to its value.
 Returns GrB_NO_VALUE if v(row) is not present, and x is unmodified.
\par
 not parallel: this function does O(log(..)) work and is already thread-safe.
*/


/** \file GrB_Vector_extractTuples.c
\brief  GrB_Vector_extractTuples: extract all tuples from a vector

\par
 Extracts all tuples from a column, like [I,~,X] = find (v) in MATLAB.  If
 any parameter I and/or X is NULL, then that component is not extracted.  The
 size of the I and X arrays (those that are not NULL) is given by nvals,
 which must be at least as large as GrB_nvals (\&nvals, v).  The values in the
 typecasted to the type of X, as needed.
\par
 If any parameter I and/or X is NULL, that component is not extracted.  So to
 extract just the row indices, pass I as non-NULL, and X as NULL.  This is
 like [I,~,~] = find (v) in MATLAB.
\par
 parallel: not here, but in see GB_extractTuples.
*/


/** \file GrB_Vector_free.c
\brief  GrB_Vector_free: free a sparse vector

\par
 free all the content of a vector.  After GrB_Vector_free (\&v), v is set
 to NULL
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Vector_new.c
\brief  GrB_Vector_new: create a new vector

\par
 The new vector is n-by-1, with no entries in it.
 A-\>p is size 2 and all zero.  Contents A-\>x and A-\>i are NULL.
 If this method fails, *v is set to NULL.  Vectors are not hypersparse,
 so format is standard CSC, and A-\>h is NULL.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_Vector_nvals.c
\brief  GrB_Vector_nvals: number of nonzeros in a sparse vector

\par
 parallel: not here, but in GB_nvals (which forces completeion).
*/


/** \file GrB_Vector_reduce_scalar.c
\brief  GrB_Vector_reduce_scalar: reduce a vector to a scalar

\par
 Reduce entries in a vector to a scalar, c = accum (c, reduce_to_scalar(u))
\par
 All entries in the vector are \"summed\" to a single scalar t using the reduce
 monoid, which must be associative (otherwise the results are undefined).
 The result is either assigned to the output scalar c (if accum is NULL), or
 it accumulated in the result c via c = accum(c,t).  If the u has no entries,
 the result t is the identity value of the monoid.  Unlike most other
 GraphBLAS operations, this operation uses an accum operator but no mask.
\par
 The actual work is done in GB_reduce_to_scalar.c.
\par
 parallel: not here but in GB_reduce_to_scalar
*/


/** \file GrB_Vector_setElement.c
\brief  GrB_Vector_setElement: set an entry in a vector, w (row) = x

\par
 Set a single scalar, w(row) = x, typecasting from the type of x to
 the type of w as needed.
\par
 not parallel: this function does O(log(..)) work and is already thread-safe.
*/


/** \file GrB_Vector_size.c
\brief  GrB_Vector_size: dimension of a sparse vector

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_eWiseAdd_Matrix.c
\brief  GrB_eWiseAdd_Matrix: matrix element-wise operations, set union

\par
 C\<M\> = accum (C,A+B) and variations.
\par
 parallel: not here but in GB_add
*/


/** \file GrB_eWiseAdd_Vector.c
\brief  GrB_eWiseAdd_Vector: vector element-wise operations, set union

\par
 w\<M\> = accum (w,u+v)
\par
 parallel: not here but in GB_add
*/


/** \file GrB_eWiseMult_Matrix.c
\brief  GrB_eWiseMult_Matrix: matrix element-wise operations, using set intersection

\par
 C\<M\> = accum (C,A.*B) and variations.
\par
 parallel: not here but in GB_emult
*/


/** \file GrB_eWiseMult_Vector.c
\brief  GrB_eWiseMult_Vector: vector element-wise multiplication

\par
 w\<M\> = accum (w,u.*v)
\par
 parallel: not here but in GB_emult
*/


/** \file GrB_error.c
\brief  GrB_error: return an error string describing the last error

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_finalize.c
\brief  GrB_finalize: finalize GraphBLAS

\par
 GrB_finalize must be called as the last GraphBLAS function, per the
 GraphBLAS C API Specification.  Only one user thread can call this
 function.  Results are undefined if more than one thread calls this
 function at the same time.
\par
 not parallel: this function does O(1) work
*/


/** \file GrB_init.c
\brief  GrB_init: initialize GraphBLAS

\par
 GrB_init (or GxB_init) must called before any other GraphBLAS operation.
 GrB_finalize must be called as the last GraphBLAS operation.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GrB_mxm.c
\brief  GrB_mxm: matrix-matrix multiply

\par
 C\<M\> = accum (C,A*B) and variations.
\par
 The input matrices A and B are optionally transposed, as determined by the
 Descriptor desc.
\par
 parallel: not here, see GB_AxB_parallel
*/


/** \file GrB_mxv.c
\brief  GrB_mxv: matrix-vector multiply

\par
 w\<M\> = accum (w,t) where t = A*u or A'*u (u is never transposed)
\par
 The input matrix A is optionally transposed, as determined by the
 Descriptor desc.
\par
 parallel: not here, see GB_AxB_parallel
*/


/** \file GrB_reduce_to_column.c
\brief  GrB_reduce_to_column: reduce a matrix to a column

\par
 parallel: not here, see GB_reduce_to_column
*/


/** \file GrB_transpose.c
\brief  GrB_transpose: transpose a sparse matrix

\par
 C\<M\> = accum (C,A') or accum (C,A)
\par
 parallel: not here; see GB_transpose and GB_shallow_cast
*/


/** \file GrB_vxm.c
\brief  GrB_vxm: vector-matrix multiply

\par
 w'\<M'\> = accum (w',t) where t = u'*A or u'*A'
\par
 Rows w', u', and M' are simply columns w, u, and M.  Thus:
 w\<M\> = accum (w,t) where t = A'*u or A*u, but with the multiply operator
 flipped.  The input descriptor for A, inp1, is also negated.
\par
 parallel: not here, see GB_AxB_parallel
*/


/** \file GrB_wait.c
\brief  GrB_wait: finish all pending computations

\par
 The GrB_wait function forces all pending operations to complete.  Blocking
 mode is as if the GrB_wait operation is called whenever a GraphBLAS
 operation returns to the user.
\par
 The non-blocking mode can have side effects if user-defined functions have
 side effects or if they rely on global variables, which are not under the
 control of GraphBLAS.  Suppose the user creates a user-defined operator that
 accesses a global variable.  That operator is then used in a GraphBLAS
 operation, which is left pending.  If the user then changes the global
 variable before pending operations complete, the pending operations will be
 eventually computed with this different value.
\par
 Worse yet, a user-defined operator can be freed before it is needed to
 finish a pending operation.  To avoid this, call GrB_wait before modifying
 any global variables relied upon by user-defined operators, or before
 freeing any user-defined types, operators, monoids, or semirings.
\par
 parallel: not here; see GB_wait.  Or can also do all matrices in
 the queue in parallel.
\par
 No other user threads should call any GraphBLAS function while GrB_wait is
 executing, except for parallel calls to GrB_wait.  Results are undefined
 otherwise, since GrB_wait could modify a matrix that another user thread is
 attempting to access.  However, it is safe for multiple user threads to call
 GrB_wait at the same time.  The user threads will then safely cooperate to
 complete all the matrices in the queue, in parallel.
*/


/** \file GxB_BinaryOp_fprint.c
\brief  GxB_BinaryOp_fprint: print and check a GrB_BinaryOp object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_BinaryOp_xtype.c
\brief  GxB_BinaryOp_xtype: return the type of x for z=f(x,y)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_BinaryOp_ytype.c
\brief  GxB_BinaryOp_ytype: return the type of y for z=f(x,y)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_BinaryOp_ztype.c
\brief  GxB_BinaryOp_ztype: return the type of z for z=f(x,y)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Col_subassign.c
\brief  GxB_Col_subassign: C(Rows,col)\<M\> = accum (C(Rows,col),u)

\par
 Compare with GrB_Col_assign, which uses M and C_replace differently
\par
 parallel: not here; see GB_subassign_kernel
*/


/** \file GxB_Desc_get.c
\brief  GxB_Desc_get: get a field in a descriptor

\par
 This is identical to GxB_Descriptor_get, just with a different order of the
 parameters.  The last argument is a pointer whose type depends on the
 field.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Desc_set.c
\brief  GxB_Desc_set: set a field in a descriptor

\par
 This is identical to GrB_Descriptor_set, except that the last argument is a
 pointer whose type depends on the field.  For the four descriptor fields
 in the spec, the type is the same as GrB_Descriptor_set (a scalar of
 type GrB_Desc_Value).
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Descriptor_fprint.c
\brief  GxB_Descriptor_fprint: print and check a GrB_Descriptor object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Descriptor_get.c
\brief  GxB_Descriptor_get: get a field in a descriptor

\par
 Use GxB_Desc_get instead; this is kept for backward compatibility.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Global_Option_get.c
\brief  GxB_Global_Option_get: get a global default option for all future matrices

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Global_Option_set.c
\brief  GxB_Global_Option_set: set a global default option for all future matrices

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Matrix_Option_get.c
\brief  GxB_Matrix_Option_get: get an option in a matrix

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Matrix_Option_set.c
\brief  GxB_Matrix_option_set: set an option in a matrix

\par
 parallel: not here, but in GB_transpose.
*/


/** \file GxB_Matrix_export_CSC.c
\brief  GxB_Matrix_export_CSC: export a matrix in CSC format

\par
 parallel: not here
*/


/** \file GxB_Matrix_export_CSR.c
\brief  GxB_Matrix_export_CSR: export a matrix in CSR format

\par
 parallel: not here
*/


/** \file GxB_Matrix_export_HyperCSC.c
\brief  GxB_Matrix_export_HyperCSC: export a matrix in hypersparse CSC format

\par
 parallel: not here
*/


/** \file GxB_Matrix_export_HyperCSR.c
\brief  GxB_Matrix_export_HyperCSR: export a matrix in hypersparse CSR format

\par
 parallel: not here
*/


/** \file GxB_Matrix_fprint.c
\brief  GxB_Matrix_fprint: print and check a GrB_Matrix object

\par
 parallel: not needed; used only for testing and debugging.
*/


/** \file GxB_Matrix_import_CSC.c
\brief  GxB_Matrix_import_CSC: import a matrix in CSC format

\par
 not parallel: O(1) time
*/


/** \file GxB_Matrix_import_CSR.c
\brief  GxB_Matrix_import_CSR: import a matrix in CSR format

\par
 not parallel: O(1) time
*/


/** \file GxB_Matrix_import_HyperCSC.c
\brief  GxB_Matrix_import_HyperCSC: import a matrix in hypersparse CSC format

\par
 not parallel: O(1) time
*/


/** \file GxB_Matrix_import_HyperCSR.c
\brief  GxB_Matrix_import_HyperCSR: import a matrix in hypersparse CSR format

\par
 not parallel: O(1) time
*/


/** \file GxB_Matrix_resize.c
\brief  GxB_Matrix_resize: change the size of a matrix

\par
 parallel: see GB_resize
*/


/** \file GxB_Matrix_select.c
\brief  GxB_Matrix_select: select entries from a matrix

\par
 C\<M\> = accum(C,select(A,k)) or accum(C,select(A',))
\par
 parallel: not here; see GB_select
*/


/** \file GxB_Matrix_subassign.c
\brief  GxB_Matrix_subassign: C(Rows,Cols)\<M\> = accum (C(Rows,Cols),A) or A'

\par
 Compare with GrB_Matrix_assign, which uses M and C_replace differently
\par
 parallel: not here; see GB_subassign_kernel
*/


/** \file GxB_Matrix_subassign_scalar.c
\brief  GxB_Matrix_subassign_[SCALAR]: assign to submatrix, via scalar expansion

\par
 Assigns a single scalar to a submatrix:
\par
 C(Rows,Cols)\<M\> = accum(C(Rows,Cols),x)
\par
 The scalar x is implicitly expanded into a matrix A of size nRows-by-nCols,
 with each entry in A equal to x.
\par
 Compare with GrB_Matrix_assign_scalar,
 which uses M and C_Replace differently.
\par
 The actual work is done in GB_subassign_scalar.c.
\par
 parallel: not here; see GB_subassign_kernel
*/


/** \file GxB_Matrix_type.c
\brief  GxB_Matrix_type: return the type of a matrix

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Monoid_fprint.c
\brief  GxB_Monoid_fprint: print and check a GrB_Monoid object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Monoid_identity.c
\brief  GxB_Monoid_identity: return the identity of a monoid

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Monoid_operator.c
\brief  GxB_Monoid_operator: return the op of a monoid

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Monoid_terminal.c
\brief  GxB_Monoid_terminal: return the terminal of a monoid (if any)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Monoid_terminal_new.c
\brief  GxB_Monoid_terminal_new:  create a new monoid with a terminal value

\par
 Identical to GrB_Monoid_new, except that a terminal value is specified.  No
 typecasting is done for the terminal value.  Its type must match the
 identity value.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Row_subassign.c
\brief  GxB_Row_subassign: C(row,Cols)\<M'\> = accum (C(row,Cols),u')

\par
 Compare with GrB_Row_assign, which uses the M and C_replace differently
\par
 parallel: not here, see GB_subassign_kernel
*/


/** \file GxB_SelectOp_fprint.c
\brief  GxB_SelectOp_fprint: print and check a GrB_SelectOp object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_SelectOp_free.c
\brief  GxB_SelectOp_free: free a select operator

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_SelectOp_new.c
\brief  GxB_SelectOp_new: create a new user-defined select operator

\par
 GxB_SelectOp_new is implemented both as a macro and a function.  Both are
 user-callable.  The macro is used by default since it can capture the name
 of the select function.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_SelectOp_xtype.c
\brief  GxB_SelectOp_xtype: return the type of x for z=f(x)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Semiring_add.c
\brief  GxB_Semiring_add: return the additive monoid of a semiring

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Semiring_fprint.c
\brief  GxB_Semiring_fprint: print and check a GrB_Semiring object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Semiring_multiply.c
\brief  GxB_Semiring_multiply: return the multiply operator of a semiring

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Type_fprint.c
\brief  GxB_Type_fprint: print and check a GrB_Type object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Type_size.c
\brief  GxB_Type_size: return the size of a type

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_UnaryOp_fprint.c
\brief  GxB_UnaryOp_fprint: print and check a GrB_UnaryOp object

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_UnaryOp_xtype.c
\brief  GxB_UnaryOp_xtype: return the type of x for z=f(x)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_UnaryOp_ztype.c
\brief  GxB_UnaryOp_ztype: return the type of z for z=f(x)

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_Vector_export.c
\brief  GxB_Vector_export: export a vector in CSR/CSC format

\par
 The indices are returned in sorted order.
\par
 not parallel: O(1) time
*/


/** \file GxB_Vector_fprint.c
\brief  GxB_Vector_fprint: print and check a GrB_Vector object

\par
 parallel: not needed; used only for testing and debugging.
*/


/** \file GxB_Vector_import.c
\brief  GxB_Vector_import: import a vector in CSR/CSC format

\par
 The indices must appear in sorted order.
\par
 not parallel: O(1) time
*/


/** \file GxB_Vector_resize.c
\brief  GxB_Vector_resize: change the size of a vector

\par
 parallel: not here; see GB_resize
*/


/** \file GxB_Vector_select.c
\brief  GxB_Vector_select: select entries from a vector

\par
 parallel: not here; see GB_select
*/


/** \file GxB_Vector_subassign.c
\brief  GxB_Vector_subassign: w(Rows)\<M\> = accum (w(Rows),u)

\par
 Compare with GrB_Vector_assign, which uses M and C_replace differently
\par
 parallel: not here; see GB_subassign_kernel
*/


/** \file GxB_Vector_subassign_scalar.c
\brief  GxB_Vector_subassign_[SCALAR]: assign scalar to vector, via scalar expansion

\par
 Assigns a single scalar to a subvector, w(Rows)\<M\> = accum(w(Rows),x)
 The scalar x is implicitly expanded into a vector u of size nRows-by-1,
 with each entry in u equal to x.
\par
 The actual work is done in GB_subassign_scalar.c.
\par
 parallel: not here; see GB_subassign_kernel
*/


/** \file GxB_Vector_type.c
\brief  GxB_Vector_type: return the type of a vector

\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GxB_init.c
\brief  GxB_init: initialize GraphBLAS and declare malloc/calloc/realloc/free to use

\par
 GrB_init (or GxB_init) must called before any other GraphBLAS operation.
 GrB_finalize must be called as the last GraphBLAS operation.  GxB_init is
 identical to GrB_init, except that it allows the user application to define
 the malloc/calloc/realloc/free functions that SuiteSparse:GraphBLAS will
 use.  The functions cannot be modified once GraphBLAS starts.
\par
 not parallel: this function does O(1) work and is already thread-safe.
\par
 Examples:
*/


/** \file GxB_kron.c
\brief  GxB_kron: Kronecker product

\par
 parallel: not here; see GB_kron_kernel
*/


/** \file all_user_objects.c
\brief  SuiteSparse/GraphBLAS/Source/all_user_objects.c

\par
 This file is constructed automatically by cmake and m4 when GraphBLAS is
 compiled, from the Config/user_def*.m4 and *.m4 files in User/.  Do not edit
 this file directly.  It contains references to internally-defined functions
 and objects inside GraphBLAS, which are not user-callable.
*/


/** \file GraphBLAS.h
\brief  GraphBLAS.h: definitions for the GraphBLAS package

\par
 SuiteSparse:GraphBLAS is an full implementation of the GraphBLAS standard,
 which defines a set of sparse matrix operations on an extended algebra of
 semirings, using an almost unlimited variety of operators and types.  When
 applied to sparse adjacency matrices, these algebraic operations are
 equivalent to computations on graphs.  GraphBLAS provides a powerful and
 expressive framework creating graph algorithms based on the elegant
 mathematics of sparse matrix operations on a semiring.
\par
 This GraphBLAS.h file contains GraphBLAS definitions for user applications
 to \#include.  Functions and variables with the prefix GB_ need to be defined
 in this file and are thus technically visible to the user, but they must not
 be accessed in user code.  They are here only so that the ANSI C11 _Generic
 feature can be used in the user-accessible polymorphic functions.  For
 example GrB_free is a macro that uses _Generic to select the right method,
 depending on the type of its argument.
\par
 This implementation fully conforms to the GraphBLAS API Specification, but
 also includes functions and features that are extensions to the spec.  These
 are cataloged here and tagged with \"SPEC.\"
\par
 All functions and definitions that are extensions to the spec are given
 names of the form GxB_* for functions, built-in objects, and macros, so it
 is clear which are in the spec and which are extensions.  Extensions with
 the name GxB_* are user-accessible in SuiteSparse:GraphBLAS but cannot be
 guaranteed to appear in all GraphBLAS implementations.  In the future, if
 any GxB_* functions are included as-is in the GraphBLAS API spec with GrB_*
 names, the prior GxB_* variants that appear here will be kept for backward
 compatibility.  If they must change for inclusion in the spec, a reasonable
 attempt will be made to keep the prior GxB_* variant alongside the GrB_*
 version, also for backward compatibility.
*/


/** \file GB_2type_template.c
\brief  GB_2type_template.c: 2-type switch factory

\par
 This is a generic switch factory for creating 121 workers that operate on
 two built-in data types (11 types each), to be \#include'd in another file.
 GB_WORKER(type1,type2) is a macro defined in the \#including file, where
 type1 and type2 are the built-in types corresponding to code1 and code2,
 respectively or (void *) for a user-defined type.  The last statement of
 GB_WORKER should be a break or return since it doesn't appear here.
\par
 User-defined types are not handled.
\par
 GB_shallow_op and GB_transpose_op use this template to create workers that
 apply unary operators.  Those functions \#define GB_BOP(x) for the boolean
 unary operator, GB_IOP(x) for integers, and GB_FOP(x) for floating-point.
 The selection of these operators is controlled by code1.
\par
 parallel: not here.
*/


/** \file GB_AxB_Gustavson_flipxy.c
\brief  GB_AxB_Gustavson_flipxy: C=A*B and C\<M\>=A*B

\par
 parallel: not here.
*/


/** \file GB_AxB_Gustavson_mask.c
\brief  GB_AxB_Gustavson_mask:  compute C\<M\>=A*B using the Gustavson method, with M

\par
 This file is \#include'd in GB_AxB_Gustavson.c, and Template/GB_AxB.c, the
 latter of which expands into Generated/GB_AxB__* for all built-in semirings.
\par
 The pattern of C has not been computed, but nnz(M) has given an upper bound
 on nnz(C) so this method will not run out of memory.  This is Gustavson's
 method, extended to handle hypersparse matrices, arbitrary semirings, and a
 mask matrix M.
\par
 The mask is present in this case (see GB_AxB_Gustavson_nomask otherwise).
 This method takes Omega(nnz(M)) time when exploiting the mask, so a very
 dense mask can be costly to exploit.  Thus, this method is not used, and
 GB_AxB_Gustavson_nomask is used instead, if the total flop count is less
 than nnz(M).
\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_Gustavson_meta.c
\brief  GB_AxB_Gustavson_meta: C=A*B and C\<M\>=A*B

\par
 parallel: not here
*/


/** \file GB_AxB_Gustavson_nomask.c
\brief  GB_AxB_Gustavson_nomask:  C=A*B using Gustavson method, precomputed pattern

\par
 This file is \#include'd in GB_AxB_Gustavson.c, and Template/GB_AxB.c, the
 latter of which expands into Generated/GB_AxB__* for all built-in semirings.
\par
 The pattern of C has already been computed in the symbolic phase of
 GB_AxB_Gustavson.  This is Gustavson's method, extended to handle
 hypersparse matrices and arbitrary semirings.
\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_Gustavson_symbolic.c
\brief  GB_AxB_Gustavson_symbolic: C=A*B symbolic analysis

\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_compare_template.c
\brief  GB_AxB_compare_template.c: switch factory for C=A*B with comparator ops

\par
 A template file \#include'd in GB_AxB_factory.c
\par
 The multiply operator is a comparator: EQ, NE, GT, LT, GE, LE.
 z=f(x,y): x and x are either boolean or non-boolean.  z is boolean.
\par
 Since z is boolean, the only monoids available are OR, AND, XOR, and EQ
 All the other four (max==plus==or, min==times==and) are redundant.
 Those opcodes have been renamed, and handled by the OR and AND workers
 defined here.
\par
 There is one special case to consider.  For boolean x, y, and z, the
 function z=NE(x,y) is the same as z=XOR(x,y).  If z is boolean, the multiply
 operator NE has already been renamed XOR by GB_AxB_semiring_builtin, and
 thus NE will never use the boolean case, below.  Thus it is removed with the
 \#ifndef GB_NO_BOOLEAN.
\par
 parallel: not here
*/


/** \file GB_AxB_dot_cij.c
\brief  GB_AxB_dot_cij: compute C(i,j) = A(:,i)'*B(:,j)

\par
 computes C(i,j) = A (:,i)'*B(:,j) via sparse dot product.
\par
 parallel: the parallelism will be handled outside this code, in
 GB_AxB_parallel.  This work is done by a single thread.
*/


/** \file GB_AxB_dot_compmask.c
\brief  GB_AxB_dot_compmask:  C\<!M\>=A'*B via dot products

\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_dot_flipxy.c
\brief  GB_AxB_dot_flipxy: C=A'*B or C\<M\>=A'*B via dot products

\par
 parallel: not here
*/


/** \file GB_AxB_dot_mask.c
\brief  GB_AxB_dot_mask:  C\<M\>=A'*B via dot products

\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_dot_meta.c
\brief  GB_AxB_dot_meta: C=A'*B or C\<M\>=A'*B via dot productes

\par
 This file is \#include'd into GB_AxB_dot.c for the generic case, and in the
 hard-coded semirings, Generated/GB_AxB__*_*_.c.  It constructs the
 dot-product variant of sparse matrix multiplication, C=A'*B, without
 transposing A.
\par
 parallel: not here.
*/


/** \file GB_AxB_dot_nomask.c
\brief  GB_AxB_dot_nomask:  C=A'*B via dot products

\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_factory.c
\brief  GB_AxB_factory: switch factory for built-in methods for C=A*B

\par
 This is used by GB_AxB_Gustavson_builtin.c, GB_AxB_dot.c and GB_AxB_heap.c
 to create built-in versions of sparse matrix-matrix multiplication.  The
 \#include'ing file defines the GB_AxB_WORKER macro, and mult_opcode,
 add_opcode, xycode, and zcode
\par
 parallel: not here
*/


/** \file GB_AxB_heap_flipxy.c
\brief  GB_AxB_heap_flipxy: compute C\<M\>=A*B or C=A*B using a heap-based method

\par
 parallel: not here
*/


/** \file GB_AxB_heap_mask.c
\brief  GB_AxB_heap_mask:  compute C\<M\>=A*B using the heap method, with M present

\par
 This file is \#include'd in GB_AxB_heap_meta.c.
\par
 if GB_MASK_CASE is defined, then the mask matrix M is present.  Otherwise it
 is not present.  However, this method takes Omega(nnz(M)) time when
 exploiting the mask, so a very dense mask can be costly to exploit.  Thus,
 the mask is not passed to the heap method if the total flop count is less
 than nnz(M).
\par
 parallel: this could be done in parallel, but the parallelism will be
 handled outside this code, in GB_AxB_parallel.  This work is done by a
 single thread.
*/


/** \file GB_AxB_heap_meta.c
\brief  GB_AxB_heap_meta: compute C\<M\>=A*B or C=A*B using a heap-based method

\par
 parallel: not here
*/


/** \file GB_AxB_template.c
\brief  GB_AxB_template.c: switch factory for each built-in monoid for C=A*B

\par
 A template file \#include'd in GB_AxB_factory.c
\par
 This file is used for 17 operators.  The multiply operator is combined here
 with 40 or 44 monoids to create 40 or 44 unique semiring workers.
\par
      FIRST, SECOND, MIN, MAX, PLUS, MINUS, TIMES, DIV,
      ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
      LAND, LOR, LXOR.
\par
 For all of them, the types of x, y, and z are the same.
 There are 40 non-boolean monoids and 4 boolean monoids defined here.
\par
 GB_NO_BOOLEAN is defined for 12 of these multiply operators in the
 \#include'ing file (min, max, plus, minus, times, div, is*) since those 12
 multiply operators are redundant and have been renamed.  For these 12, the
 boolean monoids are not needed.
\par
 parallel: not here
*/


/** \file GB_assoc_template.c
\brief  GB_assoc_template.c: switch factory for associative operators

\par
 FUTURE:: write Generator/GB_assoc.c, and use it to create 66 files:
 Generator/GB_assoc__[operator]_[type].  Each file has 4 functions for the
 kernels for GB_reduce_to_scalar, GB_build_factory, and GB_reduce_to_column
 (2 kernels).  (but 22 files with FIRST and SECOND just have one kernel for
 GB_build_factory).
\par
 This is a generic body of code for creating hard-coded versions of code for
 44 combinations of associative operators and built-in types: 10 types (all
 but boolean) with min, max, plus, and times, and one type (boolean) with
 \"or\", \"and\" \"xor\" and \"eq\"
\par
 If GB_INCLUDE_SECOND_OPERATOR is defined then an additional 11 built-in
 workers for the SECOND operator are also created, and 11 for FIRST, for
 GB_build_factory.
\par
 parallel: not here
*/


/** \file GB_critical_section.c
\brief  Source/Template/GB_critical_section: execute code in a critical section

\par
 All access to the global matrix queue, via GB_queue_* operations, must
 be done through a critical section.  No other part of SuiteSparse:GraphBLAS
 uses this critical section; it is only used for accessing the global matrix
 queue via GB_queue_*.   All GB_queue_* operations use the GB_CRITICAL macro
 to check the result, and if the critical section fails (ok == false),
 they return GrB_PANIC.
\par
 Critical sections for Windows threads and ANSI C11 threads are listed below
 as drafts, but these threading models are not yet supported.
\par
 not parallel: this function does O(1) work and is already thread-safe.
*/


/** \file GB_heap.h
\brief  GB_heap: a Heap data structure and its operations

\par
 The Heap is an array of GB_Elements: Heap [1..nheap].  Each entry in the
 Heap is a GB_Element, with a key and name.
\par
 These functions are only used by the heap method for C=A*B.
 See Source/Template/GB_AxB_heap_mask.c.
\par
 parallel: the parallelism will be handled outside this code, in
 GB_AxB_parallel.  This work is done by a single thread.
*/


/** \file GB_matrix.h
\brief  GB_matrix.h: definitions for GrB_Matrix and GrB_Vector

\par
 The GrB_Matrix and GrB_Vector objects are different names for the same
 content.  A GrB_Vector is held as an m-by-1 non-hypersparse CSC matrix.
 This file is \#include'd in GB.h to define the GB_Matrix_opaque and
 GB_Vector_opaque structs.  It would be cleaner to define just one opaque
 struct, and then GrB_Matrix and GrB_Vector would be typedef'd as pointers to
 the same struct, but then the compiler gets confused with Generic(x).
*/


/** \file GB_ops_template.c
\brief  GB_ops_template.c: built-in unary and binary functions and operators

\par
 This file is \#include'd many times in GB_ops.c to define the built-in unary
 and binary operators.  In that file, GB_TYPE is a built-in C type (bool,
 int8_t, uint64_t, double, etc), and GB(x) is the corresponding macro that
 creates the function name (GB_*_BOOL, GB_*_INT8, etc).
\par
 parallel: not here; no executable code
*/


/** \file GB_ops_template.h
\brief  GB_ops_template.h: define the unary and binary functions and operators

\par
 This file is \#include'd many times in GB.h to define the unary and binary
 functions.  That file defines the GB(x) macro, which is GB_\#\#x\#\#_BOOL,
 GB_\#\#x\#\#_INT8, etc.
*/


/** \file GB_prune_inplace.c
\brief  GB_prune_inplace: prune a matrix in place

\par
 The function that \#include's this file defines a GB_PRUNE macro that defines
 how entries are pruned in place from the matrix.  This task is similar to
 GB_select, except that here the pruning is done in place, whereas GB_select
 constructs a new matrix T with the pruned version of A.
\par
 This code is used by GB_wait to delete zombies, and by GB_resize to delete
 entries outside the resized dimensions, if A-\>vlen decreases.
\par
 PARALLEL: requires a reduction-style parallelism
*/


/** \file GB_qsort_template.c
\brief  GB_qsort_template: sort an n-by-GB_K list of integers

\par
 This file is \#include'd in GB_qsort*.c to create specific versions for GB_K
 = 1, 2, and 3.  Requires an inline or macro definition of the GB_lt
 function.  The GB_lt function has the form GB_lt (A,i,B,j) and returns true
 if A[i]\<B[j].
\par
 All of these functions are static; there will be versions of them in each
 variant of GB_qsort*, with the same names.  They are called only by the
 GB_qsort* function in the \#include'ing file.
*/


/** \file GB_semiring_template.c
\brief  GB_semiring_template.c: built-in unary and binary functions and operators

\par
 This file is \#include'd many times in GB_ops.c to define the built-in
 semirings.  That file has defined either GB_BOOLEAN, or GB_TYPE as one of
 the 10 non-boolean types.
\par
 Using built-in types and operators, 960 unique semirings can be built.  This
 count excludes redundant Boolean operators (for example GrB_TIMES_BOOL and
 GrB_LAND are different operators but they are redundant since they always
 return the same result):
\par
 680 semirings with a multiply operator TxT -\> T where T is non-Boolean, from
 the complete cross product of:
\par
      4 add monoids (MIN, MAX, PLUS, TIMES)
      17 multiply operators:
          (FIRST, SECOND, MIN, MAX, PLUS, MINUS, TIMES, DIV,
           ISEQ, ISNE, ISGT, ISLT, ISGE, ISLE,
           LOR, LAND, LXOR)
      10 non-Boolean types, T
\par
      a single instance of this file creates 4*17 = 68 semirings of this
      form, of one type T, when T is not BOOL
\par
 240 semirings with a comparison operator TxT -\> bool, where T is
 non-Boolean, from the complete cross product of:
\par
      4 Boolean add monoids: (LAND, LOR, LXOR, EQ)
      6 multiply operators: (EQ, NE, GT, LT, GE, LE)
      10 non-Boolean types, T
\par
      a single instance of this file creates 4*6 = 24 semirings of this form,
      of one type T, when T is not BOOL
\par
 40 semirings with purely Boolean types, bool x bool -\> bool, from the
 complete cross product of:
\par
      4 Boolean add monoids (LAND, LOR, LXOR, EQ)
      10 multiply operators:
          (FIRST, SECOND, LOR, LAND, LXOR, EQ, GT, LT, GE, LE)
\par
      a single instance of this file creates all 4*10 = 40 purely Boolean
      semirings, when T is BOOL and GB_BOOLEAN is defined
\par
 parallel: not here; no executable code
*/


/** \file GB_subref_template.c
\brief  GB_subref_template: C = A(I,J), C = (A(J,I))', or C = pattern (A(I,J))

\par
 This template creates two functions:
\par
 GB_subref_numeric: numeric extraction
\par
      Sparse submatrix reference, C = A(I,J), extracting the values.  This is
      an internal function called by GB_extract that does the work of the
      user-callable GrB_*_extract methods.  It is also called by GB_assign to
      extract the submask.  No pending tuples or zombies appear in A.
\par
 GB_subref_symbolic: symbolic extraction
\par
      Sparse submatrix reference, C = A(I,J), extracting the pattern, not the
      values.  This function is called only by GB_subassign_kernel.  Symbolic
      extraction creates a matrix C with the same pattern (C-\>p and C-\>i) as
      numeric extraction, but with different values, C-\>x.  For numeric
      extracion if C(inew,jnew) = A(i,j), the value of A(i,j) is copied into
      C(i,j).  For symbolic extraction, its *pointer* is copied into C(i,j).
      Suppose an entry A(i,j) is held in Ai [pa] and Ax [pa], and it appears
      in the output matrix C in Ci [pc] and Cx [pc].  Then the two methods
      differ as follows:
\par
          this is the same:
\par
          i = Ai [pa] ;           // index i of entry A(i,j)
\par
          aij = Ax [pa] ;         // value of the entry A(i,j)
\par
          Ci [pc] = inew ;        // index inew of C(inew,jnew)
\par
          this is different:
\par
          Cx [pc] = aij ;         // for numeric extraction
\par
          Cx [pc] = pa ;          // for symbolic extraction
\par
      GB_subref_symolic is created if GB_SYMBOLIC is defined.  The function
      is used by GB_subassign_kernel, which uses it to extract the pattern of
      C(I,J), for the submatrix assignment C(I,J)=A.  GB_subref_symbolic
      needs to deal with zombie entries.  The GB_subassign_kernel caller uses
      this function on its C matrix, which is called A here because it is not
      modified here.
\par
      Reading a zombie entry:  A zombie entry A(i,j) has been marked by
      flipping its index.  The value of a zombie is not important, just its
      presence in the pattern.  All zombies have been flipped (i \< 0), and
      all regular entries are not flipped (i \>= 0).  Zombies are entries that
      have been marked for deletion but have not been removed from the matrix
      yet, since it's more efficient to delete zombies all at once rather
      than one at a time.
\par
      GB_subref_pattern may encounter zombies in A.  It is zombie-agnostic,
      doing nothing to them and treating them like regular entries.  Their
      normal index must be used, not their flipped indices.  The output
      matrix C contains all unflipped indices, and its references to zombies
      and regular entries are identical.  Zombies in A are dealt with later.
      They cannot be detected in the output C matrix, but they can be
      detected in A.  Since pa = Cx [pc] holds the position of the entry in
      A, the entry is a zombie if Ai [pa] has been flipped.
\par
 Neither function is user-callable.
\par
 The output matrix is passed as a handle, and created by this function, just
 like GrB_Matrix_new or GrB_Matrix_dup.
\par
 This function is agnostic as to the CSR/CSC format, except for C_is_csc
 which is the requested format of the output matrix C (either CSR or CSC).
 It is assigned to C-\>is_csc but otherwise has no effect on this function.
\par
 PARALLEL: the list J can be partitioned, and the subref can be done in
 parallel and the results concatenated.
*/


/** \file GB_AxB.c
\brief  GB_AxB:  hard-coded C=A*B and C\<M\>=A*B

\par
 Unless this file is Generator/GB_AxB.c, do not edit it (auto-generated)
*/


/** \file GB_AxB.h
\brief Source/Generator/GB_AxB.h: used to create GB_AxB__semirings.h
*/

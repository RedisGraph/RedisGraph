
//      FIXME: make dndn its own kernel, when both A and B are both dense
//      also make a dndn kernel where A and/or B are bitmap/dense
// GB_BUCKET_DNVS:      A(:,i) is dense and B(:,j) is very sparse
// GB_BUCKET_DNSP:      A(:,i) is dense and B(:,j) is sparse
// GB_BUCKET_VSDN:      B(:,j) is dense and A(:,j) is very sparse
// GB_BUCKET_SPDN:      B(:,j) is dense and A(:,j) is sparse
//      FIXME: the four D*S* and *S*D* buckets should be split into different
//      kernels when A is bitmap/full and B is sparse/hypersparse or visa
//      versa.  They would still need a preprocessing phase1 to split the
//      entries of C into 2 buckets.


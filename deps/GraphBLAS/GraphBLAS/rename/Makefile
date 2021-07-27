go:
	nm -gD ../../build/libgraphblas.so | grep -v "__" > lib
	grep " GrB_" lib \
            | grep -v GrB_Type_new \
            | grep -v GrB_UnaryOp_new \
            | grep -v GrB_BinaryOp_new > temp.h
	grep " GxB_" lib \
            | grep -v GxB_SelectOp_new >> temp.h
	grep " GB_"  lib >> temp.h
	awk -f rename.awk < temp.h > GB_rename.h
	rm temp.h lib

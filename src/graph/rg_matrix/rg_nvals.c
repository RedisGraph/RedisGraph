/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

//#include "RG.h"
//#include "rg_matrix.h"
//
//GrB_Info RG_Matrix_nvals    // get the number of entries in a matrix
//(
//    GrB_Index *nvals,       // matrix has nvals entries
//    const RG_Matrix A       // matrix to query
//) {
//	ASSERT(A      !=  NULL);
//	ASSERT(nvals  !=  NULL);
//
//	GrB_Matrix  m;
//	GrB_Info    info;
//
//	GrB_Index  m_nvals   =  0;
//	GrB_Index  dp_nvals  =  0;
//	GrB_Index  dm_nvals  =  0;
//
//	// nvals = nvals(M) + nvals(DP) - nvals(DM)
//
//	m   =  RG_MATRIX_M(A);
//
//	info = GrB_Matrix_nvals(&m_nvals, m);
//	ASSERT(info == GrB_SUCCESS);
//	RG_Matrix_DP_nvals(&dp_nvals, A);
//	RG_Matrix_DM_nvals(&dm_nvals, A);
//
//	*nvals = m_nvals + dp_nvals - dm_nvals;
//	return info;
//}
//
//// get the number of entries in the delta minus matrix
//void RG_Matrix_DM_Nvals
//(
//	GrB_Index *nvals,       // matrix has nvals entries
//	const RG_Matrix C       // matrix to query
//) {
//	ASSERT(C     != NULL);
//	ASSERT(nvals != NULL);
//
//	*nvals = C->dm_nvals;
//}
//
//// inc delta minus nvals
//void RG_Matrix_DM_IncNvals
//(
//	RG_Matrix C
//) {
//	ASSERT(C != NULL);
//
//	C->dm_nvals++;
//}
//
//// dec delta minus nvals
//void RG_Matrix_DM_DecNvals
//(
//	RG_Matrix C
//) {
//	ASSERT(C != NULL);
//
//	C->dm_nvals--;
//}
//
//// set delta minus nvals
//void RG_Matrix_DM_SetNvals
//(
//	RG_Matrix C,
//	GrB_Index nvals
//) {
//	ASSERT(C != NULL);
//
//	C->dm_nvals = nvals;
//}
//
//// get the number of entries in the delta plus matrix
//void RG_Matrix_DP_Nvals
//(
//	GrB_Index *nvals,       // matrix has nvals entries
//	const RG_Matrix C       // matrix to query
//) {
//	ASSERT(C     != NULL);
//	ASSERT(nvals != NULL);
//
//	*nvals = C->dp_nvals;
//}
//
//// inc delta plus nvals
//void RG_Matrix_DP_IncNvals
//(
//	RG_Matrix C
//) {
//	ASSERT(C != NULL);
//
//	C->dp_nvals++;
//}
//
//// dec delta plus nvals
//void RG_Matrix_DP_DecNvals
//(
//	RG_Matrix C
//) {
//	ASSERT(C != NULL);
//
//	C->dp_nvals--;
//}
//
//// set delta plus nvals
//void RG_Matrix_DP_SetNvals
//(
//	RG_Matrix C,
//	GrB_Index nvals
//) {
//	ASSERT(C != NULL);
//
//	C->dp_nvals = nvals;
//}
//

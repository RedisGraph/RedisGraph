/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "pagerank.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

int pagerank(int argc, const char *argv[]) {
    // 'NodeLabel', "RelationshipType", {config})
    const char *nodeLabel = argv[0];
    const char *relationshipType = argv[1];
    // const char *config = argv[0];
    
    // Get node label matrix.
    GrB_Matrix label;
    // Get relation matrix.
    GrB_Matrix relation;

    GrB_Index rows;
    GrB_Index cols;

    GrB_Matrix_nrows(&rows, )
    GrB_Matrix m;
    GrB_Matrix_new(&m, GrB_BOOL, )

}

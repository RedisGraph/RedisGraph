/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "../ops/op_node_by_label_scan.h"

// this optimization scans through each label-scan operation
// in case the node being scaned is associated with multiple labels
// e.g. MATCH (n:A:B) RETURN n
// we will prefer to scan through the label with the least number of nodes
// for the above example if NNZ(A) < NNZ(B) we will want to iterate over the A
//
// in-case this optimization changed the label scanned e.g. from A to B
// it will have to patch the following conditional traversal removing B operand
// and adding A back
//
// consider MATCH (n:A:B)-[:R]->(m) RETURN m
// 
// Scan(A)
// Traverse B*R
//
// if we switch from Scan(A) to Scan(B)
// we will have to update the traversal pattern from B*R to A*R
//
// Scan(B)
// Traverse A*R


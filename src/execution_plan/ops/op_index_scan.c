#include "op_index_scan.h"

OpBase *NewIndexScanOp(QueryGraph *qg, Graph *g, Node **node, IndexIterator *iter) {
  return (OpBase*)NewIndexScan(qg, g, node, iter);
}

IndexScan* NewIndexScan(QueryGraph *qg, Graph *g, Node **node, IndexIterator *iter) {

  IndexScan *indexScan = malloc(sizeof(IndexScan));
  /* TODO I think these are getting set to dummy nodes rather than proper
   * graph nodes. It is set before getting accessed by Consume, but an assertion
   * in the FilterTree _applyPredicateFilters fails if they are not set here.
   * Determine appropriate settings. */
  indexScan->node = node;
  indexScan->_node = *node;
  indexScan->iter = iter;
  indexScan->g = g;

  // Set our Op operations
  indexScan->op.name = "Index Scan";
  indexScan->op.type = OPType_INDEX_SCAN;
  indexScan->op.consume = IndexScanConsume;
  indexScan->op.reset = IndexScanReset;
  indexScan->op.free = IndexScanFree;
  indexScan->op.modifies = NewVector(char*, 1);

  Vector_Push(indexScan->op.modifies, QueryGraph_GetNodeAlias(qg, *node));

  // Make/retrieve index of size n*n
  int n = g->node_count;
  GrB_Matrix_new(&indexScan->result_matrix, GrB_BOOL, n, n);

  return indexScan;
}

OpResult IndexScanConsume(OpBase *opBase, QueryGraph* graph) {
  IndexScan *op = (IndexScan*)opBase;
  GrB_Index node_id = (GrB_Index)IndexIterator_Next(op->iter);

  // TODO 0 is a valid id, so fix this
  if (node_id == 0) {
    return OP_DEPLETED;
  }
  // Necessary? Store graph on op or iter?
  *op->node = Graph_GetNode(op->g, node_id);
  if(*op->node == NULL) {
    return OP_DEPLETED;
  }

  GrB_Matrix_setElement_BOOL(op->result_matrix, true, node_id, node_id);

  return OP_OK;
}

OpResult IndexScanReset(OpBase *ctx) {
  IndexScan *indexScan = (IndexScan*)ctx;

  /* Restore original node. */
  *indexScan->node = indexScan->_node;
  IndexIterator_Reset(indexScan->iter);

  return OP_OK;
}

void IndexScanFree(OpBase *op) {
  IndexScan *indexScan = (IndexScan *)op;
  IndexIterator_Free(indexScan->iter);
  free(indexScan);
}
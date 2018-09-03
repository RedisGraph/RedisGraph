#include "./index.h"
#include "../ast_common.h"

AST_IndexNode* New_AST_IndexNode(const char *label, char *property, AST_IndexOpType optype) {
  AST_IndexNode *indexOp = malloc(sizeof(AST_IndexNode));
  indexOp->label = label;
  indexOp->property = property;
  indexOp->operation = optype;
  return indexOp;
}

void Free_AST_IndexNode(AST_IndexNode *indexNode) {
  if(indexNode != NULL) {
    free(indexNode);
  }
}

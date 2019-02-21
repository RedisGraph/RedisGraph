/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./skip.h"

AST_SkipNode* New_AST_SkipNode(size_t n) {
    AST_SkipNode* skipNode = malloc(sizeof(AST_SkipNode));
    skipNode->skip = n;
    return skipNode;
}

void Free_AST_SkipNode(AST_SkipNode *skipNode) {
    if(skipNode) free(skipNode);
}

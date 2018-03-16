#include "node_block.h"
#include "assert.h"

NodeBlock *NodeBlock_New() {
    NodeBlock *block = calloc(1, sizeof(NodeBlock) - sizeof(Node*) + NODEBLOCK_CAP * sizeof(Node));
    block->cap = NODEBLOCK_CAP;
    block->size = 0;
    block->next = NULL;
    return block;
}

void NodeBlock_Free(NodeBlock *block) {
    assert(block);
    
    // for(int i = 0; i < block->size; i++) {
    //     Node_Free(&(block->nodes[i]));
    // }

    free(block);
}

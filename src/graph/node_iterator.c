#include "node_iterator.h"
#include "assert.h"

NodeIterator *NodeIterator_New(NodeBlock *block, int start_pos, int end_pos) {
    assert(block && start_pos >=0 && end_pos >= start_pos);
    
    NodeIterator *iter = malloc(sizeof(NodeIterator));
    iter->_start_block = block;
    iter->_current_block = block;
    iter->_block_pos = start_pos % NODEBLOCK_CAP;
    iter->_start_pos = start_pos;
    iter->_current_pos = iter->_start_pos;
    iter->_end_pos = end_pos;
    return iter;
}

Node *NodeIterator_Next(NodeIterator *iter) {
    assert(iter);
    // Have we reached the end of our iterator?
    if(iter->_current_pos >= iter->_end_pos || iter->_current_block == NULL) return NULL;
    
    // Get node at current position.
    NodeBlock *block = iter->_current_block;
    Node *n = &block->nodes[iter->_block_pos];

    // Advance to next position.
    iter->_block_pos++;
    iter->_current_pos++;

    // Advance to next block if current block consumed.
    if(iter->_block_pos == NODEBLOCK_CAP) {
        iter->_block_pos = 0;
        iter->_current_block = iter->_current_block->next;
    }

    return n;
}

void NodeIterator_Reset(NodeIterator *iter) {
    assert(iter);
    iter->_block_pos = iter->_start_pos % NODEBLOCK_CAP;
    iter->_current_block = iter->_start_block;
    iter->_current_pos = iter->_start_pos;
}

void NodeIterator_Free(NodeIterator *iter) {
    assert(iter);
    free(iter);
}
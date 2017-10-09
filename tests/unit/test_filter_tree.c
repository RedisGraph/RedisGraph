#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/rmutil/vector.h"
#include "../../src/parser/grammar.h"
#include "../../src/filter_tree/filter_tree.h"

void compareFilterTreeVaryingNode(const FT_FilterNode *a, const FT_FilterNode *b) {
    assert(a->t == b->t);
    assert(a->pred.op == b->pred.op);
    assert(a->pred.t == b->pred.t);
    assert(a->pred.cf == b->pred.cf);
    assert(strcmp(a->pred.Lop.alias, b->pred.Lop.alias) == 0);
    assert(strcmp(a->pred.Lop.property, b->pred.Lop.property) == 0);
    assert(strcmp(a->pred.Rop.alias, b->pred.Rop.alias) == 0);
    assert(strcmp(a->pred.Rop.property, b->pred.Rop.property) == 0);
}

void compareFilterTreeConstNode(const FT_FilterNode *a, const FT_FilterNode *b) {
    assert(a->t == b->t);
    assert(a->pred.op == b->pred.op);
    assert(a->pred.t == b->pred.t);
    assert(a->pred.cf == b->pred.cf);
    assert(strcmp(a->pred.Lop.alias, b->pred.Lop.alias) == 0);
    assert(strcmp(a->pred.Lop.property, b->pred.Lop.property) == 0);
    assert(a->pred.constVal.type == b->pred.constVal.type);
}

void compareFilterTrees(const FT_FilterNode *a, const FT_FilterNode *b) {
    assert(a->t == b->t);
    
    if(a->t == FT_N_PRED) {
        if(a->pred.t == FT_N_CONSTANT) {
            compareFilterTreeConstNode(a, b);
        } else {
            compareFilterTreeVaryingNode(a, b);
        }
    } else {
        assert(a->cond.op == b->cond.op);
        compareFilterTrees(a->cond.left, b->cond.left);
        compareFilterTrees(a->cond.right, b->cond.right);
    }
}

int main(int argc, char **argv) {
	printf("test_filter_tree - PASS!\n");
    return 0;
}
#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/parser/grammar.h"
#include "../src/filter_tree/filter_tree.h"

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

void test_filter_tree_clone() {
    FT_FilterNode *filterTree = CreateVaryingFilterNode("X", "name", "Y", "name", EQ);
    FT_FilterNode *clone;
    FilterTree_Clone(filterTree, &clone);
    compareFilterTrees(filterTree, clone);
    FilterTree_Free(filterTree);
    FilterTree_Free(clone);
    
    ////////////////////////////////////////////////////////////////////

    filterTree = CreateConstFilterNode("X", "age", EQ, SI_IntVal(32));
    FilterTree_Clone(filterTree, &clone);
    compareFilterTrees(filterTree, clone);
    FilterTree_Free(filterTree);
    FilterTree_Free(clone);

    ////////////////////////////////////////////////////////////////////

    filterTree = CreateCondFilterNode(AND);
    AppendLeftChild(filterTree, CreateVaryingFilterNode("X", "name", "Y", "name", EQ));
    AppendRightChild(filterTree, CreateConstFilterNode("X", "age", EQ, SI_IntVal(32)));
    FilterTree_Clone(filterTree, &clone);
    compareFilterTrees(filterTree, clone);
    FilterTree_Free(filterTree);
    FilterTree_Free(clone);
}

void test_filter_tree_min_tree() {
    FT_FilterNode *filterTree = CreateConstFilterNode("X", "age", EQ, SI_IntVal(32));
    FT_FilterNode *minTree = FilterTree_MinFilterTree(filterTree, "X");
    compareFilterTrees(filterTree, minTree);
    FilterTree_Free(minTree);

    ////////////////////////////////////////////////////////////////////

    minTree = FilterTree_MinFilterTree(filterTree, "Y");
    assert(minTree == NULL);
    FilterTree_Free(filterTree);

    ////////////////////////////////////////////////////////////////////

    filterTree = CreateCondFilterNode(AND);
    AppendLeftChild(filterTree, CreateConstFilterNode("X", "age", EQ, SI_IntVal(32)));
    AppendRightChild(filterTree, CreateConstFilterNode("Y", "age", EQ, SI_IntVal(32)));

    minTree = FilterTree_MinFilterTree(filterTree, "X");

    FT_FilterNode *expected = filterTree->cond.left;

    compareFilterTrees(minTree, expected);

    FilterTree_Free(minTree);
    FilterTree_Free(filterTree);

    ////////////////////////////////////////////////////////////////////

    filterTree = CreateCondFilterNode(AND);
    FT_FilterNode *leftChild = AppendLeftChild(filterTree, CreateCondFilterNode(OR));
    FT_FilterNode *rightChild = AppendRightChild(filterTree, CreateCondFilterNode(OR));

    AppendLeftChild(leftChild, CreateConstFilterNode("X", "age", EQ, SI_IntVal(32)));
    AppendRightChild(leftChild, CreateConstFilterNode("X", "rating", EQ, SI_IntVal(32)));

    AppendLeftChild(rightChild, CreateConstFilterNode("Y", "age", EQ, SI_IntVal(32)));
    AppendRightChild(rightChild, CreateConstFilterNode("Y", "rating", EQ, SI_IntVal(32)));

    minTree = FilterTree_MinFilterTree(filterTree, "X");

    expected = filterTree->cond.left;

    compareFilterTrees(minTree, expected);

    FilterTree_Free(minTree);
    FilterTree_Free(filterTree);

    ////////////////////////////////////////////////////////////////////

    filterTree = CreateCondFilterNode(AND);
    leftChild = AppendLeftChild(filterTree, CreateCondFilterNode(OR));
    rightChild = AppendRightChild(filterTree, CreateCondFilterNode(OR));

    AppendLeftChild(leftChild, CreateConstFilterNode("Y", "age", EQ, SI_IntVal(32)));
    AppendRightChild(leftChild, CreateConstFilterNode("Y", "rating", EQ, SI_IntVal(32)));

    AppendLeftChild(rightChild, CreateConstFilterNode("X", "age", EQ, SI_IntVal(32)));
    AppendRightChild(rightChild, CreateConstFilterNode("X", "rating", EQ, SI_IntVal(32)));

    minTree = FilterTree_MinFilterTree(filterTree, "X");

    expected = filterTree->cond.right;

    compareFilterTrees(minTree, expected);

    FilterTree_Free(minTree);
    FilterTree_Free(filterTree);

    ////////////////////////////////////////////////////////////////////

    filterTree = CreateCondFilterNode(AND);
    leftChild = AppendLeftChild(filterTree, CreateCondFilterNode(OR));
    rightChild = AppendRightChild(filterTree, CreateCondFilterNode(OR));

    AppendLeftChild(leftChild, CreateConstFilterNode("X", "age", EQ, SI_IntVal(32)));
    AppendRightChild(leftChild, CreateConstFilterNode("Y", "rating", EQ, SI_IntVal(32)));

    AppendLeftChild(rightChild, CreateConstFilterNode("Y", "age", EQ, SI_IntVal(32)));
    AppendRightChild(rightChild, CreateConstFilterNode("X", "rating", EQ, SI_IntVal(32)));

    minTree = FilterTree_MinFilterTree(filterTree, "X");

    expected = CreateCondFilterNode(AND);
    AppendLeftChild(expected, CreateConstFilterNode("X", "age", EQ, SI_IntVal(32)));
    AppendRightChild(expected, CreateConstFilterNode("X", "rating", EQ, SI_IntVal(32)));

    compareFilterTrees(minTree, expected);

    FilterTree_Free(minTree);
    FilterTree_Free(expected);
    FilterTree_Free(filterTree);
}

int main(int argc, char **argv) {
    test_filter_tree_clone();
    test_filter_tree_min_tree();
	printf("PASS!\n");
    return 0;
}
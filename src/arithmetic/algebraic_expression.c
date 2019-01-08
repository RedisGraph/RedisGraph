/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "algebraic_expression.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include <assert.h>

static void _AlgebraicExpression_PrintNode(AlgebraicExpressionNode *root, int indent) {
  if (root == NULL) return;

  if (root->type == AL_OPERAND) {
      if (root->operand.entity_is_node) {
        Node *operand = (Node*)root->operand.entity;
        printf("%*s(%s:%s)\n", indent, "", operand->alias, operand->label);
      } else {
        Edge *operand = (Edge*)root->operand.entity;
        printf("%*s[%s:%s]\n", indent, "", operand->alias, operand->relationship);
      
      }
  } else {
      char *op;
      switch (root->operation.op) {
        case (AL_EXP_ADD):
          printf("%*s%s\n", indent, "", "ADD");
          _AlgebraicExpression_PrintNode(root->operation.l, indent + 4);
          _AlgebraicExpression_PrintNode(root->operation.r, indent + 4);
          break;
        case (AL_EXP_MUL):
          printf("%*s%s\n", indent, "", "MUL");
          _AlgebraicExpression_PrintNode(root->operation.l, indent + 4);
          _AlgebraicExpression_PrintNode(root->operation.r, indent + 4);
          break;
        case (AL_EXP_TRANSPOSE):
          printf("%*s%s\n", indent, "", "TRANS");
          if (root->operation.l) _AlgebraicExpression_PrintNode(root->operation.l, indent + 4);
          if (root->operation.r) _AlgebraicExpression_PrintNode(root->operation.r, indent + 4);
          break;
      }
  }
}

static void AlgebraicExpression_PrintTree(AlgebraicExpressionNode *root) {
    _AlgebraicExpression_PrintNode(root, 0);
}

/* For every referenced edge, add edge source and destination nodes
 * as referenced entities. */
void _referred_edge_ends(TrieMap *ref_entities, const QueryGraph *q) {
    char *alias;
    tm_len_t len;
    void *value;
    Vector *aliases = NewVector(char*, 0);
    TrieMapIterator *it = TrieMap_Iterate(ref_entities, "", 0);

    /* Scan ref_entities for referenced edges.
     * note, we can not modify triemap which scanning it. */
    while(TrieMapIterator_Next(it, &alias, &len, &value)) {
        Edge *e = QueryGraph_GetEdgeByAlias(q, alias);
        if(!e) continue;

        // Remember edge ends.        
        Vector_Push(aliases, e->src->alias);
        Vector_Push(aliases, e->dest->alias);
    }

    // Add edges ends as referenced entities.
    for(int i = 0; i < Vector_Size(aliases); i++) {
        Vector_Get(aliases, i, &alias);
        TrieMap_Add(ref_entities, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }

    TrieMapIterator_Free(it);
    Vector_Free(aliases);
}

/* Variable length edges require their own algebraic expression,
 * therefor mark both variable length edge ends as referenced. */
void _referred_variable_length_edges(TrieMap *ref_entities, Vector *matchPattern, const QueryGraph *q) {
    Edge *e;
    AST_GraphEntity *match_element;

    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        Vector_Get(matchPattern, i, &match_element);
        if(match_element->t != N_LINK) continue;
        
        AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        if(!edge->length) continue;

        e = QueryGraph_GetEdgeByAlias(q, edge->ge.alias);
        TrieMap_Add(ref_entities, e->src->alias, strlen(e->src->alias), NULL, TrieMap_DONT_CARE_REPLACE);
        TrieMap_Add(ref_entities, e->dest->alias, strlen(e->dest->alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }
}

// TODO might be unused
int AlgebraicExpression_OperandCount(AlgebraicExpressionNode *root) {
  if (root == NULL) {
    return 0;
  } else if (root->type == AL_OPERAND) {
    return 1;
  }
  int increase = AlgebraicExpression_OperandCount(root->operation.l);
  increase += AlgebraicExpression_OperandCount(root->operation.r);
  return increase;
}

// TODO might be unused
AlgebraicExpressionNode* AlgebraicExpression_Pop(AlgebraicExpressionNode **root) {
  assert(root);

  AlgebraicExpressionNode *cur = *root;
  if (cur == NULL) return NULL;

  if (cur->type == AL_OPERAND) {
    *root = NULL;
    return cur;
  }

  AlgebraicExpressionNode *ret = AlgebraicExpression_Pop(&cur->operation.l);
  if (ret) return ret;

  ret = AlgebraicExpression_Pop(&cur->operation.r);
  assert(ret);
  return ret;
}

AlgebraicExpressionNode* AlgebraicExpression_Append(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child) {
    assert(root && root->type == AL_OPERATION);
    assert(root->operation.r == NULL);
    if (root->operation.l == NULL) {
      AlgebraicExpressionNode_AppendLeftChild(root, child);
      return root;
    }

    AlgebraicExpressionNode *inner = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
    AlgebraicExpressionNode_AppendLeftChild(inner, child);
    AlgebraicExpressionNode_AppendRightChild(root, inner);
    return inner;
}

TrieMap* _referred_entities(const AST *ast, const QueryGraph *qg) {
  TrieMap *ref_entities = NewTrieMap();
  ReturnClause_ReferredEntities(ast->returnNode, ref_entities);
  CreateClause_ReferredEntities(ast->createNode, ref_entities);
  WhereClause_ReferredEntities(ast->whereNode, ref_entities);
  DeleteClause_ReferredEntities(ast->deleteNode, ref_entities);    
  SetClause_ReferredEntities(ast->setNode, ref_entities);
  _referred_edge_ends(ref_entities, qg);
  _referred_variable_length_edges(ref_entities, ast->matchNode->_mergedPatterns, qg);

  return ref_entities;
}

AE_Unit** _AddNode(TrieMap *referred_entities, const AST *ast, int *visited, AE_Unit **units, AE_Unit *unit, Node *cur) {
  // Node is labeled, inject operand
  if (cur->mat != NULL) {
    AlgebraicExpressionNode *node_operand = AlgebraicExpressionNode_NewOperandNode(cur, true);
    unit->exp_leaf = AlgebraicExpression_Append(unit->exp_leaf, node_operand);
  }

  // Node is referenced
  void *referred = TrieMap_Find(referred_entities, cur->alias, strlen(cur->alias));
  if (referred != TRIEMAP_NOTFOUND) {
    if (unit->src == NULL) {
      unit->src = cur;
    } else {
      assert(unit->dest == NULL);
      unit->dest = cur;
    }
  }
  Edge *e;
  // Outgoing edges
  for (int i = 0; i < Vector_Size(cur->outgoing_edges); i ++) {
    Vector_Get(cur->outgoing_edges, i, &e);
    int edge_id = AST_GetAliasID(ast, e->alias);
    if (visited[edge_id] != -1) continue; // already used edge
    visited[edge_id] = 1;

    // Mapped full unit, start the next
    if (unit->dest) {
      AE_Unit *new_unit = rm_calloc(1, sizeof(AE_Unit));
      units = array_append(units, new_unit);
      unit = new_unit;
      unit->exp_root = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
      unit->exp_leaf = unit->exp_root;
      unit->src = cur;
    }
    // Referenced edge
    referred = TrieMap_Find(referred_entities, e->alias, strlen(e->alias));
    if (referred != TRIEMAP_NOTFOUND) {
      // TODO deal with multiple referenced edges without referenced intermediate nodes
      assert(unit->edge == NULL);
      unit->edge = e;
    }
    // TODO If edge is variable length or covers multiple relation types,
    // might want to handle that here
    AlgebraicExpressionNode *edge_matrix = AlgebraicExpressionNode_NewOperandNode(e, false);
    unit->exp_leaf = AlgebraicExpression_Append(unit->exp_leaf, edge_matrix);
    units = _AddNode(referred_entities, ast, visited, units, unit, e->dest);
  }
  return units;
}

AE_Unit** _build_units(TrieMap *referred_entities, const AST *ast, int *visited, Node *root) {
  // Set up units array
  AE_Unit **units = array_new(AE_Unit *, 1);
  AE_Unit *unit = rm_calloc(1, sizeof(AE_Unit));
  units = array_append(units, unit);

  unit->exp_root = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
  unit->exp_leaf = unit->exp_root;
  
  // Traverse connected component
  units = _AddNode(referred_entities, ast, visited, units, unit, root);

  return units;
}

AE_Unit** AlgebraicExpression_FromComponent(const AST *ast, TrieMap *referred_entities, Node *src) {
  // TrieMap *unique_edges = NewTrieMap();
  // AlgebraicExpressionNode *exp = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
  
  // Move to ExecutionPlan to reuse visited array for all components
  int cardinality = ast->_aliasIDMapping->cardinality;
  int visited[cardinality];
  memset(visited, -1, cardinality * sizeof(int));
  AE_Unit **units = _build_units(referred_entities, ast, visited, src);
  // _AddNode(referred_entities, exp, src);

  TrieMap_Free(referred_entities, TrieMap_NOP_CB);

  return units;
}

AE_Unit** _AE_ScanOp(Node *n) {
  // TODO this is rather dumb, think of alternatives
  AE_Unit **scan_container = array_new(AE_Unit*, 1);
  AE_Unit *unit = rm_calloc(1, sizeof(AE_Unit));
  unit->src = n;
  scan_container = array_append(scan_container, unit);
  return scan_container;
}

AE_Unit*** AlgebraicExpression_BuildExps(const AST *ast, const QueryGraph *qg, Node **starting_points, int component_count) {
    AE_Unit ***components  = rm_malloc(component_count * sizeof(AE_Unit**));
    // Build triemap of entities that must be populated in the record
    TrieMap *referred_entities = _referred_entities(ast, qg);

    for (int i = 0; i < component_count; i ++) {
      AE_Unit **mapped_component;
      Node *src = starting_points[i];
      if (Vector_Size(src->incoming_edges) == 0 && Vector_Size(src->outgoing_edges) == 0) {
        // Handle single disconnected entity by creating a scan
        mapped_component = _AE_ScanOp(src);
      } else {
        // Build 1 or more units to contain all necessary expressions
        mapped_component = AlgebraicExpression_FromComponent(ast, referred_entities, src);
      }
      components[i] = mapped_component;
    }

    // Debug print
    for (int i = 0; i < component_count; i ++) {
      printf("component %d\n", i);
      AE_Unit **component = components[i];
      for (int j = 0; j < array_len(component); j ++) {
        printf("expression %d\n", j);
        if (component[j] == NULL) continue; // TODO refactor to get rid of trailing NULLs or not create them
        AlgebraicExpression_PrintTree(component[j]->exp_root);
      }
    }

    return components;
}

AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperationNode(AL_EXP_OP op) {
    AlgebraicExpressionNode *node = rm_calloc(1, sizeof(AlgebraicExpressionNode));
    node->type = AL_OPERATION;
    node->operation.op = op;
    node->operation.reusable = false;
    node->operation.v = NULL;
    node->operation.l = NULL;
    node->operation.r = NULL;
    return node;
}

AlgebraicExpressionNode *AlgebraicExpressionNode_NewOperandNode(void *operand, bool is_node) {
    AlgebraicExpressionNode *node = rm_calloc(1, sizeof(AlgebraicExpressionNode));
    node->type = AL_OPERAND;
    node->operand.entity = operand;
    if (is_node) {
      node->operand.entity_is_node = true;
      node->operand.mat = ((Node*)operand)->mat;
    } else {
      node->operand.entity_is_node = false;
      node->operand.mat = ((Edge*)operand)->mat;
    }
    return node;
}

void AlgebraicExpressionNode_AppendLeftChild(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child) {
    assert(root && root->type == AL_OPERATION && root->operation.l == NULL);
    root->operation.l = child;
}

void AlgebraicExpressionNode_AppendRightChild(AlgebraicExpressionNode *root, AlgebraicExpressionNode *child) {
    assert(root && root->type == AL_OPERATION && root->operation.r == NULL);
    root->operation.r = child;
}

// restructure tree
//              (*)
//      (*)               (+)
// (a)       (b)    (e0)       (e1)

// To
//               (+)
//       (*)                (*)  
// (ab)       (e0)    (ab)       (e1)

// Whenever we encounter a multiplication operation
// where one child is an addition operation and the other child
// is a multiplication operation, we'll replace root multiplication
// operation with an addition operation with two multiplication operations
// one for each child of the original addition operation, as can be seen above.
// we'll want to reuse the left handside of the multiplication.
void AlgebraicExpression_SumOfMul(AlgebraicExpressionNode **root) {
    if((*root)->type == AL_OPERATION && (*root)->operation.op == AL_EXP_MUL) {
        AlgebraicExpressionNode *l = (*root)->operation.l;
        AlgebraicExpressionNode *r = (*root)->operation.r;

        if((l->type == AL_OPERATION && l->operation.op == AL_EXP_ADD &&
            !(r->type == AL_OPERATION && r->operation.op == AL_EXP_ADD)) ||
            (r->type == AL_OPERATION && r->operation.op == AL_EXP_ADD &&
            !(l->type == AL_OPERATION && l->operation.op == AL_EXP_ADD))) {
            
            AlgebraicExpressionNode *add = AlgebraicExpressionNode_NewOperationNode(AL_EXP_ADD);
            AlgebraicExpressionNode *lMul = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);
            AlgebraicExpressionNode *rMul = AlgebraicExpressionNode_NewOperationNode(AL_EXP_MUL);

            AlgebraicExpressionNode_AppendLeftChild(add, lMul);
            AlgebraicExpressionNode_AppendRightChild(add, rMul);
            
            if(l->operation.op == AL_EXP_ADD) {
                // Lefthand side is addition, righthand side is multiplication.
                AlgebraicExpressionNode_AppendLeftChild(lMul, r);
                AlgebraicExpressionNode_AppendRightChild(lMul, l->operation.l);
                AlgebraicExpressionNode_AppendLeftChild(rMul, r);
                AlgebraicExpressionNode_AppendRightChild(rMul, l->operation.r);

                // Mark r as reusable.
                if(r->type == AL_OPERATION) r->operation.reusable = true;
            } else {
                // Righthand side is addition, lefthand side is multiplication.
                AlgebraicExpressionNode_AppendLeftChild(lMul, l);
                AlgebraicExpressionNode_AppendRightChild(lMul, r->operation.l);
                AlgebraicExpressionNode_AppendLeftChild(rMul, l);
                AlgebraicExpressionNode_AppendRightChild(rMul, r->operation.r);

                // Mark r as reusable.
                if(l->type == AL_OPERATION) l->operation.reusable = true;
            }

            // TODO: free old root.
            *root = add;
            AlgebraicExpression_SumOfMul(root);
        } else {
            AlgebraicExpression_SumOfMul(&l);
            AlgebraicExpression_SumOfMul(&r);
        }
    }
}

// Forward declaration.
static GrB_Matrix _AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res);

static GrB_Matrix _AlgebraicExpression_Eval_ADD(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    // Expression already evaluated.
    if(exp->operation.v != NULL) return exp->operation.v;

    GrB_Index nrows;
    GrB_Index ncols;
    GrB_Matrix r = NULL;
    GrB_Matrix l = NULL;
    GrB_Matrix inter = NULL;
    GrB_Descriptor desc = NULL; // Descriptor used for transposing.
    AlgebraicExpressionNode *rightHand = exp->operation.r;
    AlgebraicExpressionNode *leftHand = exp->operation.l;

    // Determine if left or right expression needs to be transposed.
    if(leftHand && leftHand->type == AL_OPERATION && leftHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
    }
    
    if(rightHand && rightHand->type == AL_OPERATION && rightHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
    }

    // Evaluate right expressions.
    r = _AlgebraicExpression_Eval(exp->operation.r, res);

    // Evaluate left expressions,
    // if lefthandside expression requires a matrix
    // to hold its intermidate value allocate one here.
    if(leftHand->type == AL_OPERATION) {
        GrB_Matrix_nrows(&nrows, r);
        GrB_Matrix_ncols(&ncols, r);
        GrB_Matrix_new(&inter, GrB_BOOL, nrows, ncols);
        l = _AlgebraicExpression_Eval(exp->operation.l, inter);
    } else {
        l = _AlgebraicExpression_Eval(exp->operation.l, NULL);
    }

    // Perform addition.
    assert(GrB_eWiseAdd_Matrix_Semiring(res, NULL, NULL, Rg_structured_bool, l, r, desc) == GrB_SUCCESS);
    if(inter) GrB_Matrix_free(&inter);

    // Store intermidate if expression is marked for reuse.
    // TODO: might want to use inter if available.
    if(exp->operation.reusable) {
        assert(exp->operation.v == NULL);
        GrB_Matrix_dup(&exp->operation.v, res);
    }

    if(desc) GrB_Descriptor_free(&desc);
    return res;
}

static GrB_Matrix _AlgebraicExpression_Eval_MUL(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    // Expression already evaluated.
    if(exp->operation.v != NULL) return exp->operation.v;

    GrB_Descriptor desc = NULL; // Descriptor used for transposing.
    AlgebraicExpressionNode *rightHand = exp->operation.r;
    AlgebraicExpressionNode *leftHand = exp->operation.l;

    // Determine if left or right expression needs to be transposed.
    if(leftHand && leftHand->type == AL_OPERATION && leftHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
    }
    
    if(rightHand && rightHand->type == AL_OPERATION && rightHand->operation.op == AL_EXP_TRANSPOSE) {
        if(!desc) GrB_Descriptor_new(&desc);
        GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);
    }

    // Evaluate right left expressions.
    GrB_Matrix r = _AlgebraicExpression_Eval(exp->operation.r, res);
    GrB_Matrix l = _AlgebraicExpression_Eval(exp->operation.l, res);

    if (r == NULL) {
      GrB_Matrix_dup(&res, l);
      return res;
    }
    // Perform multiplication.
    assert(GrB_mxm(res, NULL, NULL, Rg_structured_bool, l, r, desc) == GrB_SUCCESS);

    // Store intermidate if expression is marked for reuse.
    if(exp->operation.reusable) {
        assert(exp->operation.v == NULL);
        GrB_Matrix_dup(&exp->operation.v, res);
    }

    if(desc) GrB_Descriptor_free(&desc);
    return res;
}

static GrB_Matrix _AlgebraicExpression_Eval_TRANSPOSE(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    // Transpose is an unary operation which gets delayed.
    AlgebraicExpressionNode *rightHand = exp->operation.r;
    AlgebraicExpressionNode *leftHand = exp->operation.l;

    assert( !(leftHand && rightHand) && (leftHand || rightHand) ); // Verify unary.
    if(leftHand) return _AlgebraicExpression_Eval(leftHand, res);
    else return _AlgebraicExpression_Eval(rightHand, res);
}

static GrB_Matrix _AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res) {
    if(exp == NULL) return NULL;
    if(exp->type == AL_OPERAND) return exp->operand.mat;

    // Perform operation.
    switch(exp->operation.op) {
        case AL_EXP_MUL:
            return _AlgebraicExpression_Eval_MUL(exp, res);

        case AL_EXP_ADD:
            return _AlgebraicExpression_Eval_ADD(exp, res);

        case AL_EXP_TRANSPOSE:
            return _AlgebraicExpression_Eval_TRANSPOSE(exp, res);

        default:
            assert(false);
    }    
    return res;
}

void AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix filter, GrB_Matrix res) {
    // If filter matrix is provided, it will be used as the first operand.
    _AlgebraicExpression_Eval(exp, res);
}

static void _AlgebraicExpressionNode_UniqueNodes(AlgebraicExpressionNode *root, AlgebraicExpressionNode ***uniqueNodes) {
    if(!root) return;

    // Have we seen this node before?
    int nodeCount = array_len(*uniqueNodes);
    for(int i = 0; i < nodeCount; i++) if(root == (*uniqueNodes)[i]) return;

    *uniqueNodes = array_append((*uniqueNodes), root);

    if(root->type != AL_OPERATION) return;

    _AlgebraicExpressionNode_UniqueNodes(root->operation.r, uniqueNodes);
    _AlgebraicExpressionNode_UniqueNodes(root->operation.l, uniqueNodes);
}

void AlgebraicExpressionNode_Free(AlgebraicExpressionNode *root) {
    if(!root) return;

    // Delay free for nodes which are referred from multiple points.
    AlgebraicExpressionNode **uniqueNodes = array_new(AlgebraicExpressionNode*, 1);
    _AlgebraicExpressionNode_UniqueNodes(root, &uniqueNodes);

    // Free unique nodes.
    AlgebraicExpressionNode *node;
    int nodesCount = array_len(uniqueNodes);
    for(int i = 0; i < nodesCount; i++) {
        node = array_pop(uniqueNodes);
        if(node->operation.v) GrB_Matrix_free(&node->operation.v);
        rm_free(node);
    }
    array_free(uniqueNodes);
}

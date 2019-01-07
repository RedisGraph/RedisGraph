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


AlgebraicExpression *_AE_MUL(size_t operand_cap) {
    AlgebraicExpression *ae = malloc(sizeof(AlgebraicExpression));
    ae->op = AL_EXP_MUL;
    ae->operand_cap = operand_cap;
    ae->operand_count = 0;
    ae->operands = malloc(sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    ae->edge = NULL;
    ae->edgeLength = NULL;
    return ae;
}

int _intermidate_node(const Node *n) {
    /* ->()<- 
     * <-()->
     * ->()->
     * <-()<- */
    return ((Vector_Size(n->incoming_edges) > 1) ||
            (Vector_Size(n->outgoing_edges) > 1) ||
            ((Vector_Size(n->incoming_edges) > 0) && (Vector_Size(n->outgoing_edges) > 0)));
}

int _referred_entity(char *alias, TrieMap *ref_entities) {
    return TRIEMAP_NOTFOUND != TrieMap_Find(ref_entities, alias, strlen(alias));
}

int _referred_node(const Node *n, TrieMap *ref_entities) {
    return _referred_entity(n->alias, ref_entities);
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

/* Variable length expression must contain only a single operand, the edge being 
 * traversed multiple times, in cases such as (:labelA)-[e*]->(:labelB) both label A and B
 * are applied via a label matrix operand, this function migrates A and B from a
 * variable length expression to other expressions. */
AlgebraicExpression** _AlgebraicExpression_IsolateVariableLenExps(AlgebraicExpression **expressions, size_t *expCount) {
    /* Return value is a new set of expressions, where each variable length expression
      * is guaranteed to have a single operand, as such in the worst case the number of
      * expressions doubles + 1. */
    AlgebraicExpression **res = malloc(sizeof(AlgebraicExpression*) * (*expCount * 2 + 1));
    size_t newExpCount = 0;

    /* Scan through each expression, locate expression which 
     * have a variable length edge in them. */
    for(size_t expIdx = 0; expIdx < *expCount; expIdx++) {
        AlgebraicExpression *exp = expressions[expIdx];
        if(!exp->edgeLength) {
            res[newExpCount++] = exp;
            continue;
        }

        Edge *e = exp->edge;
        Node *src = exp->src_node;
        Node *dest = exp->dest_node;
        
        // A variable length expression with a labeled source node
        // We only care about the source label matrix, when it comes to
        // the first expression, as in the following expressions
        // src is the destination of the previous expression.
        if(src->mat && expIdx == 0) {
            // Remove src node matrix from expression.
            AlgebraicExpressionOperand op;
            AlgebraicExpression_RemoveTerm(exp, 0, &op);

            /* Create a new expression. */
            AlgebraicExpression *newExp = _AE_MUL(1);
            newExp->src_node = exp->src_node;
            newExp->dest_node = exp->src_node;
            AlgebraicExpression_PrependTerm(newExp, op.operand, op.transpose, op.free);
            res[newExpCount++] = newExp;
        }

        res[newExpCount++] = exp;

        // A variable length expression with a labeled destination node.
        if(dest->mat) {
            // Remove dest node matrix from expression.
            AlgebraicExpressionOperand op;
            AlgebraicExpression_RemoveTerm(exp, exp->operand_count-1, &op);

            /* See if dest mat can be prepended to the following expression.
             * If not create a new expression. */            
            if(expIdx < *expCount-1 && !expressions[expIdx+1]->edgeLength) {
                AlgebraicExpression_PrependTerm(expressions[expIdx+1], op.operand, op.transpose, op.free);
            } else {
                AlgebraicExpression *newExp = _AE_MUL(1);
                newExp->src_node = exp->dest_node;
                newExp->dest_node = exp->dest_node;
                AlgebraicExpression_PrependTerm(newExp, op.operand, op.transpose, op.free);
                res[newExpCount++] = newExp;
            }
        }
    }

    *expCount = newExpCount;
    free(expressions);
    return res;
}

/* Break down expression into sub expressions.
 * considering referenced intermidate nodes and edges. */
AlgebraicExpression **_AlgebraicExpression_Intermidate_Expressions(AlgebraicExpression *exp, const AST *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    /* Allocating maximum number of expression possible. */
    AlgebraicExpression **expressions = malloc(sizeof(AlgebraicExpression *) * exp->operand_count);
    int expIdx = 0;     // Sub expression index.
    int operandIdx = 0; // Index to currently inspected operand.
    int transpose;     // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;

    TrieMap *ref_entities = NewTrieMap();
    ReturnClause_ReferredEntities(ast->returnNode, ref_entities);
    CreateClause_ReferredEntities(ast->createNode, ref_entities);
    WhereClause_ReferredEntities(ast->whereNode, ref_entities);
    DeleteClause_ReferredEntities(ast->deleteNode, ref_entities);    
    SetClause_ReferredEntities(ast->setNode, ref_entities);
    _referred_edge_ends(ref_entities, q);
    _referred_variable_length_edges(ref_entities, matchPattern, q);

    AlgebraicExpression *iexp = _AE_MUL(exp->operand_count);
    iexp->src_node = exp->src_node;
    iexp->dest_node = exp->dest_node;
    iexp->operand_count = 0;
    expressions[expIdx++] = iexp;

    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        AST_GraphEntity *match_element;
        Vector_Get(matchPattern, i, &match_element);

        if(match_element->t != N_LINK) continue;

        AST_LinkEntity *edge = (AST_LinkEntity*)match_element;
        transpose = (edge->direction == N_RIGHT_TO_LEFT);
        e = QueryGraph_GetEdgeByAlias(q, edge->ge.alias);

        /* If edge is referenced, set expression edge pointer. */
        if(_referred_entity(edge->ge.alias, ref_entities))
            iexp->edge = e;

        /* If this is a variable length edge, which is not fixed in length
         * remember edge length. */
        if(edge->length && !AST_LinkEntity_FixedLengthEdge(edge)) {
            iexp->edgeLength = edge->length;
            iexp->edge = e;
        }

        dest = e->dest;
        src = e->src;
        if(transpose) {
            dest = e->src;
            src = e->dest;
        }        

        if(operandIdx == 0 && src->mat) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        unsigned int hops = 1;
        /* Expand fixed variable length edge */
        if(edge->length && AST_LinkEntity_FixedLengthEdge(edge)) {
            hops = edge->length->minHops;
        }

        for(int j = 0; j < hops; j++) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        if(dest->mat) {
            iexp->operands[iexp->operand_count++] = exp->operands[operandIdx++];
        }

        /* If intermidate node is referenced, create a new algebraic expression. */
        if(_intermidate_node(dest) && _referred_node(dest, ref_entities)) {
            // Finalize current expression.
            iexp->dest_node = dest;

            /* Create a new algebraic expression. */
            iexp = _AE_MUL(exp->operand_count - operandIdx);
            iexp->operand_count = 0;
            iexp->src_node = expressions[expIdx-1]->dest_node;
            iexp->dest_node = exp->dest_node;
            expressions[expIdx++] = iexp;
        }
    }
    
    *exp_count = expIdx;
    TrieMap_Free(ref_entities, TrieMap_NOP_CB);
    return expressions;
}

static inline void _AlgebraicExpression_Execute_MUL(GrB_Matrix C, GrB_Matrix A, GrB_Matrix B, GrB_Descriptor desc) {
    // Using our own compile-time, user defined semiring see rg_structured_bool.m4
    // A,B,C must be boolean matrices.
    GrB_mxm(
        C,                  // Output
        NULL,               // Mask
        NULL,               // Accumulator
        Rg_structured_bool, // Semiring
        A,                  // First matrix
        B,                  // Second matrix
        desc                // Descriptor        
    );    
}

// Reverse order of operand within expression,
// A*B*C will become C*B*A. 
void _AlgebraicExpression_ReverseOperandOrder(AlgebraicExpression *exp) {
    int right = exp->operand_count-1;
    int left = 0;
    while(right > left) {
        AlgebraicExpressionOperand leftOp = exp->operands[left];
        exp->operands[left] = exp->operands[right];
        exp->operands[right] = leftOp;
        right--;
        left++;
    }
}

void AlgebraicExpression_AppendTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp, bool freeOp) {
    assert(ae);    
    if(ae->operand_count+1 > ae->operand_cap) {
        ae->operand_cap += 4;
        ae->operands = realloc(ae->operands, sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    }

    ae->operands[ae->operand_count].transpose = transposeOp;
    ae->operands[ae->operand_count].free = freeOp;
    ae->operands[ae->operand_count].operand = m;
    ae->operand_count++;
}

void AlgebraicExpression_PrependTerm(AlgebraicExpression *ae, GrB_Matrix m, bool transposeOp, bool freeOp) {
    assert(ae);

    ae->operand_count++;
    if(ae->operand_count+1 > ae->operand_cap) {
        ae->operand_cap += 4;
        ae->operands = realloc(ae->operands, sizeof(AlgebraicExpressionOperand) * ae->operand_cap);
    }

    // TODO: might be optimized with memcpy.
    // Shift operands to the right, making room at the begining.
    for(int i = ae->operand_count-1; i > 0 ; i--) {
        ae->operands[i] = ae->operands[i-1];
    }

    ae->operands[0].transpose = transposeOp;
    ae->operands[0].free = freeOp;
    ae->operands[0].operand = m;
}

int _ExpressionContainsEdge(TrieMap *t, Edge *e) {
  return !TrieMap_Add(t, e->alias, strlen(e->alias), NULL, TrieMap_DONT_CARE_REPLACE);
}

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

AlgebraicExpressionNode* _AddNode(AlgebraicExpressionNode *root, Node *cur, TrieMap *unique_edges) {
  // TODO only need to add node matrix on start and end of expression
  AlgebraicExpressionNode *node_matrix = AlgebraicExpressionNode_NewOperandNode(cur, true);
  root = AlgebraicExpression_Append(root, node_matrix);

  Edge *e;
  // Outgoing edges
  for (int i = 0; i < Vector_Size(cur->outgoing_edges); i ++) {
    Vector_Get(cur->outgoing_edges, i, &e);
    if (_ExpressionContainsEdge(unique_edges, e)) continue;
    // TODO If edge is variable length or covers multiple relation types,
    // might want to handle that here
    AlgebraicExpressionNode *edge_matrix = AlgebraicExpressionNode_NewOperandNode(e, false);
    root = AlgebraicExpression_Append(root, edge_matrix);
    root = _AddNode(root, e->dest, unique_edges);
  }

  return root;
  // Incoming edges (swap src and dest, transpose edge matrix)
  /*
  for (int i = 0; i < Vector_Size(cur->incoming_edges); i ++) {
    Vector_Get(cur->incoming_edges, i, &e);
    if (_ExpressionContainsEdge(unique_edges, e)) continue;
    // Transpose edge matrix for incoming edges
    AlgebraicExpressionNode *trans = AlgebraicExpressionNode_NewOperationNode(AL_EXP_TRANSPOSE);
    AlgebraicExpressionNode *edge_matrix = AlgebraicExpressionNode_NewOperandNode(e, false);
    AlgebraicExpressionNode_AppendLeftChild(trans, edge_matrix);
    root = AlgebraicExpression_Append(root, trans);

    _AddNode(root, e->src, unique_edges);
  }
  */
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

AE_Unit** e_AddNode(TrieMap *referred_entities, const AST *ast, int *visited, AE_Unit **units, AE_Unit *unit, Node *cur) {
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
    units = e_AddNode(referred_entities, ast, visited, units, unit, e->dest);
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
  units = e_AddNode(referred_entities, ast, visited, units, unit, root);

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
    AE_Unit ***components  = array_new(AE_Unit**, component_count);
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
      components = array_append(components, mapped_component);
    }

    // Debug print
    for (int i = 0; i < array_len(components); i ++) {
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

AlgebraicExpression **AlgebraicExpression_From_Query(const AST *ast, Vector *matchPattern, const QueryGraph *q, size_t *exp_count) {
    assert(q->edge_count != 0);

    AlgebraicExpression *exp = _AE_MUL(q->edge_count + q->node_count);
    int transpose; // Indicate if matrix operand needs to be transposed.    
    Node *dest = NULL;
    Node *src = NULL;
    Edge *e = NULL;
    
    // Scan MATCH clause from left to right.
    for(int i = 0; i < Vector_Size(matchPattern); i++) {
        AST_GraphEntity *match_element;
        Vector_Get(matchPattern, i, &match_element);

        if(match_element->t != N_LINK) continue;
        AST_LinkEntity *astEdge = (AST_LinkEntity*)match_element;
        transpose = (astEdge->direction == N_RIGHT_TO_LEFT);
        e = QueryGraph_GetEdgeByAlias(q, astEdge->ge.alias);
        assert(e);

        GrB_Matrix mat = e->mat;
        dest = e->dest;
        src = e->src;

        if(transpose) {
            dest = e->src;
            src = e->dest;
        }

        if(exp->operand_count == 0) {
            exp->src_node = src;
            if(src->mat) AlgebraicExpression_AppendTerm(exp, src->mat, false, false);
        }

        // ()-[:A|:B.]->()
        // Create matrix M, where M = A+B+...
        int labelCount = AST_LinkEntity_LabelCount(astEdge);
        if(labelCount > 1) {
            GraphContext *gc = GraphContext_GetFromLTS();
            Graph *g = gc->g;

            GrB_Matrix m;
            GrB_Matrix_new(&m, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

            for(int i = 0; i < labelCount; i++) {
                char *label = astEdge->labels[i];
                LabelStore *s = GraphContext_GetStore(gc, label, STORE_EDGE);
                if(!s) continue;
                GrB_Matrix l = Graph_GetRelationMatrix(g, s->id);
                GrB_Info info = GrB_eWiseAdd_Matrix_Semiring(m, NULL, NULL, Rg_structured_bool, m, l, NULL);                
            }
            mat = m;
        }

        unsigned int hops = 1;
        /* Expand fixed variable length edge */
        if(astEdge->length && AST_LinkEntity_FixedLengthEdge(astEdge)) {
            hops = astEdge->length->minHops;
        }

        for(int i = 0; i < hops; i++) {
            bool freeMatrix = (labelCount > 1);
            AlgebraicExpression_AppendTerm(exp, mat, transpose, freeMatrix);
        }

        if(dest->mat) AlgebraicExpression_AppendTerm(exp, dest->mat, false, false);
    }

    exp->dest_node = dest;
    AlgebraicExpression **expressions = _AlgebraicExpression_Intermidate_Expressions(exp, ast, matchPattern, q, exp_count);
    expressions = _AlgebraicExpression_IsolateVariableLenExps(expressions, exp_count);
    // AlgebraicExpression_Free(exp);
    free(exp);

    /* Because matrices are column ordered, when multiplying A*B
     * we need to reverse the order: B*A. */
    for(int i = 0; i < *exp_count; i++) _AlgebraicExpression_ReverseOperandOrder(expressions[i]);
    return expressions;
}

void AlgebraicExpression_Execute(AlgebraicExpression *ae, GrB_Matrix res) {
    assert(ae && res);

    size_t operand_count = ae->operand_count;
    assert(operand_count > 1);

    AlgebraicExpressionOperand operands[operand_count];
    memcpy(operands, ae->operands, sizeof(AlgebraicExpressionOperand) * operand_count);

    GrB_Descriptor desc;
    GrB_Descriptor_new(&desc);
    AlgebraicExpressionOperand leftTerm;
    AlgebraicExpressionOperand rightTerm;

    /* Multiply right to left,
     * A*B*C*D,
     * X = C*D
     * Y = B*X
     * Z = A*Y */
    while(operand_count > 1) {
        rightTerm = operands[operand_count-1];
        leftTerm = operands[operand_count-2];

        // Multiply and reduce.
        if(leftTerm.transpose) GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
        if(rightTerm.transpose) GrB_Descriptor_set(desc, GrB_INP1, GrB_TRAN);

        _AlgebraicExpression_Execute_MUL(res, leftTerm.operand, rightTerm.operand, desc);

        // Quick return if C is ZERO, there's no way to make progress.
        GrB_Index nvals = 0;
        GrB_Matrix_nvals(&nvals, res);
        if(nvals == 0) break;

        // Restore descriptor to default.
        GrB_Descriptor_set(desc, GrB_INP0, GxB_DEFAULT);
        GrB_Descriptor_set(desc, GrB_INP1, GxB_DEFAULT);

        // Assign result and update operands count.
        operands[operand_count-2].operand = res;
        operands[operand_count-2].transpose = false;
        operand_count--;        
    }

    GrB_Descriptor_free(&desc);
}

void AlgebraicExpression_RemoveTerm(AlgebraicExpression *ae, int idx, AlgebraicExpressionOperand *operand) {
    assert(idx >= 0 && idx < ae->operand_count);
    if(operand) *operand = ae->operands[idx];

    // Shift left.
    for(int i = idx; i < ae->operand_count-1; i++) {
        ae->operands[i] = ae->operands[i+1];
    }

    ae->operand_count--;
}

void AlgebraicExpression_Transpose(AlgebraicExpression *ae) {
    /* Actually transpose expression:
     * E = A*B*C 
     * Transpose(E) = 
     * = Transpose(A*B*C) = 
     * = CT*BT*AT */
    
    // Switch expression src and dest nodes.
    Node *n = ae->src_node;
    ae->src_node = ae->dest_node;
    ae->dest_node = n;

    _AlgebraicExpression_ReverseOperandOrder(ae);

    for(int i = 0; i < ae->operand_count; i++)
        ae->operands[i].transpose = !ae->operands[i].transpose;
}

void AlgebraicExpression_Free(AlgebraicExpression* ae) {
    for(int i = 0; i < ae->operand_count; i++) {
        if(ae->operands[i].free) {
            GrB_Matrix_free(&ae->operands[i].operand);
        }
    }

    free(ae->operands);
    free(ae);
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

void AlgebraicExpression_Eval(AlgebraicExpressionNode *exp, GrB_Matrix res) {
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

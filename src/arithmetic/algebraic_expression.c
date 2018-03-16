#include "algebraic_expression.h"

void _Algebraic_Expression_Apply_Filter(char *expression, const char *label) {
    /* Applay filter */
    strcat(expression, label);
    strcat(expression, " * ");
}

void _Algebraic_Expression_Multiply_Relation(char *expression, const char *relation) {
    strcat(expression, relation);
    strcat(expression, " * ");
}

/* Construct an algebraic expression from a query graph.*/
void _QueryGraph_To_Algebraic_Expression(const QueryGraph *g) {
    // Node *n;
    // Edge *e;
    // char expression[2048] = {0};
    // Vector *start_nodes = QueryGraph_GetNDegreeNodes(g, 0);

    // if(Vector_Size(start_nodes) == 0) return;

    // Vector_Get(start_nodes, 0, &n);

    // /* Traverse node. */
    // while(1) {
    //     if(n->label)
    //         _Algebraic_Expression_Apply_Filter(expression, n->label);

    //     if(Vector_Size(n->outgoing_edges) == 0)
    //         break;
        
    //     Vector_Get(n->outgoing_edges, 0, &e);            
    //     _Algebraic_Expression_Multiply_Relation(expression, e->relationship);
        
    //     /* Advance. */
    //     n = e->dest;
    // }

    // /* Clear extra multiplication. */
    // expression[strlen(expression) - 3] = '\0';
    // printf("Expression: %s\n", expression);
}

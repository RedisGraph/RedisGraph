#include "./set.h"

AST_SetNode* New_AST_SetNode(Vector *elements) {
	AST_SetNode *set_node = (AST_SetNode*)malloc(sizeof(AST_SetNode));
	set_node->set_elements = elements;
	return set_node;
}

AST_SetElement* New_AST_SetElement(AST_Variable *updated_entity, AST_ArithmeticExpressionNode *exp) {
	AST_SetElement *set_element = malloc(sizeof(AST_SetElement));
	set_element->entity = updated_entity;
	set_element->exp = exp;
	return set_element;
}

void SetClause_ReferredNodes(const AST_SetNode *set_node, TrieMap *referred_nodes) {
    int set_element_count = Vector_Size(set_node->set_elements);

    for(int i = 0; i < set_element_count; i++) {
        AST_SetElement *set_element;
        Vector_Get(set_node->set_elements, i, &set_element);

		TrieMap_Add(referred_nodes, set_element->entity->alias, strlen(set_element->entity->alias), NULL, NULL);
        AR_EXP_GetAllAliases(set_element->exp, referred_nodes);
    }
}

void Free_AST_SetNode(AST_SetNode *setNode) {
    if(!setNode) return;

    AST_SetElement *elem = NULL;
    while(Vector_Pop(setNode->set_elements, &elem)) {
        Free_AST_Variable(elem->entity);
        Free_AST_ArithmeticExpressionNode(elem->exp);
        free(elem);
    }

    Vector_Free(setNode->set_elements);
    free(setNode);
}

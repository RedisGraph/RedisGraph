#include "AST_Node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ItemNode* CreateItemNode(const char* name, const char* var) {
	printf("CreateItemNode name: %s var: %s\n", name, var);
	ItemNode* node = (ItemNode*)malloc(sizeof(ItemNode));

	node->name = (char*)malloc(strlen(name) + 1);
	node->var = (char*)malloc(strlen(var) + 1);

	strcpy(node->name, name);
	strcpy(node->var, var);

	return node;
}

LinkNode* CreateLinkNode(const char* name, const char* var) {
	printf("CreateLinkNode name: %s var: %s\n", name, var);
	LinkNode* node = (LinkNode*)malloc(sizeof(LinkNode));

	node->name = (char*)malloc(strlen(name) + 1);
	node->var = (char*)malloc(strlen(var) + 1);

	strcpy(node->name, name);
	strcpy(node->var, var);

	return node;
}

FilterNode* CreateFilterNode(const char* property, const char* value) {
	printf("CreateFilterNode property: %s value: %s\n", property, value);
	FilterNode* node = (FilterNode*)malloc(sizeof(FilterNode));

	node->property = (char*)malloc(strlen(property) + 1);
	node->value = (char*)malloc(strlen(value) + 1);

	strcpy(node->property, property);
	strcpy(node->value, value);

	return node;
}

RelationshipNode* CreateRelationshipNode(ItemNode* src, LinkNode* relation, ItemNode* dest) {
	printf("CreateRelationshipNode src: %p relation: %p dest: %p \n", src, relation, dest);

	RelationshipNode* relationshipNode = (RelationshipNode*)malloc(sizeof(RelationshipNode));
	relationshipNode->src = src;
	relationshipNode->relation = relation;
	relationshipNode->dest = dest;

	return relationshipNode;
}

void FreeItemNode(ItemNode* itemNode) {
	free(itemNode->name);
	free(itemNode->var);
	free(itemNode);
}

void FreeLinkNode(LinkNode* linkNode) {
	free(linkNode->name);
	free(linkNode->var);
	free(linkNode);
}

void FreeFilterNode(FilterNode* filterNode) {
	free(filterNode->property);
	free(filterNode->value);
	free(filterNode);
}

void FreeRelationshipNode(RelationshipNode* relationshipNode) {
	FreeItemNode(relationshipNode->src);
	FreeItemNode(relationshipNode->dest);
	FreeLinkNode(relationshipNode->relation);
	free(relationshipNode);
}
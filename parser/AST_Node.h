#ifndef ASTNODE_H_
#define ASTNODE_H_

typedef struct {
	char* name;
	char* var;
} ItemNode;

typedef struct {
	char* name;
	char* var;
} LinkNode;

typedef struct {
	char* property;
	char* value;
} FilterNode;

typedef struct {
	ItemNode* src;
	LinkNode* relation;
	ItemNode* dest;
} RelationshipNode;


ItemNode* CreateItemNode(const char* name, const char* var);
LinkNode* CreateLinkNode(const char* name, const char* var);
FilterNode* CreateFilterNode(const char* property, const char* value);
RelationshipNode* CreateRelationshipNode(ItemNode* src, LinkNode* relation, ItemNode* dest);

void FreeItemNode(ItemNode* itemNode);
void FreeLinkNode(LinkNode* linkNode);
void FreeFilterNode(FilterNode* filterNode);

#endif
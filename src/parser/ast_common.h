/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _AST_COMMON_H
#define _AST_COMMON_H

#include <stdbool.h>
#include "../util/vector.h"

typedef enum {
	N_ENTITY,
	N_LINK,
	N_SCALAR,
} AST_GraphEntityType;

typedef enum {
	N_LEFT_TO_RIGHT,
	N_RIGHT_TO_LEFT,
	N_DIR_UNKNOWN,
} AST_LinkDirection;

typedef struct {
	char *alias;			// Alias given to entity.
	char *label;			// Label of entity.
	Vector *properties;		// Array of attributes.
	AST_GraphEntityType t;	// Type of entity.
	bool anonymous;			// Entity isn't referenced.
} AST_GraphEntity;

typedef AST_GraphEntity AST_NodeEntity;

typedef struct {
	char *alias;
	char *property;
} AST_Variable;

typedef struct {
	unsigned int minHops;
	unsigned int maxHops;
} AST_LinkLength;

typedef struct {
	AST_GraphEntity ge;
	AST_LinkDirection direction;
	AST_LinkLength *length;			// NULL If edge is of length 1.
	char **labels;
} AST_LinkEntity;

AST_NodeEntity* New_AST_NodeEntity(char *alias, char *label, Vector *properties);
AST_LinkEntity* New_AST_LinkEntity(char *alias, char **labels, Vector *properties, AST_LinkDirection dir, AST_LinkLength *length);
AST_LinkLength* New_AST_LinkLength(unsigned int minHops, unsigned int maxHops);
AST_Variable* New_AST_Variable(const char *alias, const char *property);
AST_GraphEntity* Clone_AST_GraphEntity(const AST_GraphEntity *src);
bool AST_LinkEntity_FixedLengthEdge(const AST_LinkEntity* edge);
int AST_LinkEntity_LabelCount(const AST_LinkEntity* edge);
void Free_AST_GraphEntity(AST_GraphEntity *entity);
void Free_AST_Variable(AST_Variable *v);

#endif

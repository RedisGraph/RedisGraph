/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _AST_COMMON_H
#define _AST_COMMON_H

#include "../rmutil/vector.h"

typedef enum {
	N_ENTITY,
	N_LINK,
} AST_GraphEntityType;

typedef enum {
	N_LEFT_TO_RIGHT,
	N_RIGHT_TO_LEFT,
	N_DIR_UNKNOWN,
} AST_LinkDirection;

typedef struct {
	char *alias;
	char *label;
	Vector *properties;
	AST_GraphEntityType t;
} AST_GraphEntity;

typedef AST_GraphEntity AST_NodeEntity;

typedef struct {
	char *alias;
	char *property;
} AST_Variable;

typedef struct {
	AST_GraphEntity ge;
	AST_LinkDirection direction;
} AST_LinkEntity;

AST_NodeEntity* New_AST_NodeEntity(char *alias, char *label, Vector *properties);
AST_LinkEntity* New_AST_LinkEntity(char *alias, char *relationship, Vector *properties, AST_LinkDirection dir);
AST_Variable* New_AST_Variable(const char *alias, const char *property);
void Free_AST_GraphEntity(AST_GraphEntity *entity);
void Free_AST_Variable(AST_Variable *v);

#endif

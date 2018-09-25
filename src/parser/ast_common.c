/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./ast_common.h"
#include "../value.h"

AST_Variable* New_AST_Variable(const char* alias, const char* property) {
	AST_Variable *v = (AST_Variable*)calloc(1, sizeof(AST_Variable));

	if(alias != NULL) {
		v->alias = strdup(alias);
	}
	if(property != NULL) {
		v->property = strdup(property);
	}

	return v;
}

AST_LinkEntity* New_AST_LinkEntity(char *alias, char *label, Vector *properties, AST_LinkDirection dir, AST_LinkLength *length) {
	AST_LinkEntity* le = (AST_LinkEntity*)calloc(1, sizeof(AST_LinkEntity));
	le->direction = dir;
	le->length = length;
	le->ge.t = N_LINK;
	le->ge.properties = properties;
	
	if(label != NULL) {
		le->ge.label = strdup(label);
	}
	if(alias != NULL) {
		le->ge.alias = strdup(alias);
	}

	return le;
}

AST_LinkLength* New_AST_LinkLength(unsigned int minHops, unsigned int maxHops) {
	AST_LinkLength *linkLength = malloc(sizeof(AST_LinkLength));
	linkLength->minHops = minHops;
	linkLength->maxHops = maxHops;
	return linkLength;
}

AST_NodeEntity* New_AST_NodeEntity(char *alias, char *label, Vector *properties) {
	AST_NodeEntity* ne = (AST_NodeEntity*)calloc(1, sizeof(AST_NodeEntity));
	ne->t = N_ENTITY;
	ne->properties = properties;
	
	if(alias != NULL) {
		ne->alias = strdup(alias);
	}
	if(label != NULL) {
		ne->label = strdup(label);
	}

	return ne;
}

bool AST_LinkEntity_FixedLengthEdge(AST_LinkEntity* edge) {
	return (!edge->length || edge->length->minHops == edge->length->maxHops);
}

void Free_AST_GraphEntity(AST_GraphEntity *graphEntity) {
	if(graphEntity->label != NULL) {
		free(graphEntity->label);
	}
	if(graphEntity->alias != NULL) {
		free(graphEntity->alias);
	}
	if(graphEntity->properties != NULL) {
		for(int i = 0; i < Vector_Size(graphEntity->properties); i++) {
			SIValue *val;
			Vector_Get(graphEntity->properties, i, &val);
			SIValue_Free(val);
			free(val);
		}
		Vector_Free(graphEntity->properties);
	}
	if(graphEntity->t == N_LINK) {
		AST_LinkEntity *link = (AST_LinkEntity*)graphEntity;
		if(link->length) free(link->length);
	}
	free(graphEntity);
}

void Free_AST_Variable(AST_Variable *v) {
	if(v != NULL) {
		if(v->alias != NULL) {
			free(v->alias);
		}
		if(v->property != NULL) {
			free(v->property);
		}
		free(v);
	}
}

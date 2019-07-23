/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./ast_common.h"
#include "../util/arr.h"
#include "../value.h"

AST_Variable *New_AST_Variable(char *alias, char *property) {
	AST_Variable *v = malloc(sizeof(AST_Variable));
	v->alias = alias;
	v->property = property;

	return v;
}

static void _AST_Clone_BaseEntity(AST_GraphEntity *clone,
								  const AST_GraphEntity *src) {
	clone->alias = src->alias;
	clone->label = src->label;
	clone->t = src->t;
	clone->anonymous = src->anonymous;
	clone->properties = NULL;

	if(src->properties) {
		int prop_count = Vector_Size(src->properties);
		clone->properties = NewVector(SIValue *, prop_count);

		for(int i = 0; i < prop_count; i++) {
			SIValue *prop;
			Vector_Get(src->properties, i, &prop);
			Vector_Push(clone->properties, prop);
		}
	}
}

static AST_LinkEntity *_AST_Clone_LinkEntity(const AST_LinkEntity *src) {
	AST_LinkEntity *clone = (AST_LinkEntity *)calloc(1, sizeof(AST_LinkEntity));
	_AST_Clone_BaseEntity((AST_GraphEntity *)clone, (const AST_GraphEntity *)src);

	clone->direction = src->direction;

	clone->length = NULL;
	if(src->length) {
		clone->length = New_AST_LinkLength(src->length->minHops, src->length->maxHops);
	}

	clone->labels = NULL;
	if(src->labels) {
		uint32_t label_count = array_len(src->labels);
		clone->labels = array_new(char *, label_count);

		for(int i = 0; i < label_count; i++) {
			clone->labels = array_append(clone->labels, src->labels[i]);
		}
	}

	return clone;
}

static AST_NodeEntity *_AST_Clone_NodeEntity(const AST_NodeEntity *src) {
	AST_NodeEntity *clone = (AST_NodeEntity *)calloc(1, sizeof(AST_NodeEntity));
	_AST_Clone_BaseEntity((AST_GraphEntity *)clone, (const AST_GraphEntity *)src);
	return clone;
}

AST_GraphEntity *Clone_AST_GraphEntity(const AST_GraphEntity *src) {
	if(src->t == N_LINK) {
		return (AST_GraphEntity *)_AST_Clone_LinkEntity((AST_LinkEntity *)src);
	} else {
		return (AST_GraphEntity *)_AST_Clone_NodeEntity((AST_NodeEntity *)src);
	}
}

AST_LinkEntity *New_AST_LinkEntity(char *alias, char **labels,
								   Vector *properties, AST_LinkDirection dir, AST_LinkLength *length) {
	AST_LinkEntity *le = (AST_LinkEntity *)calloc(1, sizeof(AST_LinkEntity));
	le->direction = dir;
	le->length = length;
	le->ge.t = N_LINK;
	le->ge.properties = properties;
	le->labels = NULL;

	if(labels) {
		le->ge.label = labels[0];
		le->labels = labels;
	}

	if(alias != NULL) le->ge.alias = alias;

	return le;
}

AST_NodeEntity *New_AST_NodeEntity(char *alias, char *label,
								   Vector *properties) {
	AST_NodeEntity *ne = (AST_NodeEntity *)calloc(1, sizeof(AST_NodeEntity));
	ne->t = N_ENTITY;
	ne->properties = properties;

	ne->alias = alias;
	ne->label = label;

	return ne;
}

AST_LinkLength *New_AST_LinkLength(unsigned int minHops, unsigned int maxHops) {
	AST_LinkLength *linkLength = malloc(sizeof(AST_LinkLength));
	linkLength->minHops = minHops;
	linkLength->maxHops = maxHops;
	return linkLength;
}

bool AST_LinkEntity_FixedLengthEdge(const AST_LinkEntity *edge) {
	return (!edge->length || edge->length->minHops == edge->length->maxHops);
}

int AST_LinkEntity_LabelCount(const AST_LinkEntity *edge) {
	return array_len(edge->labels);
}

void Free_AST_GraphEntity(AST_GraphEntity *graphEntity) {
	if(graphEntity->properties != NULL) {
		SIValue *val;
		while(Vector_Pop(graphEntity->properties, &val)) {
			SIValue_Free(val);
			free(val); // SIValues in property map literals are heap-allocated
		}
		Vector_Free(graphEntity->properties);
	}
	if(graphEntity->alias) free(graphEntity->alias);
	if(graphEntity->label) free(graphEntity->label);

	if(graphEntity->t == N_LINK) {
		AST_LinkEntity *link = (AST_LinkEntity *)graphEntity;
		if(link->length) free(link->length);
		if(link->labels) array_free(link->labels);
	}

	free(graphEntity);
}

void Free_AST_Variable(AST_Variable *v) {
	if(v != NULL) {
		if(v->alias != NULL) free(v->alias);
		if(v->property != NULL) free(v->property);
		free(v);
	}
}

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

AST_LinkEntity* New_AST_LinkEntity(char *alias, char *label, Vector *properties, AST_LinkDirection dir) {
	AST_LinkEntity* le = (AST_LinkEntity*)calloc(1, sizeof(AST_LinkEntity));
	le->direction = dir;
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

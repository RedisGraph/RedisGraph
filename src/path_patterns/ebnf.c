#include "ebnf.h"

#include "../util/arr.h"
#include "../graph/graphcontext.h"
#include "../query_ctx.h"

void EBNFBase_Init(EBNFBase *base, fpEBNFCopy fp, EBNFBase_Type type, fpEBNFFree free) {
    base->type = type;
    base->copy = fp;
    base->custom_free = free;
    base->children = array_new(EBNFBase*, 1);
}

void EBNFBase_Free(EBNFBase *node) {
    for (int i = 0; i < array_len(node->children); ++i) {
        EBNFBase_Free(node->children[i]);
    }
    array_free(node->children);
    node->custom_free(node);
    rm_free(node);
}

void EBNFBase_CustomFreeDefault(EBNFBase *node) {

}

void EBNFBase_AddChild(EBNFBase *root, EBNFBase *child) {
    root->children = array_append(root->children, child);
}

EBNFBase *EBNFAlternative_New() {
    EBNFAlternative *alt = rm_malloc(sizeof(EBNFAlternative));
    EBNFBase_Init(&alt->base, EBNFAlternative_Copy, EBNF_ALT, EBNFBase_CustomFreeDefault);

    return (EBNFBase*) alt;
}

EBNFBase *EBNFSequence_New() {
    EBNFSequence *seq = rm_malloc(sizeof(EBNFSequence));
    EBNFBase_Init(&seq->base, EBNFSequence_Copy, EBNF_SEQ, EBNFBase_CustomFreeDefault);

    return (EBNFBase*) seq;
}

EBNFBase *EBNFGroup_New(enum cypher_rel_direction direction, EBNFNode_Repetition repetition) {
    EBNFGroup *params = rm_malloc(sizeof(EBNFGroup));
    EBNFBase_Init(&params->base, EBNFGroup_Copy, EBNF_GROUP, EBNFBase_CustomFreeDefault);

    params->direction = direction;
    params->repetition = repetition;
    return (EBNFBase*) params;
}

EBNFBase *EBNFEdge_New(const char *label) {
    EBNFEdge *edge = rm_malloc(sizeof(EBNFEdge));
    EBNFBase_Init(&edge->base, EBNFEdge_Copy, EBNF_EDGE, EBNFBase_CustomFreeDefault);

    edge->reltype = label;

    GraphContext *gc = QueryCtx_GetGraphCtx();
    Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_EDGE);
    if (!s) {
        edge->reltype_id = GRAPH_UNKNOWN_RELATION;
    } else {
        edge->reltype_id = s->id;
    }

    return (EBNFBase*) edge;
}

EBNFBase *EBNFNode_New(const char *label) {
    EBNFNode *node = rm_malloc(sizeof(EBNFNode));
    EBNFBase_Init(&node->base, EBNFNode_Copy, EBNF_NODE, EBNFBase_CustomFreeDefault);

    node->label = label;
    return (EBNFBase*) node;
}

void EBNFReference_Free(EBNFBase *base) {
	EBNFReference *ref = (EBNFReference *) base;
	rm_free(ref->name);
}

EBNFBase *EBNFReference_New(const char *name) {
    EBNFReference *ref = rm_malloc(sizeof(EBNFReference));
    EBNFBase_Init(&ref->base, EBNFReference_Copy, EBNF_REF, EBNFReference_Free);

    size_t len = strlen(name);
	ref->name = rm_malloc(sizeof(char) * (len + 1));
	strcpy(ref->name, name);
    return (EBNFBase *) ref;
}

EBNFBase *EBNFAlternative_Copy(EBNFBase *base) {
    return EBNFAlternative_New();
}

EBNFBase *EBNFSequence_Copy(EBNFBase *base) {
    return EBNFSequence_New();
}

EBNFBase *EBNFGroup_Copy(EBNFBase *base) {
    EBNFGroup *params = (EBNFGroup*)base;
    return EBNFGroup_New(params->direction, params->repetition);
}

EBNFBase *EBNFEdge_Copy(EBNFBase *base) {
    EBNFEdge *edge = (EBNFEdge*)base;
    return EBNFEdge_New(edge->reltype);
}

EBNFBase *EBNFNode_Copy(EBNFBase *base) {
    EBNFNode *node = (EBNFNode*)base;
    return EBNFNode_New(node->label);
}

EBNFBase *EBNFReference_Copy(EBNFBase *base) {
    EBNFReference *ref = (EBNFReference*) base;
    return EBNFReference_New(ref->name);
}

EBNFBase *EBNFBase_Clone(EBNFBase *base) {
    EBNFBase *new_base = base->copy(base);
    for (int i = 0; i < array_len(base->children); ++i) {
        EBNFBase *child = base->children[i];
        new_base->children = array_append(new_base->children, EBNFBase_Clone(child));
    }
    return new_base;
}

void _EBNFBase_ToStr(EBNFBase *base, char *buf) {
    switch (base->type) {
        case EBNF_SEQ: {
            if (array_len(base->children)) {
                _EBNFBase_ToStr(base->children[0], buf);
                for (int i = 1; i < array_len(base->children); ++i) {
                    sprintf(buf + strlen(buf), " ");
                    _EBNFBase_ToStr(base->children[i], buf);
                }
            }
            break;
        }
        case EBNF_ALT: {
            if (array_len(base->children)) {
                _EBNFBase_ToStr(base->children[0], buf);
                for (int i = 1; i < array_len(base->children); ++i) {
                    sprintf(buf + strlen(buf), " | ");
                    _EBNFBase_ToStr(base->children[i], buf);
                }
            }
            break;
        }
        case EBNF_GROUP:{
            EBNFGroup *params = (EBNFGroup *)base;
            if (params->direction == CYPHER_REL_INBOUND || params->direction == CYPHER_REL_BIDIRECTIONAL)
                sprintf(buf + strlen(buf), "<[");
            else
                sprintf(buf + strlen(buf), "[");
            _EBNFBase_ToStr(base->children[0], buf);
            if (params->direction == CYPHER_REL_OUTBOUND || params->direction == CYPHER_REL_BIDIRECTIONAL)
                sprintf(buf + strlen(buf), "]>");
            else
                sprintf(buf + strlen(buf), "]");
            break;
        }
        case EBNF_EDGE: {
            EBNFEdge *edge = (EBNFEdge*) base;
            sprintf(buf + strlen(buf), ":%s", edge->reltype);
            break;
        }
        case EBNF_NODE: {
            EBNFNode *edge = (EBNFNode*) base;
            sprintf(buf + strlen(buf), "(:%s)", edge->label);
            break;
        }
        case EBNF_REF: {
            EBNFReference *ref = (EBNFReference*) base;
            sprintf(buf + strlen(buf), "~%s", ref->name);
            break;
        }
    }
}


char *EBNFBase_ToStr(EBNFBase *base) {
    char *buff = rm_calloc(1024, sizeof(char));
    _EBNFBase_ToStr(base, buff);
    return buff;
}





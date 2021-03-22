/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "qg_edge.h"
#include "qg_node.h"
#include "../graph.h"
#include "../../util/arr.h"

QGEdge *QGEdge_NewRelationPattern(QGNode *src, QGNode *dest, const char *relationship, const char *alias) {
	QGEdge *e = rm_malloc(sizeof(QGEdge));
    e->type = QG_RELATION_PATTERN;
    e->alias = alias;
	e->reltypes = array_new(const char *, 1);
	e->reltypeIDs = array_new(int, 1);
	e->src = NULL;
	e->dest = NULL;
	e->minHops = 1;
	e->maxHops = 1;
	e->bidirectional = false;

	return e;
}

QGEdge *QGEdge_NewPathPattern(QGNode *src, QGNode *dest, EBNFBase *pattern) {
    QGEdge *p = rm_malloc(sizeof(QGEdge));
    p->type = QG_PATH_PATTERN;
    p->src = src;
    p->dest = dest;
    p->bidirectional = false;
    p->pattern = pattern;

    return p;
}

QGEdge *QGEdge_Clone(const QGEdge *orig) {
	QGEdge *e = rm_malloc(sizeof(QGEdge));
	memcpy(e, orig, sizeof(QGEdge));

	e->type = orig->type;
	e->src = NULL;
	e->dest = NULL;

	switch(orig->type) {
		case QG_RELATION_PATTERN:
			array_clone(e->reltypes, orig->reltypes);
			array_clone(e->reltypeIDs, orig->reltypeIDs);
			break;
		case QG_PATH_PATTERN:
			e->pattern = EBNFBase_Clone(orig->pattern);
			break;
	}
	return e;
}

bool QGEdge_VariableLength(const QGEdge *e) {
	ASSERT(e);
	return (e->minHops != e->maxHops);
}

void QGEdge_Reverse(QGEdge *e) {
	QGNode *src = e->src;
	QGNode *dest = e->dest;

	QGNode_RemoveOutgoingEdge(src, e);
	QGNode_RemoveIncomingEdge(dest, e);

	// Reconnect nodes with the source and destination reversed.
	e->src = dest;
	e->dest = src;

	QGNode_ConnectNode(e->src, e->dest, e);
}

int QGEdge_ToString(const QGEdge *e, char *buff, int buff_len) {
	ASSERT(e && buff);

	int offset = 0;
    switch (e->type) {
        case QG_RELATION_PATTERN: {
            offset += snprintf(buff + offset, buff_len - offset, "[");

            if (e->alias) offset += snprintf(buff + offset, buff_len - offset, "%s", e->alias);
            uint reltype_count = array_len(e->reltypes);
            for (uint i = 0; i < reltype_count; i++) {
                // Multiple relationship types are printed separated by pipe characters
                if (i > 0) offset += snprintf(buff + offset, buff_len - offset, "|");
                offset += snprintf(buff + offset, buff_len - offset, ":%s", e->reltypes[i]);
            }
            if (e->minHops != 1 || e->maxHops != 1) {
                if (e->maxHops == EDGE_LENGTH_INF)
                    offset += snprintf(buff + offset, buff_len - offset, "*%u..INF", e->minHops);
                else
                    offset += snprintf(buff + offset, buff_len - offset, "*%u..%u", e->minHops, e->maxHops);
            }

            offset += snprintf(buff + offset, buff_len - offset, "]");
            break;
        }
        case QG_PATH_PATTERN: {
            offset += snprintf(buff + offset, buff_len - offset,
                    "/alias=%s, bidir=%d $ %s/", e->alias, e->bidirectional, EBNFBase_ToStr(e->pattern));
            break;
        }
    }
    return offset;
}

void QGEdge_Free(QGEdge *e) {
	if(!e) return;
    switch (e->type) {
        case QG_RELATION_PATTERN:
            array_free(e->reltypes);
            array_free(e->reltypeIDs);
            break;
        case QG_PATH_PATTERN:
            EBNFBase_Free(e->pattern);
            break;
    }
    rm_free(e);
}

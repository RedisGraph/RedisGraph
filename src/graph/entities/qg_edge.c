/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "qg_edge.h"
#include "qg_node.h"
#include "../graph.h"
#include "../../util/arr.h"
#include <assert.h>

QGEdge* QGEdge_New(QGNode *src, QGNode *dest, const char *relationship, const char *alias) {
    QGEdge *e = rm_malloc(sizeof(QGEdge));
    e->reltypes = array_new(const char*, 1);
    e->reltypeIDs = array_new(uint, 1);
    e->minHops = 1;
    e->maxHops = 1;
    e->alias = NULL;

    return e;
}

QGEdge* QGEdge_Clone(const QGEdge *orig) {
    QGEdge *e = rm_malloc(sizeof(QGEdge));
    e->alias = orig->alias;
    e->reltypes = orig->reltypes;
    e->reltypeIDs = orig->reltypeIDs;
    e->minHops = orig->minHops;
    e->maxHops = orig->maxHops;
    e->id = orig->id;
    e->src = NULL;
    e->dest = NULL;

    return e;
}

bool QGEdge_VariableLength(const QGEdge *e) {
	assert(e);
	return (e->minHops != e->maxHops);
}

void QGEdge_Reverse(QGEdge *e) {
	QGNode *t;
	QGNode *src = e->src;
	QGNode *dest = e->dest;

	QGNode_RemoveOutgoingEdge(src, e);
	QGNode_RemoveIncomingEdge(dest, e);

	// Swap and reconnect.
	t = e->src;
	e->src = e->dest;
	e->dest = t;

    // Edge_Reverse(e->e);

	QGNode_ConnectNode(e->src, e->dest, e);
}

int QGEdge_ToString(const QGEdge *e, char *buff, int buff_len) {
    assert(e && buff);

    int offset = 0;
    offset += snprintf(buff + offset, buff_len - offset, "[");

    if(e->alias)
        offset += snprintf(buff + offset, buff_len - offset, "%s", e->alias);
    uint reltype_count = array_len(e->reltypes);
    for (uint i = 0; i < reltype_count; i ++) {
        offset += snprintf(buff + offset, buff_len - offset, ":%s", e->reltypes[i]); // TODO how should this print?
    }
    if(e->minHops !=1 || e->maxHops !=1) {
        if(e->maxHops == EDGE_LENGTH_INF)
            offset += snprintf(buff + offset, buff_len - offset, "*%u..INF", e->minHops);
        else
            offset += snprintf(buff + offset, buff_len - offset, "*%u..%u", e->minHops, e->maxHops);
    }

    offset += snprintf(buff + offset, buff_len - offset, "]");
    return offset;
}

void QGEdge_Free(QGEdge* edge) {
    // TODO
	// if(!edge) return;

	// if(edge->alias != NULL) free(edge->alias);
	// if(edge->relationship != NULL) free(edge->relationship);
	// free(edge);
}

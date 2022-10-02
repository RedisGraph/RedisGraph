/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"
#include "qg_path.h"
#include "../../query_ctx.h"
#include "../../util/arr.h"

QGPath *QGPath_New
(
	const char *alias
) {
	QGPath *n = rm_malloc(sizeof(QGPath));

	n->alias    =  alias;
	n->nodes    =  array_new(QGNode *, 0);
	n->edges    =  array_new(QGEdge *, 0);

	return n;
}

const char *QGPath_Alias
(
	const QGPath *p
) {
	ASSERT(p != NULL);

	return p->alias;
}

inline void QGPath_AddNode
(
	QGPath *p,
	QGNode *n
) {
	ASSERT(p != NULL);
	ASSERT(n != NULL);

	array_append(p->nodes, n);
}

inline void QGPath_AddEdge
(
	QGPath *p,
	QGEdge *e
) {
	ASSERT(p != NULL);
	ASSERT(e != NULL);

	array_append(p->edges, e);
}

QGPath *QGPath_Clone
(
	const QGPath *p
) {
	ASSERT(p != NULL);

	QGPath *clone = rm_malloc(sizeof(QGPath));
	memcpy(clone, p, sizeof(QGPath));

	array_clone_with_cb(clone->nodes, p->nodes, QGNode_Clone);
	array_clone_with_cb(clone->edges, p->edges, QGEdge_Clone);

	return clone;
}

void QGPath_ToString
(
	const QGPath *p,
	sds *buff
) {
	ASSERT(p != NULL);
	ASSERT(buff != NULL);

	*buff = sdscatprintf(*buff, "(");

	*buff = sdscatprintf(*buff, "%s", p->alias);

	*buff = sdscatprintf(*buff, ")");
}

void QGPath_Free
(
	QGPath *p
) {
	if(p == NULL) return;

	array_free(p->nodes);
	array_free(p->edges);

	rm_free(p);
}


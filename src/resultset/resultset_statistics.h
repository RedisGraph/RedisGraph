/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __GRAPH_RESULTSET_STATS_H__
#define __GRAPH_RESULTSET_STATS_H__

#include <stdbool.h>
#define STAT_NOT_SET -1

typedef struct {
	int labels_added;           /* Number of labels added as part of a create query. */
	int nodes_created;          /* Number of nodes created as part of a create query. */
	int properties_set;         /* Number of properties created as part of a create query. */
	int relationships_created;  /* Number of edges created as part of a create query. */
	int nodes_deleted;          /* Number of nodes removed as part of a delete query.*/
	int relationships_deleted;  /* Number of edges removed as part of a delete query.*/
	int indices_created;        /* Number of indices created. */
	int indices_deleted;        /* Number of indices deleted. */
	bool cached;                /* Indication for a cached query execution. */
} ResultSetStatistics;

/* Checks to see if resultset-statistics indicate that a modification was made. */
bool ResultSetStat_IndicateModification(ResultSetStatistics stats);

#endif

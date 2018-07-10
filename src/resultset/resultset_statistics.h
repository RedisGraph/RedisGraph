#ifndef __GRAPH_RESULTSET_STATS_H__
#define __GRAPH_RESULTSET_STATS_H__

#include <stdbool.h>

typedef struct {
    int labels_added;           /* Number of labels added as part of a create query. */
    int nodes_created;          /* Number of nodes created as part of a create query. */
    int properties_set;         /* Number of properties created as part of a create query. */
    int relationships_created;  /* Number of edges created as part of a create query. */
    int nodes_deleted;          /* Number of nodes removed as part of a delete query.*/
    int relationships_deleted;  /* Number of edges removed as part of a delete query.*/
} ResultSetStatistics;

/* Checks to see if resultset-statistics indicate that a modification was made. */
bool ResultSetStat_IndicateModification(ResultSetStatistics stats);

#endif

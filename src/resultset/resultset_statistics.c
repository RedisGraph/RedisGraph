#include "resultset_statistics.h"

bool ResultSetStat_IndicateModification(ResultSetStatistics stats) {
    return (stats.labels_added > 0
            || stats.nodes_created > 0
            || stats.properties_set > 0
            || stats.relationships_created > 0
            || stats.nodes_deleted > 0
            || stats.relationships_deleted > 0);
}

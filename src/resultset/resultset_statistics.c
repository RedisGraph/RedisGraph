/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "resultset_statistics.h"

bool ResultSetStat_IndicateModification(ResultSetStatistics stats) {
    return (stats.labels_added > 0
            || stats.nodes_created > 0
            || stats.properties_set > 0
            || stats.relationships_created > 0
            || stats.nodes_deleted > 0
            || stats.relationships_deleted > 0);
}

/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef BULKINSERT_H
#define BULKINSERT_H

#include "redismodule.h"
#include "./graph/graph.h"

/*
Bulk insert performs fast insertion of large amount of data,
it's an alternative to Cypher's CREATE query, one should prefer using
bulk insert over CREATE queries when constructing a fairly large
(thousands of entities) new graph, one of the down sides of using bulk insert
is the need to prepare the data in the format which follows below,
also for bulk insert to execute fast we support interleaved queries similar to 
Neo's CSV insert.
The upside is an extremely fast data ingestion.

------------------------------------------------------------------------------
Bulk insert format:
------------------------------------------------------------------------------

[Optional] NODES
    {TOTAL_NUMBER_OF_NODES_TO_CREATE}
    {NUMBER_OF_LABELS}
    {LABEL_I} {NUMBER_OF_NODES_LABELED_AS_I} [{NUM_ATTRIBUTES} {attr} ... ]

    {!-- ACTUAL NODES, MUST BE ORDERED ACCORDING TO LABELING ABOVE. --}
    [{KEY1}{VALUE1}...]

    {!-- NONE LABELED NODES. --}
    {NUMBER_OF_ATTRIBUTE} [{KEY1}{VALUE1}...]

[Optional] RELATIONS
    {TOTAL_NUMBER_OF_RELATIONS_TO_CREATE}
    {NUMBER_OF_LABELED_RELATIONS}
    {LABEL_1} {NUMBER_OF_RELATIONS_LABELED_AS}
    {LABEL_2} {NUMBER_OF_RELATIONS_LABELED_AS}
    {!-- TOTAL_NUMBER_OF_RELATIONS_TO_CREATE - SUM(NUMBER_OF_RELATIONS_LABELED_AS) = UN_LABELED_RELATIONS. --}

    {!-- Node IDs referrs to the position on nodes within this command --}
    {SRC_NODE_ID}{DEST_NODE_ID}
    {SRC_NODE_ID}{DEST_NODE_ID}
    {SRC_NODE_ID}{DEST_NODE_ID}
    {SRC_NODE_ID}{DEST_NODE_ID}

------------------------------------------------------------------------------
 Example:
------------------------------------------------------------------------------
NODES                                   // nodes section.
    6                                   // total number of nodes to create.
    2                                   // total number of unique labels.
    Person 2 2 first_name last_name     // number of nodes of type Person + Person schema.
    Country 2 2 name population         // number of nodes of type Country + Country schema.

    // Actual nodes follows

    Roi lipman          // person ID 1.
    Hila lipman         // person ID 2.
    Israel 7000000      // country ID 3.
    Japan 127000000     // country ID 4.
    2 key1 val1 key2 2  // Unknown label ID 5.
    2 foo bar bar baz   // Unknown label ID 6.

RELATIONS               // relations section.
    6                   // total number of relations to create.
    2                   // total number of unique labels.
    visited 2           // number of relations of type visited.
    knows 2             // number of relations of type knows.
    
    // Actual relations follows.
    
    1 3     // node ID 1 connect to node ID 3 with relation visited.
    1 4     // node ID 1 connect to node ID 4 with relation visited.
    1 2     // node ID 1 connect to node ID 2 with relation knows.
    2 1     // node ID 2 connect to node ID 1 with relation knows.
    3 4     // node ID 3 connect to node ID 4 with relation unknow relation.
    4 3     // node ID 4 connect to node ID 3 with relation unknow relation.
*/

/* Parse bulk insert format and inserts new entities */
void Bulk_Insert (
    RedisModuleCtx *ctx,        // Redis module context.
    RedisModuleString **argv,   // Arguments passed to bulk insert command.
    int argc,                   // Number of elements in argv.
    Graph *g,                   // Graph to populate.
    const char *graph_name,     // Name of the graph.
    size_t *nodes,              // [Optional] number of nodes created.
    size_t *edges               // [Optional] number of edges created.
);

#endif

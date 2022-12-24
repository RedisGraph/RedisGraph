/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "commands.h"

// Convert from string representation to an enum.
GRAPH_Commands CommandFromString(const char *cmd_name) {
	if(strcasecmp(cmd_name, "graph.QUERY")    == 0) return CMD_QUERY;
	if(strcasecmp(cmd_name, "graph.RO_QUERY") == 0) return CMD_RO_QUERY;
	if(strcasecmp(cmd_name, "graph.EXPLAIN")  == 0) return CMD_EXPLAIN;
	if(strcasecmp(cmd_name, "graph.PROFILE")  == 0) return CMD_PROFILE;

	// we shouldn't reach this point
	ASSERT(false);
	return CMD_UNKNOWN;
}

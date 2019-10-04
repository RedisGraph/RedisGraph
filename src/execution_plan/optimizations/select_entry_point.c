/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./select_entry_point.h"
#include "../../util/arr.h"
#include "rax.h"

void selectEntryPoint(AlgebraicExpression *ae, const FT_FilterNode *tree) {
	if(ae->operand_count == 1 && ae->src_node == ae->dest_node) return;

	rax *modifies = FilterTree_CollectModified(tree);
	bool destFiltered = false;
	bool srcLabeled = ae->src_node->label != NULL;
	bool destLabeled = ae->dest_node->label != NULL;

	// See if either source or destination nodes are filtered.
	if(raxFind(modifies, (unsigned char *)ae->src_node->alias,
			   strlen(ae->src_node->alias)) != raxNotFound) {
		goto cleanup;
	}

	if(raxFind(modifies, (unsigned char *)ae->dest_node->alias,
			   strlen(ae->dest_node->alias)) != raxNotFound) {
		destFiltered = true;
	}

	/* Prefer filter over label
	 * if no filters are applied prefer labeled entity. */

	/* TODO: when additional statistics are available
	 * do not use label scan if for every node N such that
	 * (N)-[relation]->(T) N is of the same type T, and type of
	 * either source or destination node is T. */
	if(destFiltered) {
		AlgebraicExpression_Transpose(ae);
	} else if(srcLabeled) {
		goto cleanup;
	} else if(destLabeled) {
		AlgebraicExpression_Transpose(ae);
	}

cleanup:
	raxFree(modifies);
}

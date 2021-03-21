/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "filter_tree.h"

bool _isInFilter(const FT_FilterNode *filter);

bool _extractOriginAndRadius(const FT_FilterNode *filter, SIValue *origin,
		SIValue *radius, char **point);

bool _isDistanceFilter(FT_FilterNode *filter);


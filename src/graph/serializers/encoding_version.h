/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#define GRAPHCONTEXT_TYPE_ENCODING_VERSION 7 // Current RDB encoding version

#define DECODER_SUPPORT_MAX_V 7      // Highest RDB version that can be decoded.
#define DECODER_SUPPORT_MIN_V 7      // Lowest version that can be decoded using the latest routine.
#define PREV_DECODER_SUPPORT_MIN_V 4 // Lowest version that has backwards-compatibility decoding routines.

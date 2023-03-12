/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#define GRAPH_ENCODING_VERSION_LATEST 13 // Latest RDB encoding version.
#define GRAPHCONTEXT_TYPE_DECODE_MIN_V 5 // Lowest version that has backwards-compatibility decoding routines for graphcontext type.
#define GRAPHMETA_TYPE_DECODE_MIN_V 7    // Lowest version that has backwards-compatibility decoding routines for graphmeta type.

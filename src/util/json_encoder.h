/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../value.h"

// Prints the input value to buffer encoded as a JSON string.
void JsonEncoder_SIValue(SIValue v, char **buf, size_t *bufferLen, size_t *bytesWritten);


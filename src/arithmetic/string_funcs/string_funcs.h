/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../value.h"

void Register_StringFuncs();

/* returns a string in which all occurrences of a specified string in the original string have been replaced by ANOTHER (specified) string. */
SIValue AR_REPLACE(SIValue *argv, int argc);
/* returns a list of strings resulting from the splitting of the original string around matches of the given delimiter. */
SIValue AR_SPLIT(SIValue *argv, int argc);
/* returns a string concatenation of given values. */
SIValue AR_CONCAT(SIValue *argv, int argc);


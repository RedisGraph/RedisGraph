/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __RESULTSET_HEADER_H__
#define __RESULTSET_HEADER_H__

#include <stdlib.h>

/* A column within the result-set
 * a column can be referred to either by its name or alias */
typedef struct {
    // TODO necessary?
    char* name;
    char* alias;
} Column;

typedef struct {
    size_t columns_len; /* Number of columns in record */
    Column** columns;   /* Vector of Columns, desired elements specified in return clause */
} ResultSetHeader;

#endif

#ifndef __RESULTSET_HEADER_H__
#define __RESULTSET_HEADER_H__

#include <stdlib.h>

/* A column within the result-set
 * a column can be referred to either by its name or alias */
typedef struct {
    char* name;
    char* alias;
    int aggregated; /* 1 if column is aggregated, 0 otherwise. */
} Column;

typedef struct {
    size_t columns_len; /* Number of columns in record */
    Column** columns;   /* Vector of Columns, desired elements specified in return clause */
    size_t orderby_len; /* How many elements are there in orderBys */
    int* orderBys;      /* Array of indices into elements */
} ResultSetHeader;

#endif
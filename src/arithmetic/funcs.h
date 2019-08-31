/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../value.h"

/* Mathematical functions - numeric */
/* returns the summation of given values. */
SIValue AR_ADD(SIValue *argv, int argc);
/* returns the subtracting given values. */
SIValue AR_SUB(SIValue *argv, int argc);
/* returns the multiplication of given values. */
SIValue AR_MUL(SIValue *argv, int argc);
/* returns the division of given values. */
SIValue AR_DIV(SIValue *argv, int argc);
/* returns the absolute value of the given number. */
SIValue AR_ABS(SIValue *argv, int argc);
/* returns the smallest floating point number that is greater than or equal to the given number and equal to a mathematical integer. */
SIValue AR_CEIL(SIValue *argv, int argc);
/* returns the largest floating point number that is less than or equal to the given number and equal to a mathematical integer. */
SIValue AR_FLOOR(SIValue *argv, int argc);
/* returns a random floating point number in the range from 0 to 1; i.e. [0,1]. The numbers returned follow an approximate uniform distribution. */
SIValue AR_RAND(SIValue *argv, int argc);
/* returns the value of the given number rounded to the nearest integer. */
SIValue AR_ROUND(SIValue *argv, int argc);
/* returns the signum of the given number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number. */
SIValue AR_SIGN(SIValue *argv, int argc);

/* String functions */
/* returns a string containing the specified number of leftmost characters of the original string. */
SIValue AR_LEFT(SIValue *argv, int argc);
/* returns the original string with leading whitespace removed. */
SIValue AR_LTRIM(SIValue *argv, int argc);
/* returns a string in which all occurrences of a specified string in the original string have been replaced by ANOTHER (specified) string. */
SIValue AR_REPLACE(SIValue *argv, int argc);
/* returns a string in which the order of all characters in the original string have been reversed. */
SIValue AR_REVERSE(SIValue *argv, int argc);
/* returns a string containing the specified number of rightmost characters of the original string. */
SIValue AR_RIGHT(SIValue *argv, int argc);
/* returns the original string with trailing whitespace removed. */
SIValue AR_RTRIM(SIValue *argv, int argc);
/* returns a list of strings resulting from the splitting of the original string around matches of the given delimiter. */
SIValue AR_SPLIT(SIValue *argv, int argc);
/* returns a substring of the original string, beginning with a 0-based index start and length. */
SIValue AR_SUBSTRING(SIValue *argv, int argc);
/* returns the original string in lowercase. */
SIValue AR_TOLOWER(SIValue *argv, int argc);
/* converts an integer, float or boolean value to a string. */
SIValue AR_TOSTRING(SIValue *argv, int argc);
/* returns the original string in uppercase. */
SIValue AR_TOUPPER(SIValue *argv, int argc);
/* returns the original string with leading and trailing whitespace removed. */
SIValue AR_TRIM(SIValue *argv, int argc);
/* returns a string concatenation of given values. */
SIValue AR_CONCAT(SIValue *argv, int argc);
/* returns true if argv[1] is a substring of argv[0]. */
SIValue AR_CONTAINS(SIValue *argv, int argc);
/* returns true if argv[0] starts with argv[1]. */
SIValue AR_STARTSWITH(SIValue *argv, int argc);
/* returns true if argv[0] ends with argv[1]. */
SIValue AR_ENDSWITH(SIValue *argv, int argc);
/* returns the id of a relationship or node. */
SIValue AR_ID(SIValue *argv, int argc);
/* returns a string representations the label of a node. */
SIValue AR_LABELS(SIValue *argv, int argc);
/* returns a string representation of the type of a relation. */
SIValue AR_TYPE(SIValue *argv, int argc);
/* returns true if the specified property exists in the node, or relationship. */
SIValue AR_EXISTS(SIValue *argv, int argc);

/* Boolean functions */
/* returns true if AND(arg[i]) is true. */
SIValue AR_AND(SIValue *argv, int argc);
/* returns true if OR(arg[i]) is true. */
SIValue AR_OR(SIValue *argv, int argc);
/* returns true if (arg[i] != arg[i+1]) is true. */
SIValue AR_XOR(SIValue *argv, int argc);
/* returns true if !argv[0] */
SIValue AR_NOT(SIValue *argv, int argc);
/* returns true if argv[0] > argv[1] */
SIValue AR_GT(SIValue *argv, int argc);
/* returns true if argv[0] >= argv[1] */
SIValue AR_GE(SIValue *argv, int argc);
/* returns true if argv[0] < argv[1] */
SIValue AR_LT(SIValue *argv, int argc);
/* returns true if argv[0] <= argv[1] */
SIValue AR_LE(SIValue *argv, int argc);
/* returns true if argv[0] = argv[1] */
SIValue AR_EQ(SIValue *argv, int argc);
/* returns true if argv[0] != argv[1] */
SIValue AR_NE(SIValue *argv, int argc);

/* Temporal functions */
/* returns a timestamp - millis from epoch */
SIValue AR_TIMESTAMP(SIValue *argv, int argc);

/* Conditional flow functions */
/* Case When
 * Case Value [When Option i Then Result i] Else Default end */
SIValue AR_CASEWHEN(SIValue *argv, int argc);

/* Node functions */

/* Returns the number of incoming edges for given node. */
SIValue AR_INCOMEDEGREE(SIValue *argv, int argc);
/* Returns the number of outgoing edges for given node. */
SIValue AR_OUTGOINGDEGREE(SIValue *argv, int argc);

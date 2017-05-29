#include <stdio.h>
#include <stdlib.h>
#include "prng.h"
#include "sha1.h"
#include "snowflake.h"

void get_new_id(unsigned char id[33]) {
    long int snowflakeID = snowflake_id();
    sprintf(id, "%ld", snowflakeID);
}

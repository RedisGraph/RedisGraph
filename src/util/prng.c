#include <stdio.h>
#include <stdlib.h>
#include "prng.h"
#include "sha1.h"
#include "snowflake.h"

long int get_new_id() {
    return snowflake_id();
}

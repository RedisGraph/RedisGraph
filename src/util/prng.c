#include <stdio.h>
#include <stdlib.h>
#include "prng.h"
#include "sha1.h"
#include "snowflake.h"

void get_new_id(unsigned char id[33]) {
    long int snowflakeID = snowflake_id();
    sprintf(id, "%ld", snowflakeID);
}

// void get_new_id(unsigned char id[33]) {
//     uuid_t uid;
//     uuid_generate(uid);
//     sprintf(id, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
//         uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7],
//         uid[8], uid[9], uid[10], uid[11], uid[12], uid[13], uid[14], uid[15]);

//     // unsigned char hash[20];

//     // SHA1_CTX ctx;
//     // SHA1Init(&ctx);
//     // SHA1Update(&ctx, uid, sizeof(uid));
//     // SHA1Final(hash, &ctx);
    
//     // for(int i=0; i<20; i++) {
//     //     snprintf(id+(i*2), 40-(i*2), "%02x", hash[i]);
//     // }
// }
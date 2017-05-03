#include <stdio.h>
#include <stdlib.h>
#include <uuid/uuid.h>
#include "prng.h"
#include "sha1.h"

void get_new_id(unsigned char id[40]) {
    uuid_t uid;
    uuid_generate(uid);
    unsigned char hash[20];

    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, uid, sizeof(uid));
    SHA1Final(hash, &ctx);
    
    for(int i=0; i<20; i++) {
        snprintf(id+(i*2), 40-(i*2), "%02x", hash[i]);
    }
}
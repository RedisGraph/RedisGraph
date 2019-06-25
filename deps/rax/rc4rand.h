#ifndef RC4RAND_H
#define RC4RAND_H

#include <stdint.h>
void rc4srand(uint64_t seed);
uint32_t rc4rand(void);
uint64_t rc4rand64(void);

#endif

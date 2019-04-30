/* A simple RC4 based RNG that sucks less than the certain libc rand()
 * implementations, notably the incredibly bad MacOS() implementation.
 *
 * Copyright (c) 2018, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>

/* Initialize the sbox with an initial seed. */
static unsigned char rc4_sbox[256] = {244, 223, 67, 241, 181, 90, 95, 76, 248, 141, 130, 109, 92, 230, 238, 131, 111, 26, 75, 196, 186, 185, 51, 4, 40, 156, 11, 23, 15, 101, 144, 128, 173, 121, 123, 124, 218, 89, 187, 29, 178, 7, 0, 1, 217, 113, 235, 33, 182, 158, 94, 221, 100, 53, 140, 172, 139, 24, 162, 143, 229, 202, 36, 37, 39, 35, 45, 231, 87, 103, 198, 14, 255, 83, 93, 136, 165, 64, 3, 16, 49, 219, 155, 183, 233, 73, 215, 10, 159, 247, 212, 99, 106, 74, 154, 72, 22, 19, 65, 52, 46, 195, 134, 197, 69, 209, 34, 91, 137, 115, 102, 135, 63, 211, 47, 56, 127, 161, 27, 9, 118, 220, 191, 54, 21, 216, 18, 253, 81, 148, 42, 213, 114, 12, 190, 245, 129, 8, 41, 201, 168, 177, 204, 32, 242, 82, 13, 117, 2, 152, 60, 66, 166, 193, 164, 79, 142, 133, 126, 30, 85, 252, 184, 20, 17, 205, 236, 174, 226, 138, 38, 62, 222, 157, 77, 145, 250, 6, 249, 78, 192, 132, 170, 189, 176, 86, 153, 97, 125, 50, 254, 44, 61, 240, 203, 225, 246, 88, 234, 150, 179, 149, 122, 25, 104, 214, 105, 199, 228, 200, 163, 5, 112, 84, 55, 206, 251, 31, 98, 167, 108, 116, 175, 194, 171, 188, 58, 210, 237, 96, 147, 208, 110, 80, 169, 71, 180, 243, 239, 59, 28, 227, 160, 146, 48, 107, 68, 151, 224, 119, 43, 57, 207, 120, 232, 70};

void rc4srand(uint64_t seed) {
    /* To seed the RNG in a reproducible way we use a simple
     * hashing function, together with the swapping idea of the
     * original RC4 key scheduling algorithm. */
    for (int j = 0; j < 255; j++) rc4_sbox[j] = j;

    seed ^= 5381;
    for (int j = 0; j < 256; j++) {
        seed = ((seed << 5) + seed) + j + 1;
        int i = seed & 0xff;

        /* Swap sbox[i] and sbox[j]. */
        unsigned char t = rc4_sbox[j];
        rc4_sbox[j] = rc4_sbox[i];
        rc4_sbox[i] = t;
    }
}

static void rc4(unsigned char *buf, unsigned long len) {
    static unsigned int i = 0, j = 0;
    unsigned int si, sj, x;

    for (x = 0; x < len; x++) {
	i = (i+1) & 0xff;
	si = rc4_sbox[i];
	j = (j + si) & 0xff;
	sj = rc4_sbox[j];
	rc4_sbox[i] = sj;
	rc4_sbox[j] = si;
	*buf++ = rc4_sbox[(si+sj)&0xff];
    }
}

uint32_t rc4rand(void) {
    uint32_t r;
    rc4((unsigned char*)&r,sizeof(r));
    return r;
}

uint64_t rc4rand64(void) {
    uint64_t r;
    rc4((unsigned char*)&r,sizeof(r));
    return r;
}

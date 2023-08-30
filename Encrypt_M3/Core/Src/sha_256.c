/*
 * sha_256.c
 *
 *  Created on: Aug 30, 2023
 *      Author: mdaef
 */

#include "sha_256.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SHA256 sha256_init() {
	SHA256 sha;
    sha.m_blocklen = 0;
    sha.m_bitlen = 0;

    sha.m_state[0] = 0x6a09e667;
    sha.m_state[1] = 0xbb67ae85;
    sha.m_state[2] = 0x3c6ef372;
    sha.m_state[3] = 0xa54ff53a;
    sha.m_state[4] = 0x510e527f;
    sha.m_state[5] = 0x9b05688c;
    sha.m_state[6] = 0x1f83d9ab;
    sha.m_state[7] = 0x5be0cd19;

    return sha;
}

void sha256_update(SHA256 *sha, const uint8_t *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        sha->m_data[sha->m_blocklen++] = data[i];
        if (sha->m_blocklen == 64) {
            sha256_transform(sha);

            // End of the block
            sha->m_bitlen += 512;
            sha->m_blocklen = 0;
        }
    }
}

void sha256_update_string(SHA256 *sha,const char *data) {
    sha256_update(sha, (const uint8_t *)data, strlen(data));
}


uint8_t *sha256_digest(SHA256 *sha) {
    uint8_t *hash = (uint8_t *)malloc(32);

    sha256_pad(sha);
    sha256_revert(sha, hash);

    return hash;
}

uint32_t sha256_rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

uint32_t sha256_choose(uint32_t e, uint32_t f, uint32_t g) {
    return (e & f) ^ (~e & g);
}

uint32_t sha256_majority(uint32_t a, uint32_t b, uint32_t c) {
    return (a & (b | c)) | (b & c);
}

uint32_t sha256_sig0(uint32_t x) {
    return sha256_rotr(x, 7) ^ sha256_rotr(x, 18) ^ (x >> 3);
}

uint32_t sha256_sig1(uint32_t x) {
    return sha256_rotr(x, 17) ^ sha256_rotr(x, 19) ^ (x >> 10);
}


void sha256_transform(SHA256 *sha) {
    uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
    uint32_t state[8];

    for (uint8_t i = 0, j = 0; i < 16; i++, j += 4) {
        m[i] = (sha->m_data[j] << 24) | (sha->m_data[j + 1] << 16) | (sha->m_data[j + 2] << 8) | (sha->m_data[j + 3]);
    }

    for (uint8_t k = 16; k < 64; k++) {
        m[k] = sha256_sig1(m[k - 2]) + m[k - 7] + sha256_sig0(m[k - 15]) + m[k - 16];
    }

    for (uint8_t i = 0; i < 8; i++) {
        state[i] = sha->m_state[i];
    }

    for (uint8_t i = 0; i < 64; i++) {
        maj = sha256_majority(state[0], state[1], state[2]);
        xorA = sha256_rotr(state[0], 2) ^ sha256_rotr(state[0], 13) ^ sha256_rotr(state[0], 22);

        ch = sha256_choose(state[4], state[5], state[6]);

        xorE = sha256_rotr(state[4], 6) ^ sha256_rotr(state[4], 11) ^ sha256_rotr(state[4], 25);

        sum = m[i] + K.value[i] + state[7] + ch + xorE;
        newA = xorA + maj + sum;
        newE = state[3] + sum;

        state[7] = state[6];
        state[6] = state[5];
        state[5] = state[4];
        state[4] = newE;
        state[3] = state[2];
        state[2] = state[1];
        state[1] = state[0];
        state[0] = newA;
    }

    for (uint8_t i = 0; i < 8; i++) {
        sha->m_state[i] += state[i];
    }
}


void sha256_pad(SHA256 *sha) {
    uint64_t i = sha->m_blocklen;
    uint8_t end = sha->m_blocklen < 56 ? 56 : 64;

    sha->m_data[i++] = 0x80; // Append a bit 1
    while (i < end) {
        sha->m_data[i++] = 0x00; // Pad with zeros
    }

    if (sha->m_blocklen >= 56) {
        sha256_transform(sha);
        memset(sha->m_data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    sha->m_bitlen += sha->m_blocklen * 8;
    sha->m_data[63] = sha->m_bitlen;
    sha->m_data[62] = sha->m_bitlen >> 8;
    sha->m_data[61] = sha->m_bitlen >> 16;
    sha->m_data[60] = sha->m_bitlen >> 24;
    sha->m_data[59] = sha->m_bitlen >> 32;
    sha->m_data[58] = sha->m_bitlen >> 40;
    sha->m_data[57] = sha->m_bitlen >> 48;
    sha->m_data[56] = sha->m_bitlen >> 56;
    sha256_transform(sha);
}


void sha256_revert(SHA256 *sha, uint8_t *hash) {
    // SHA uses big endian byte ordering
    // Revert all bytes
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            hash[i + (j * 4)] = (sha->m_state[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}


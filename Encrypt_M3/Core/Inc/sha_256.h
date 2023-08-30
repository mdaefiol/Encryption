/*
 * sha_256.h
 *
 *  Created on: Aug 30, 2023
 *      Author: mdaef
 */

#ifndef INC_SHA_256_H_
#define INC_SHA_256_H_

#include <stdint.h>
#include <stddef.h>
#include "stm32f1xx_hal.h"

typedef struct {
    uint8_t m_data[64];
    uint32_t m_blocklen;
    uint64_t m_bitlen;
    uint32_t m_state[8];
} SHA256;

SHA256 sha256_init();
void sha256_update(SHA256 *sha, const uint8_t *data, size_t length);
uint8_t *sha256_digest(SHA256 *sha);

uint32_t sha256_rotr(uint32_t x, uint32_t n);
uint32_t sha256_choose(uint32_t e, uint32_t f, uint32_t g);
uint32_t sha256_majority(uint32_t a, uint32_t b, uint32_t c);
uint32_t sha256_sig0(uint32_t x);
uint32_t sha256_sig1(uint32_t x);

void sha256_transform(SHA256 *sha);
void sha256_pad(SHA256 *sha);
void sha256_revert(SHA256 *sha, uint8_t *hash);

typedef struct {
    uint32_t value[64];
} sha256_constants;

extern const sha256_constants K;


#endif /* INC_SHA_256_H_ */

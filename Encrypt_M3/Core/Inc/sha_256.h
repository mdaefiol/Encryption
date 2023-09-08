/*
 * sha_256.h
 *
 *  Created on: Aug 30, 2023
 *      Author: mdaef
 */

#ifndef INC_SHA_256_H_
#define INC_SHA_256_H_

#include "stm32f1xx_hal.h"

// Define a estrutura para armazenar o estado do SHA-256
typedef struct {
    uint32_t m_state[8]; // A B C D E F G H
    uint8_t m_data[64];
    uint64_t m_bitlen;
    uint32_t m_blocklen;
} SHA256;

// Inicializa o contexto SHA-256
void sha256_init(SHA256 *ctx);

extern const uint32_t K[64];

void sha256_update(SHA256 *ctx, const uint8_t *data, uint8_t length) ;
void sha256_update_string(SHA256 *ctx, const char *data) ;
uint8_t *sha256_digest(SHA256 *ctx) ;

uint32_t sha256_rotr(uint32_t x, uint32_t n);
uint32_t sha256_choose(uint32_t e, uint32_t f, uint32_t g);
uint32_t sha256_majority(uint32_t a, uint32_t b, uint32_t c) ;
uint32_t sha256_sig0(uint32_t x) ;
uint32_t sha256_sig1(uint32_t x);
void sha256_transform(SHA256 *ctx);

void sha256_pad(SHA256 *ctx);
void sha256_revert( SHA256 *ctx, uint8_t *hash) ;
//void sha256_toString(const uint8_t *digest, char *hashString) ;

void NonceSHA256Hash(uint8_t *data, uint8_t *NumIn, uint8_t *aux) ;
void GenDigSHA256Hash(uint8_t *slot , uint8_t *data, uint8_t *aux) ;
void MACSHA256Hash(uint8_t *slot ,uint8_t *data, uint8_t *aux);
void calculateSHA256Hash(uint8_t *data, uint8_t dataSize, uint8_t *aux) ;

#endif /* INC_SHA_256_H_ */

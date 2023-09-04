/*
 * sha_256.c
 *
 *  Created on: Aug 30, 2023
 *      Author: mdaef
 */
#include "sha_256.h"
#include "ATSHA204.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t general_conf[]	=  {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 104 PAD
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00 };// 128 LENGTH;

uint8_t slot2[] =		{0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24,
				     	 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29};

uint8_t slot4[] =		{0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44,
					 	 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49};

const uint32_t K[64] = {
		0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
		0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
		0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
		0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
		0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
		0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
		0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
		0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
		0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
		0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
		0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
		0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
		0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
		0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
		0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
		0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

SHA256 ctx;

// Implementação do construtor SHA256::SHA256()
void sha256_init(SHA256 *ctx) {
    // Inicialize m_blocklen e m_bitlen
    ctx->m_blocklen = 0;
    ctx->m_bitlen = 0;

    // Inicialize o estado do hash
    ctx->m_state[0] = 0x6a09e667;
    ctx->m_state[1] = 0xbb67ae85;
    ctx->m_state[2] = 0x3c6ef372;
    ctx->m_state[3] = 0xa54ff53a;
    ctx->m_state[4] = 0x510e527f;
    ctx->m_state[5] = 0x9b05688c;
    ctx->m_state[6] = 0x1f83d9ab;
    ctx->m_state[7] = 0x5be0cd19;
}

void sha256_update(SHA256 *ctx, const uint8_t *data, uint8_t length) {
    for (size_t i = 0; i < length; i++) {
        ctx->m_data[ctx->m_blocklen++] = data[i];
        if (ctx->m_blocklen == 64) {
        	sha256_transform(ctx);


            // Fim do bloco
            ctx->m_bitlen += 512;
            ctx->m_blocklen = 0;
        }
    }
}

void sha256_update_string(SHA256 *ctx, const char *data) {
    size_t length = strlen(data);
    sha256_update(ctx, (const uint8_t *)data, length);
}


uint8_t *sha256_digest(SHA256 *ctx) {
    uint8_t *hash = (uint8_t *)malloc(32 * sizeof(uint8_t));
    if (hash == NULL) {
        // Tratamento de erro, não foi possível alocar memória
        return NULL;
    }

    //sha256_pad(ctx);
    sha256_revert(ctx, hash);

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

void sha256_transform(SHA256 *ctx) {
    uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
    uint32_t state[8];

    for (uint8_t i = 0, j = 0; i < 16; i++, j += 4) {
        m[i] = (ctx->m_data[j] << 24) | (ctx->m_data[j + 1] << 16) | (ctx->m_data[j + 2] << 8) | (ctx->m_data[j + 3]);
    }

    for (uint8_t k = 16; k < 64; k++) {
        m[k] = sha256_sig1(m[k - 2]) + m[k - 7] + sha256_sig0(m[k - 15]) + m[k - 16];
    }

    for (uint8_t i = 0; i < 8; i++) {
        state[i] = ctx->m_state[i];
    }

    for (uint8_t i = 0; i < 64; i++) {
        maj = sha256_majority(state[0], state[1], state[2]);
        xorA = sha256_rotr(state[0], 2) ^ sha256_rotr(state[0], 13) ^ sha256_rotr(state[0], 22);

        ch = sha256_choose(state[4], state[5], state[6]);

        xorE = sha256_rotr(state[4], 6) ^ sha256_rotr(state[4], 11) ^ sha256_rotr(state[4], 25);

        sum = m[i] + K[i] + state[7] + ch + xorE;
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
        ctx->m_state[i] += state[i];
    }
}

void sha256_pad(SHA256 *ctx) {
    uint64_t i = ctx->m_blocklen;
    uint8_t end = (ctx->m_blocklen < 56) ? 56 : 64;

    ctx->m_data[i++] = 0x80; // Anexar um bit 1
    while (i < end) {
        ctx->m_data[i++] = 0x00; // Preenche com zeros
    }

    if (ctx->m_blocklen >= 56) {
        sha256_transform(ctx);
        memset(ctx->m_data, 0, 56);
    }

    ctx->m_bitlen += ctx->m_blocklen * 8;
    ctx->m_data[63] = (uint8_t)(ctx->m_bitlen);
    ctx->m_data[62] = (uint8_t)(ctx->m_bitlen >> 8);
    ctx->m_data[61] = (uint8_t)(ctx->m_bitlen >> 16);
    ctx->m_data[60] = (uint8_t)(ctx->m_bitlen >> 24);
    ctx->m_data[59] = (uint8_t)(ctx->m_bitlen >> 32);
    ctx->m_data[58] = (uint8_t)(ctx->m_bitlen >> 40);
    ctx->m_data[57] = (uint8_t)(ctx->m_bitlen >> 48);
    ctx->m_data[56] = (uint8_t)(ctx->m_bitlen >> 56);
    sha256_transform(ctx);
}

void sha256_revert( SHA256 *ctx, uint8_t*hash) {
    // O SHA usa ordenação de bytes big-endian
    // Reverter todos os bytes
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            hash[i + (j * 4)] = (ctx->m_state[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}
/*
void sha256_toString(const uint8_t *digest, char *hashString) {
    const char hexChars[] = "0123456789abcdef";

    for (int i = 0; i < 32; i++) {
        hashString[i * 2] = hexChars[(digest[i] >> 4) & 0xF];
        hashString[i * 2 + 1] = hexChars[digest[i] & 0xF];
    }
    hashString[64] = '\0';
}

void GenDigSHA256Hash(uint8_t *data) {
    uint8_t gendig[128];
    uint8_t config_gendig[] = {0x15, 0x02, 0x02, 0x00, 0xEE, 0x01, 0x23, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    for (int i = 0; i < 32; i++) {
        gendig[i] = 0x20 + i;
    }

    for (int i = 0; i < 32; i++) {
        gendig[32 + i] = config_gendig[i];
        gendig[64 + i] = data[i];
        gendig[96 + i] = general_conf[i];
    }
}
*/

void GenDigSHA256Hash(uint8_t *data, uint8_t *aux) {

    SHA256 sha;
	uint8_t gendig[128];
    uint8_t config_gendig[] = {0x15, 0x02, 0x02, 0x00, 0xEE, 0x01, 0x23, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(gendig, slot2, 32);
    memcpy(gendig + 32, config_gendig, 32);
    memcpy(gendig + 64, data, 32);
    memcpy(gendig + 96, general_conf, 32);

    sha256_init(&sha);
    sha256_update(&sha, gendig, sizeof(gendig));
    uint8_t *digest = sha256_digest(&sha);

    for (int i = 0; i<128;i++){
    	aux[i] = digest[i];
    }

    free(digest);
}


void MACSHA256Hash(uint8_t *data, uint8_t *aux){

	SHA256 sha;
	uint8_t mac[128];
	uint8_t config_mac[] = {0x08, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE,
							0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0x00, 0x00,
							0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t mac_final[] = {	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			  	  	  	  	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xC0};
    memcpy(mac, slot4, 32);
    memcpy(mac + 32, data, 32);
    memcpy(mac + 64, config_mac, 32);
    memcpy(mac + 96, mac_final, 32);

    sha256_init(&sha);
    sha256_update(&sha, mac, sizeof(mac));
    uint8_t *digest = sha256_digest(&sha);

    for (int i = 0; i<128;i++){
    	aux[i] = digest[i];
    }

    free(digest);
}


void calculateSHA256Hash(uint8_t *data, uint8_t dataSize, uint8_t *aux) {
    SHA256 sha;

    sha256_init(&sha);
    sha256_update(&sha, data, dataSize);
    uint8_t *digest = sha256_digest(&sha);

    for (int i =0; i<64;i++){
    	aux[i] = digest[i];
    }
    //sha256_toString(digest, hashString);

    // Liberar a memória alocada para o digest
    free(digest);
}


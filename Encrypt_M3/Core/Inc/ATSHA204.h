/*
 * ATSHA204.h
 *
 *  Created on: 24 de abr de 2023
 *      Author: oriongames
 */

#ifndef INC_ATSHA204_H_
#define INC_ATSHA204_H_

#include "stm32f1xx_hal.h"

// Geral
#define I2C_ADDRESS 			0xC8 	// Endereço I2C do ATSHA204A
#define SERIAL_NUMBER_SIZE 		9 		// Tamanho do número de série do ATSHA204A
#define KEY_SIZE 				32 		// Tamanho da chave criptográfica gerada
#define ZONE_ACCESS_CONFIG_SIZE 4 		// Tamanho da zona de configuração
#define PUBLIC_KEY_SIZE 		64 		// Tamanho da chave pública gerada

// Geral
#define COMMAND 				0x03
#define SIZE_WRITE_DATA			0x27
#define SIZE_WRITE_OTP			0x0A
#define SIZE_WRITE_CONFIG		0x0B
#define SIZE_BLOCK_CONFIG		0x07
#define ZONE_CONFIG_LOCK 		0x00
#define ZONE_DATA_LOCK 			0x81
#define ZONE_DATA_READ         	0x82

// Definir os códigos de comando do ATSHA204A
#define COMMAND_CHECKMAC        0x28
#define COMMAND_DERIVE_KEY      0x1C
#define COMMAND_INFO            0x30
#define COMMAND_GENKEY          0x40
#define COMMAND_GENDIG          0x15
#define COMMAND_LOCK            0x17
#define COMMAND_MAC             0x08
#define COMMAND_NONCE           0x16
#define COMMAND_PAUSE           0x01
#define COMMAND_RANDOM          0x1B
#define COMMAND_READ            0x02
#define COMMAND_SHA             0x47
#define COMMAND_UPDATE_EXTRA    0x20
#define COMMAND_WRITE           0x12

// Definir os códigos de zonas do ATSHA204A
#define ZONE_CONFIG 0x00 			// Zona de configuração
#define ZONE_DATA 	0x02 			// Zona de dados

// Definir as configurações de slot do ATSHA204A
#define SLOT_CONFIG_SECRET      	0x03 // Configuração de slot para chave secreta
#define SLOT_CONFIG_WRITE_ONLY  	0x06 // Configuração de slot para escrita somente

// Definir as configurações de bloqueio de zona do ATSHA204A
#define ZONE_LOCK_CONFIG_LOCKED 	0x55 // Zona bloqueada
#define ZONE_LOCK_CONFIG_UNLOCKED 	0x00 // Zona desbloqueada


void WakeUp(uint8_t *data_rec);
void ReadConfig(uint8_t *readCommand, uint16_t size, uint8_t *data_config);
void WriteConfigZone(void);
void BlockConfigZone(uint8_t *receiv_byte);
void WriteDataZone(void);
void WriteOTPZone(void);
void BlockDataZone(void);
void ReadDataZone(uint8_t *readData, uint16_t size, uint8_t *data);
//void sha204c_calculate_crc(uint8_t length, uint8_t *data, uint8_t *crc) ;

#endif /* INC_ATSHA204_H_ */

/*
 * ATSHA204.c
 *
 *  Created on: 25 de abr de 2023
 *      Author: mdaef
 */

/*
 * ATSHA204.c
 *
 *  Created on: 24 de abr de 2023
 *      Author: oriongames
 */

#include "ATSHA204.h"

extern I2C_HandleTypeDef hi2c2;

// command packet: 8.5.1
// 6.2.1: Word Address Value: COMMAND == 0x03
// read command: 8.5.15
// read configuration zone: {COMMAND, COUNT, OPCODE, ZONE, ADDRESS_1, ADDRESS_2, CRC_LSB, CRC_MSB}
// read configuration zone: {  0x03,    0x07, 0x02, 0x00,      0x00,      0x00,      0xB2,    0x7E}
// CRC-16 Polinomial: 0x8005: includes COUNT, OPCODE, ZONE, ADDRESS_1, ADDRESS_2, CRC_LSB, CRC_MSB (does not include COMMAND)
// CRC https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
// Zone encoding table 8-5

uint8_t rec_MasterKey[32];

void WakeUp(void){

	uint8_t data = 0;

	HAL_I2C_Master_Receive(&hi2c2, 0xFE, &data, sizeof(data), 1000); 		// Ver onde fala do 0XFE
	HAL_Delay(5); // 2.5 ms para acordar; 45 ms para entrar em sleep
}

void ReadConfig(uint8_t *readCommand, uint8_t *data_rec ,uint8_t *data_config) {

	WakeUp();
	uint8_t data = 0;

	// first read: 0 byte read - should receive an ACK
	HAL_I2C_Master_Receive(&hi2c2, 0xC8, &data, 1, 1000);
	HAL_Delay(5);

	// Read 88bytes
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, &data, sizeof(data), 1000);		// Envia 1 byte
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, 0xC8, data_rec, 4, 1000); 		    	// Recebe 0x04 0x11 0x33 0x43
	HAL_Delay(5);
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, readCommand, 8, 1000); 		    // Enviar o comando de leitura
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, 0xC8, data_config, 32, 1000); 	        // Recebe(byte de tamanho, 35 em decimal)..0x01 0x23...
	HAL_Delay(5);
}

void WriteConfigZone(void){
	// WritePwd1 + ReadPwd1
	// comando para configuração da zona de configuração do slot: 0x00 e 0x01
	uint8_t configSlot0_1[] = {0x03, 0x0B, 0x12, 0x00, 0x05, 0x00, 0x60, 0xE9, 0x70, 0xE8, 0x53, 0x98};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot0_1, sizeof(configSlot0_1), 1000);
	HAL_Delay(5);

	// DATA0 + DATA1
	// comando para configuração da zona de configuração do slot: 0x02 e 0x03
	uint8_t configSlot2_3[] = {0x03, 0x0B, 0x12, 0x00, 0x06, 0x00,  0x40, 0xE1, 0x80, 0xA1,0xB0, 0x68 };
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot2_3, sizeof(configSlot2_3), 1000);
	HAL_Delay(5);

	// PASSWORD1 + SECRET1
	// comando para configuração da zona de configuração do slot: 0x04 e 0x05
	uint8_t configSlot4_5[] = {0x03, 0x0B, 0x12, 0x00, 0x07, 0x00, 0x75, 0xC1, 0x60, 0xA0,  0x5B, 0x89};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot4_5, sizeof(configSlot4_5), 1000);
	HAL_Delay(5);

	// PASSWORD2 + SECRET2
	// comando para configuração da zona de configuração do slot: 0x06 e 0x07
	uint8_t configSlot6_7[] = {0x03, 0x0B, 0x12, 0x00, 0x08, 0x00,  0x47, 0xD1, 0x40, 0xB0, 0x2A, 0x61};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot6_7, sizeof(configSlot6_7), 1000);
	HAL_Delay(5);

	// ReadPwd2 + WritePwd2
	// comando para configuração da zona de configuração do slot: 0x08 e 0x09
	uint8_t configSlot8_9[] = {0x03, 0x0B, 0x12, 0x00, 0x09, 0x00, 0x49,  0xCE, 0x49,  0xCE, 0xDB, 0x4C};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot8_9, sizeof(configSlot8_9), 1000);
	HAL_Delay(5);

	// DATA[2] + DATA[3]
	// comando para configuração da zona de configuração do slot: 0x10 e 0x11
	uint8_t configSlot10_11[] = {0x03, 0x0B, 0x12, 0x00, 0x0A, 0x00, 0x49, 0xC8, 0x89, 0x88, 0xD8, 0xC0};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot10_11, sizeof(configSlot10_11), 1000);
	HAL_Delay(5);

	// PASSWORD[3] + SECRET[3]
	// comando para configuração da zona de configuração do slot: 0x12 e 0x13
	uint8_t configSlot12_13[] = {0x03, 0x0B, 0x12, 0x00, 0x0B, 0x00, 0x4D, 0xD8,  0x49, 0x90, 0xAC, 0x4A };
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot12_13, sizeof(configSlot12_13), 1000);
	HAL_Delay(5);

	// MASTERKEY + DATA[4]
	// comando para configuração da zona de configuração do slot: 0x14 e 0x15
	uint8_t configSlot14_15[] = {0x03, 0x0B, 0x12, 0x00, 0x0C, 0x00, 0x0E, 0x0E, 0x49, 0x88, 0x2F, 0xE5 };
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, configSlot14_15, sizeof(configSlot14_15), 1000);
	HAL_Delay(5);
}

void BlockConfigZone(void){
	uint8_t blockConfig[] = {0x07, 0x07, 0x17, 0x00, 0x00, 0x00, 0x2e, 0x0d };
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, blockConfig, sizeof(blockConfig), 1000);
	HAL_Delay(5);
}

void WriteDataZone(void){


}


/*
void SerialRead(uint8_t *data_rec1, uint8_t *data_rec2){

	// Leitura serial number
    // SHA204_READ = 0x03

	WakeUp();

    uint8_t data = 0;
    uint8_t readCommand[10] = {0x03, 0x07, 0x02, 0x80, 0x00, 0x00, 0x09, 0xAD};

    HAL_I2C_Master_Transmit(&hi2c2, 0xC8, &data, sizeof(data), 1000);		// Envia 1 byte
    HAL_Delay(5);
    HAL_I2C_Master_Receive(&hi2c2, 0xC8, data_rec1, 4, 1000); 				// Recebe 0x04 0x11 0x33 0x43
    HAL_Delay(5);
    HAL_I2C_Master_Transmit(&hi2c2, 0xC8, readCommand, 8, 1000); 			// Enviar o comando de leitura
    HAL_Delay(5);
    HAL_I2C_Master_Receive(&hi2c2, 0xC8, data_rec2, 32, 1000); 				// Recebe(byte de tamanho, 35 em decimal)..0x01 0x23...
    HAL_Delay(5);
}

void CommandSerial(uint8_t *sData){

	WakeUp();

	uint8_t data = 0;
	HAL_I2C_Master_Receive(&hi2c2, 0xF7, &data, sizeof(data), 1000); 	// Ver onde fala do 0XFE
	HAL_Delay(10); // 2.5 ms para acordar; 45 ms para entrar em sleep

	    // first read: 0 byte read - should receive an ACK
	HAL_I2C_Master_Receive(&hi2c2, 0xC8, &data, 1, 1000);
	uint8_t snCmd[4] = {0x02, 0x00, 0x00, 0x00}; // Comando para leitura do serial number

	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, snCmd, 4, 1000);
    HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, 0xC8, sData, 9, 1000);
    HAL_Delay(5);
    // snData = 0x00, 0xC8, 0x00, 0x55, 0x00

}

*/

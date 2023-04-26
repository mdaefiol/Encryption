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


void WakeUp(void){

    // Wake-up

	uint8_t data = 0;

	HAL_I2C_Master_Receive(&hi2c2, 0xF7, &data, sizeof(data), 1000); 	// Ver onde fala do 0XFE
    HAL_Delay(10); // 2.5 ms para acordar; 45 ms para entrar em sleep

    // first read: 0 byte read - should receive an ACK
    HAL_I2C_Master_Receive(&hi2c2, 0xC8, &data, 1, 1000);
}

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

/*
 * 	    //COMANDO DE BLOQUEIO DE CONFIGURAÇÃO
	    uint8_t blockCommand[10] = {0x03, 0x80, 0x01, 0x23, 0x04, 0x00, 0x00, 0x00};
	    HAL_Delay(5);
	  //  ret5 = HAL_I2C_Master_Transmit(&hi2c2, 0xC8, &data, sizeof(data), 1000); // Tem que enviar 1 byte
	  //  HAL_Delay(5);
	  //  saida3 = HAL_I2C_Master_Receive(&hi2c2, 0xC8, reply, 4, 1000); // tem que receber 0x04 0x11 0x33 0x43
	  //  HAL_Delay(5);
	    uint8_t reply_block[32];
	    ret5 = HAL_I2C_Master_Transmit(&hi2c2, 0xC8, blockCommand, 8, 1000); // enviar o comando de bloqueio de configuração
	    HAL_Delay(5);
	    saida3 = HAL_I2C_Master_Receive(&hi2c2, 0xC8, reply_block, 32, 1000); // tem que receber (byte de tamanho, 35 em decimal) .. 0x01 0x23 ...
	    HAL_Delay(5);

	    uint8_t writedata[8] = {0x03, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55};
	    HAL_I2C_Master_Transmit(&hi2c2, 0xC8, blockCommand, 8, 1000); // enviar o comando de bloqueio de configuração
	   	HAL_Delay(5);
	    HAL_I2C_Master_Receive(&hi2c2, 0xC8, reply_block, 32, 1000); // tem que receber (byte de tamanho, 35 em decimal) .. 0x01 0x23 ...
	   	HAL_Delay(5);


void sha204c_calculate_crc(uint8_t length, uint8_t *data, uint8_t *crc) {
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
	  for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
		 data_bit = (data[counter] & shift_register) ? 1 : 0;
		 crc_bit = crc_register >> 15;
		 crc_register <<= 1;
		 if (data_bit != crc_bit)
			crc_register ^= polynom;
	  }
	}
	crc[0] = (uint8_t) (crc_register & 0x00FF);
	crc[1] = (uint8_t) (crc_register >> 8);
}
*/


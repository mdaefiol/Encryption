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

uint8_t WritePwd1[] =	{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04,
					 	 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
uint8_t ReadPwd1[] =	{0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x10, 0x11, 0x12, 0x13, 0x14,
					 	 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
uint8_t DATA0[] =		{0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24,
				     	 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29};
uint8_t DATA1[] =		{0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x30, 0x31, 0x32, 0x33, 0x34,
					 	 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
uint8_t PASSWORD1[] =	{0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44,
					 	 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49};
uint8_t SECRET1[] =		{0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x50, 0x51, 0x52, 0x53, 0x54,
						 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59};
uint8_t PASSWORD2[] =	{0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x60, 0x61, 0x62, 0x63, 0x64,
					  	 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69};
uint8_t SECRET2[] =		{0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x70, 0x71, 0x72, 0x73, 0x74,
						 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79};
uint8_t ReadPwd2[] = 	{0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x80, 0x81, 0x82, 0x83, 0x84,
		 	 	 	 	 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89};
uint8_t WritePwd2[] = 	{0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x90, 0x91, 0x92, 0x93, 0x94,
						 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99};
uint8_t DATA2[] = 		{0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
						 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9};
uint8_t DATA3[] = 		{0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
						 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9};
uint8_t PASSWORD3[] =	{0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4,
						 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9};
uint8_t SECRET3[] =		{0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
						 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9};
uint8_t MASTERKEY[] =	{0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
						 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9};
uint8_t DATA4[] = 		{0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
						 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9};


void WakeUp(uint8_t *data_rec){

	uint8_t data = 0;

	HAL_I2C_Master_Receive(&hi2c2, 0xFE, &data, sizeof(data), 1000);
	HAL_Delay(5); // 2.5 ms para acordar; 45 ms para entrar em sleep
	// first read: 0 byte read - should receive an ACK

	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, &data, 1, 1000);
	HAL_Delay(5);

	// Read 88bytes
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, &data, sizeof(data), 1000);		// Send 1 byte
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, data_rec, 4, 1000); 		    	// Receiv: 0x04, 0x11, 0x33, 0x43.
	HAL_Delay(5);
}

void ReadConfig(uint8_t *readCommand, uint16_t size, uint8_t *data_config) {

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, readCommand, 8, 1000); 		    // Send read command
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, data_config, size, 1000); 	        // Receive: data packet size, 0x01 0x23..., CRC_LSB, CRC_MSB
	HAL_Delay(5);
}


void WriteConfigZone(void){
	// WritePwd1 + ReadPwd1
	// comando para configuração da zona de configuração do slot: 0x00 e 0x01
	uint8_t configSlot0_1[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x05, 0x00, 0x60, 0xE9, 0x70, 0xE8, 0x53, 0x98};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot0_1, sizeof(configSlot0_1), 1000);
	HAL_Delay(5);

	// DATA0 + DATA1
	// comando para configuração da zona de configuração do slot: 0x02 e 0x03
	uint8_t configSlot2_3[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x06, 0x00,  0x40, 0xE1, 0x80, 0xA1,0xB0, 0x68 };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot2_3, sizeof(configSlot2_3), 1000);
	HAL_Delay(5);

	// PASSWORD1 + SECRET1
	// comando para configuração da zona de configuração do slot: 0x04 e 0x05
	uint8_t configSlot4_5[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x07, 0x00, 0x75, 0xC1, 0x60, 0xA0,  0x5B, 0x89};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot4_5, sizeof(configSlot4_5), 1000);
	HAL_Delay(5);

	// PASSWORD2 + SECRET2
	// comando para configuração da zona de configuração do slot: 0x06 e 0x07
	uint8_t configSlot6_7[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x08, 0x00,  0x47, 0xD1, 0x40, 0xB0, 0x2A, 0x61};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot6_7, sizeof(configSlot6_7), 1000);
	HAL_Delay(5);

	// ReadPwd2 + WritePwd2
	// comando para configuração da zona de configuração do slot: 0x08 e 0x09
	uint8_t configSlot8_9[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x09, 0x00, 0x49,  0xCE, 0x49,  0xCE, 0xDB, 0x4C};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot8_9, sizeof(configSlot8_9), 1000);
	HAL_Delay(5);

	// DATA[2] + DATA[3]
	// comando para configuração da zona de configuração do slot: 0x10 e 0x11
	uint8_t configSlot10_11[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x0A, 0x00, 0x49, 0xC8, 0x89, 0x88, 0xD8, 0xC0};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot10_11, sizeof(configSlot10_11), 1000);
	HAL_Delay(5);

	// PASSWORD[3] + SECRET[3]
	// comando para configuração da zona de configuração do slot: 0x12 e 0x13
	uint8_t configSlot12_13[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x0B, 0x00, 0x4D, 0xD8,  0x49, 0x90, 0xAC, 0x4A };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot12_13, sizeof(configSlot12_13), 1000);
	HAL_Delay(5);

	// MASTERKEY + DATA[4]
	// comando para configuração da zona de configuração do slot: 0x14 e 0x15
	uint8_t configSlot14_15[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x0C, 0x00, 0x0E, 0x0E, 0x49, 0x88, 0x2F, 0xE5 };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot14_15, sizeof(configSlot14_15), 1000);
	HAL_Delay(5);
}

void BlockConfigZone(uint8_t *receiv_byte){
	// Lock command: {COMMAND, COUNT, OPCODE, ZONE, CRC_88_LSB,  CRC_88_MSB, CRC_LSB, CRC_MSB}
	uint8_t blockConfig[] = { COMMAND, SIZE_BLOCK_CONFIG, COMMAND_LOCK, ZONE_CONFIG_LOCK, 0x00, 0xc3, 0xa4, 0x0f};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, blockConfig, sizeof(blockConfig), 1000);
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv_byte, 1, 1000);
	HAL_Delay(5);
}


void WriteDataZone(void){

	// Write command: {COMMAND, COUNT, OPCODE, Param1 + Param2_LSB + Param2_MSB + DADOS + CRC_LSB + CRC_MSB}
	uint8_t writeData0[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x00, 0x00 , 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04,
		 	 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,  0x3a, 0x04};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData0, sizeof(writeData0), 1000);
	HAL_Delay(5);

	uint8_t writeData1[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x08, 0x00 , 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x10, 0x11, 0x12, 0x13, 0x14,
	          0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x33, 0x59};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData1, sizeof(writeData1), 1000);
	HAL_Delay(5);

	uint8_t writeData2[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x10, 0x00, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24,
   	      0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0xbc, 0xea};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData2, sizeof(writeData2), 1000);
	HAL_Delay(5);

	uint8_t writeData3[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x18, 0x00, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x30, 0x31, 0x32, 0x33, 0x34,
 	        0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xb5, 0xb7};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData3, sizeof(writeData3), 1000);
	HAL_Delay(5);

	uint8_t writeData4[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x20, 0x00, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44,
	          0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x79, 0x73};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData4, sizeof(writeData4), 1000);
	HAL_Delay(5);

	uint8_t writeData5[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x28, 0x00, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x50, 0x51, 0x52, 0x53, 0x54,
	          0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x70, 0x2e};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData5, sizeof(writeData5), 1000);
	HAL_Delay(5);

	uint8_t writeData6[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x30, 0x00, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x60, 0x61, 0x62, 0x63, 0x64,
  	        0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xff, 0x9d};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData6, sizeof(writeData6), 1000);
	HAL_Delay(5);

	uint8_t writeData7[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x38, 0x00, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x70, 0x71, 0x72, 0x73, 0x74,
	          0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0xf6, 0xc0};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData7, sizeof(writeData7), 1000);
	HAL_Delay(5);

	uint8_t writeData8[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x40, 0x00, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x80, 0x81, 0x82, 0x83, 0x84,
	 	          0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x99, 0xff};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData8, sizeof(writeData8), 1000);
	HAL_Delay(5);

	uint8_t writeData9[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x48, 0x00, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x90, 0x91, 0x92, 0x93, 0x94,
            0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x90, 0xa2};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData9, sizeof(writeData9), 1000);
	HAL_Delay(5);

	uint8_t writeData10[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x50, 0x00, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
	          0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0x1f, 0x11};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData10, sizeof(writeData10), 1000);
	HAL_Delay(5);

	uint8_t writeData11[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x58, 0x00, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
	          0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0x16, 0x4c};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData11, sizeof(writeData11), 1000);
	HAL_Delay(5);

	uint8_t writeData12[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x60, 0x00, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4,
	          0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xda, 0x88};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData12, sizeof(writeData12), 1000);
	HAL_Delay(5);

	uint8_t writeData13[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x68, 0x00, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
			 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xd3, 0xd5 };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData13, sizeof(writeData13), 1000);
	HAL_Delay(5);

	// Write command: {COMMAND, COUNT, OPCODE, Param1 + Param2_LSB + Param2_MSB + DADOS + CRC_LSB + CRC_MSB}
	uint8_t writeData14[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x70, 0x00, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
			 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,  0x5c, 0x66};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData14, sizeof(writeData14), 1000);
	HAL_Delay(5);

	uint8_t writeData15[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x78, 0x00, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
			 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x55, 0x3b};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeData15, sizeof(writeData15), 1000);
	HAL_Delay(5);
}

void WriteOTPZone(void){

	// Write OTP command: {COMMAND, COUNT, OPCODE, ZONE, ADRESS, DADOS, CRC1, CRC2}
	uint8_t writeOTP0[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x00, 0x01, 0x02, 0x03,0x04, 0x05, 0x48};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP0, sizeof(writeOTP0), 1000);
	HAL_Delay(5);

	uint8_t writeOTP1[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x01, 0x01, 0x02, 0x03,0x04, 0x06, 0xf4};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP1, sizeof(writeOTP1), 1000);
	HAL_Delay(5);

	uint8_t writeOTP2[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x02, 0x01, 0x02, 0x03,0x04, 0x06, 0xd6};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP2, sizeof(writeOTP2), 1000);
	HAL_Delay(5);

	uint8_t writeOTP3[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x03, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6a};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP3, sizeof(writeOTP3), 1000);
	HAL_Delay(5);

	uint8_t writeOTP4[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x04, 0x01, 0x02, 0x03, 0x04, 0x06, 0xc7};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP4, sizeof(writeOTP4), 1000);
	HAL_Delay(5);

	uint8_t writeOTP5[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x7b};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP5, sizeof(writeOTP5), 1000);
	HAL_Delay(5);

	uint8_t writeOTP6[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x59};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP6, sizeof(writeOTP6), 1000);
	HAL_Delay(5);

	uint8_t writeOTP7[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x07, 0x01, 0x02, 0x03, 0x04, 0x06, 0xe5};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP7, sizeof(writeOTP7), 1000);
	HAL_Delay(5);

	uint8_t writeOTP8[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x08, 0x01, 0x02, 0x03, 0x04, 0x86, 0xcf};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP8, sizeof(writeOTP8), 1000);
	HAL_Delay(5);

	uint8_t writeOTP9[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x09, 0x01, 0x02, 0x03, 0x04, 0x85, 0x73};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP9, sizeof(writeOTP9), 1000);
	HAL_Delay(5);

	uint8_t writeOTP10[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x85, 0x51};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP10, sizeof(writeOTP10), 1000);
	HAL_Delay(5);

	uint8_t writeOTP11[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x0B, 0x01, 0x02, 0x03, 0x04, 0x86, 0xed};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP11, sizeof(writeOTP11), 1000);
	HAL_Delay(5);

	uint8_t writeOTP12[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x85, 0x40};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP12, sizeof(writeOTP12), 1000);
	HAL_Delay(5);

	uint8_t writeOTP13[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x0D, 0x01, 0x02, 0x03, 0x04, 0x86, 0xfc};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP13, sizeof(writeOTP13), 1000);
	HAL_Delay(5);

	uint8_t writeOTP14[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x0E, 0x01, 0x02, 0x03, 0x04, 0x86, 0xde};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP14, sizeof(writeOTP14), 1000);
	HAL_Delay(5);

	uint8_t writeOTP15[] = {COMMAND, SIZE_WRITE_OTP, COMMAND_WRITE, 0x01, 0x0F, 0x01, 0x02, 0x03, 0x04, 0x85, 0x62};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP15, sizeof(writeOTP15), 1000);
	HAL_Delay(5);
}

void BlockDataZone(void){
	// Lock command: {COMMAND, COUNT, OPCODE, ZONE, CRC_DATA_OTP_LSB,   CRC_DATA_OTP_MSB, CRC_LSB, CRC_MSB}
	//Data and OTP Zone: Seus conteúdos são concatenados nessa ordem para criar a entrada para o algoritmo CRC
	uint8_t blockConfig[] = { COMMAND, SIZE_BLOCK_CONFIG, COMMAND_LOCK, ZONE_DATA_LOCK, 0x04, 0x58, 0x66, 0xc7};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, blockConfig, sizeof(blockConfig), 1000);
	HAL_Delay(5);
}


void ReadDataZone(uint8_t *readData, uint16_t size, uint8_t *data) {

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, readData, 8, 1000); 		    // Send read command
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, data, size, 1000);
	HAL_Delay(5);
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

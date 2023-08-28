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

uint8_t CRC_LSB = 0x00;
uint8_t CRC_MSB = 0x00;


void atCRC(uint8_t *data, uint8_t size, uint8_t *crc_le)
{
    uint8_t counter;
    uint16_t crc_register = 0;
    uint16_t polynom = 0x8005;
    uint8_t shift_register;
    uint8_t data_bit, crc_bit;

    for (counter = 1; counter < (size - 2); counter++)
    {
        for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
        {
            data_bit = (data[counter] & shift_register) ? 1 : 0;
            crc_bit = crc_register >> 15;
            crc_register <<= 1;
            if (data_bit != crc_bit)
            {
                crc_register ^= polynom;
            }
        }
    }
    crc_le[0] = (uint8_t)(crc_register & 0x00FF);
    crc_le[1] = (uint8_t)(crc_register >> 8);
}


void WakeUp(uint8_t *receiv){

	uint8_t data = 0;

	HAL_I2C_Master_Receive(&hi2c2, 0xFE, &data, sizeof(data), 1000);
	HAL_Delay(5); // 2.5 ms para acordar; 45 ms para entrar em sleep
	// first read: 0 byte read - should receive an ACK

	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, &data, 1, 1000);
	HAL_Delay(5);

	// Read 88bytes
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, &data, sizeof(data), 1000);	// Send 1 byte
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, 4, 1000); 		    	// Receiv: 0x04, 0x11, 0x33, 0x43.
	HAL_Delay(5);
}


void ReadConfig(uint8_t *data, uint16_t size, uint8_t *receiv) {

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, data, 8, 1000); 		    // Send read command
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000); 	    // Receive: data packet size, 0x01 0x23..., CRC_LSB, CRC_MSB
	HAL_Delay(5);
}


void WriteConfigZone(void){

	// Comando para configuração da zona de configuração dos slots
	uint8_t configSlot[12];
	uint8_t CRC_receiv[2];
	uint8_t slot_addr = 0x04;
//	uint8_t config_1[] = {0xC8, 0xff, 0x55, 0x00, 0xE9, 0x60, 0xE8, 0x70, 0xE1, 0x40, 0xA1, 0x80, 0xC1, 0x75, 0xA0, 0x60, 0xD1, 0x47, 0xB0, 0x40, 0xCE, 0x49, 0xCE, 0x49,
//						 0xC8, 0x49, 0x88, 0x89, 0xD8, 0x4D,  0x90, 0x49, 0x0E, 0x0E, 0x88, 0x49};

	uint8_t config_2[] = {0xC8, 0x00, 0x55, 0x00, 0xC1, 0x40, 0xc0, 0x50, 0xc0, 0x40, 0x80, 0x80, 0xd2, 0x55, 0x80, 0x40, 0xc5, 0x43, 0x85, 0x40, 0xc5, 0x43, 0xc5, 0x43,
						  0xc5, 0x43, 0x85, 0x83, 0xc5, 0x43, 0x85, 0x43, 0x05, 0x03, 0x85, 0x43};

	for (uint8_t i = 0; i < sizeof(config_2); i += 4){

		configSlot[0] = COMMAND;
		configSlot[1] = SIZE_WRITE_CONFIG;
		configSlot[2] = COMMAND_WRITE;
		configSlot[3] = 0x00;
		configSlot[4] = slot_addr + (i / 4);
		configSlot[5] = 0x00;

		for (uint8_t j = 0; j < 4; j++) {
			configSlot[6 + j] = config_2[i + j];
		}

		atCRC(configSlot, sizeof(configSlot), CRC_receiv);

		configSlot[10] = CRC_receiv[0];
		configSlot[11] = CRC_receiv[1];

		HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot, sizeof(configSlot), 1000);
		HAL_Delay(30);
	}
}


void BlockConfigZone(uint8_t *receiv){

	uint8_t SendCommand[8];
	uint8_t BlockConfig[8];
	uint8_t CRC_receiv[2];
	uint8_t CRC_datablock[2];
	uint8_t DataZoneConfig[91];


	uint8_t DataReceive_35[35] = {0x00};
	uint8_t DataReceive_7[7] = {0x00};
	uint8_t adress = 0x08;
	uint8_t size = 35;
	uint8_t slot;

	SendCommand[0]= COMMAND;
	SendCommand[1]= 0x07;
	SendCommand[2]= 0x02;
	SendCommand[5]= 0x00;

	if (size == 35){
		for (uint8_t i = 0; i < 2; i++){
			slot = i*adress;

			SendCommand[3]= READ_WRITE_32;
			SendCommand[4]= slot;

			atCRC(SendCommand,sizeof(SendCommand),CRC_receiv);

			SendCommand[6]= CRC_receiv[0];
			SendCommand[7]= CRC_receiv[1];

			ReadConfig(SendCommand, size, DataReceive_35);

			uint8_t length = i * 32;
			for (uint8_t j = 1; j <= 32 ; j++){
				DataZoneConfig[length+j] = DataReceive_35[j];
			}

		}
		size = 7;
	}

	if(size == 7){
		for (uint8_t x = 0; x <= 5; x++){
			adress = 0x10;
			slot = x + adress;

			SendCommand[3]= READ_WRITE_4;
			SendCommand[4]= slot;

			atCRC(SendCommand,sizeof(SendCommand), CRC_receiv);
			SendCommand[6]= CRC_receiv[0];
			SendCommand[7]= CRC_receiv[1];

			ReadConfig(SendCommand, size, DataReceive_7);

			for(uint8_t y = 0; y <= 3; y++){
				uint8_t length = (x*4) + (64+1) + y;
				DataZoneConfig[length] = DataReceive_7[y+1];
		 	}

		 }
	}

	atCRC(DataZoneConfig,sizeof(DataZoneConfig), CRC_datablock);
	// Lock command: {COMMAND, COUNT, OPCODE, ZONE, CRC_88_LSB,  CRC_88_MSB, CRC_LSB, CRC_MSB}
	BlockConfig[0] = COMMAND;
	BlockConfig[1] = SIZE_BLOCK_CONFIG;
	BlockConfig[2] = COMMAND_LOCK;
	BlockConfig[3] = ZONE_CONFIG_LOCK;

	BlockConfig[4] = CRC_datablock[0];
	BlockConfig[5] = CRC_datablock[1];

	//uint8_t BlockConfig[] = { COMMAND, SIZE_BLOCK_CONFIG, COMMAND_LOCK, ZONE_CONFIG_LOCK, CRC_datablock[0], CRC_datablock[1],/* 0xed, 0xef*/
	//						  /*0xc4, 0xe1*/ 0x00, 0x00};
	atCRC(BlockConfig,sizeof(BlockConfig), CRC_receiv);
	BlockConfig[6] = CRC_receiv[0] ;
	BlockConfig[7] = CRC_receiv[1] ;

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, BlockConfig, sizeof(BlockConfig), 1000);
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, 4, 1000);
	HAL_Delay(10);
}


void WriteDataZone(void){

	// Write command: {COMMAND, COUNT, OPCODE, Param1 + Param2_LSB + Param2_MSB + DADOS + CRC_LSB + CRC_MSB}
	uint8_t writeData0[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x00, 0x00 , 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04,
		 	 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,  0x3a, 0x04};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData0, sizeof(writeData0), 1000);
	HAL_Delay(40);

	uint8_t writeData1[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x08, 0x00 , 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x10, 0x11, 0x12, 0x13, 0x14,
	          0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x33, 0x59};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData1, sizeof(writeData1), 1000);
	HAL_Delay(40);

	uint8_t writeData2[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x10, 0x00, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24,
   	      0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0xbc, 0xea};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData2, sizeof(writeData2), 1000);
	HAL_Delay(40);

	uint8_t writeData3[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x18, 0x00, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x30, 0x31, 0x32, 0x33, 0x34,
 	        0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xb5, 0xb7};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData3, sizeof(writeData3), 1000);
	HAL_Delay(40);

	uint8_t writeData4[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x20, 0x00, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44,
	          0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x79, 0x73};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData4, sizeof(writeData4), 1000);
	HAL_Delay(40);

	uint8_t writeData5[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x28, 0x00, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x50, 0x51, 0x52, 0x53, 0x54,
	          0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x70, 0x2e};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData5, sizeof(writeData5), 1000);
	HAL_Delay(40);

	uint8_t writeData6[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x30, 0x00, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x60, 0x61, 0x62, 0x63, 0x64,
  	        0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xff, 0x9d};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData6, sizeof(writeData6), 1000);
	HAL_Delay(40);

	uint8_t writeData7[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x38, 0x00, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x70, 0x71, 0x72, 0x73, 0x74,
	          0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0xf6, 0xc0};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData7, sizeof(writeData7), 1000);
	HAL_Delay(40);

	uint8_t writeData8[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x40, 0x00, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x80, 0x81, 0x82, 0x83, 0x84,
	 	          0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x99, 0xff};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData8, sizeof(writeData8), 1000);
	HAL_Delay(40);

	uint8_t writeData9[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x48, 0x00, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x90, 0x91, 0x92, 0x93, 0x94,
            0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x90, 0xa2};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData9, sizeof(writeData9), 1000);
	HAL_Delay(40);

	uint8_t writeData10[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x50, 0x00, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
	          0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0x1f, 0x11};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData10, sizeof(writeData10), 1000);
	HAL_Delay(40);

	uint8_t writeData11[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x58, 0x00, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4,
	          0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0x16, 0x4c};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData11, sizeof(writeData11), 1000);
	HAL_Delay(40);

	uint8_t writeData12[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x60, 0x00, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4,
	          0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xda, 0x88};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData12, sizeof(writeData12), 1000);
	HAL_Delay(40);

	uint8_t writeData13[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x68, 0x00, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
			 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xd3, 0xd5 };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData13, sizeof(writeData13), 1000);
	HAL_Delay(40);

	// Write command: {COMMAND, COUNT, OPCODE, Param1 + Param2_LSB + Param2_MSB + DADOS + CRC_LSB + CRC_MSB}
	uint8_t writeData14[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x70, 0x00, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
			 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,  0x5c, 0x66};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeData14, sizeof(writeData14), 1000);
	HAL_Delay(40);

	uint8_t writeData15[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x78, 0x00, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
			 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0x55, 0x3b};
	HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeData15, sizeof(writeData15), 1000);
	HAL_Delay(40);
}


void WriteOTPZone(void){

	uint8_t writeOTP[11];
	uint8_t CRC_receiv[2];
	uint8_t adress = 0x00;
	uint8_t OTP_Zone[4] = {0x01, 0x02, 0x03, 0x04};
	// Write OTP command: {COMMAND, COUNT, OPCODE, ZONE, ADRESS, DADOS, CRC1, CRC2}

	for (uint8_t i = 0; i < 0x10; i++){

		writeOTP[0] = COMMAND;
		writeOTP[1] = SIZE_WRITE_OTP;
		writeOTP[2] = COMMAND_WRITE;
		writeOTP[3] = ZONE_OTP;
		writeOTP[4] = adress + i;

		for(uint8_t j = 0; j < 5; j++){
			writeOTP[6 + j] = OTP_Zone[j];
		}

		atCRC(writeOTP,sizeof(writeOTP), CRC_receiv);
		writeOTP[sizeof(writeOTP) - 2] = CRC_receiv[0];
		writeOTP[sizeof(writeOTP) - 1] = CRC_receiv[1];

		HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP, sizeof(writeOTP), 1000);
		HAL_Delay(5);
	}
}

void BlockDataZone(void){

	// Lock command: {COMMAND, COUNT, OPCODE, ZONE, CRC_DATA_OTP_LSB, CRC_DATA_OTP_MSB, CRC_LSB, CRC_MSB}
	//Data and OTP Zone: Seus conteúdos são concatenados nessa ordem para criar a entrada para o algoritmo CRC
	uint8_t CRC_receiv[2];
	uint8_t blockConfig[8];
	//{ COMMAND, SIZE_BLOCK_CONFIG, COMMAND_LOCK, ZONE_DATA_LOCK, 0x04, 0x58, /*0x66, 0xc7*/ 0x00, 0x00};
	blockConfig [0] = COMMAND;
	blockConfig [1] =SIZE_BLOCK_CONFIG ;
	blockConfig [2] = COMMAND_LOCK;
	blockConfig [3] = ZONE_DATA_LOCK;
	blockConfig [4] = 0x04;
	blockConfig [5] = 0x58;

	atCRC(blockConfig,sizeof(blockConfig), CRC_receiv);
	blockConfig[6] = CRC_receiv[0] ;
	blockConfig[7] = CRC_receiv[1] ;

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, blockConfig, sizeof(blockConfig), 1000);
	HAL_Delay(5);
}


void ReadDataZone(uint8_t *data, uint16_t size, uint8_t *receiv) {

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, data, 8, 1000);
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}


void CommandNonce(uint8_t *NumIn, uint16_t size, uint8_t *receiv){ //OK

	// NONCE command: {COMMAND, COUNT, OPCODE, Param1_mode, 0x00, 0x00, NumIn[20], CRC_LSB, CRC_MSB}
	uint8_t noncecommand[28]; //40
	uint8_t CRC_receiv[2];

    noncecommand[0] = COMMAND;
    noncecommand[1] = SIZE_WRITE_NONCE20; //SIZE_WRITE_NONCE32;
    noncecommand[2] = COMMAND_NONCE;
    noncecommand[3] = 0x00; // modo3
    noncecommand[4] = 0x00;
    noncecommand[5] = 0x00;

	for(uint8_t i = 0; i <= 20; i++){  // <=32
		 noncecommand[6 + i] = NumIn[i];
	}

	atCRC(noncecommand,sizeof(noncecommand), CRC_receiv);
	noncecommand[sizeof(noncecommand) - 2] = CRC_receiv[0] ;
	noncecommand[sizeof(noncecommand) - 1] = CRC_receiv[1] ;

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, noncecommand, sizeof(noncecommand), 1000);
	HAL_Delay(40);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(10);
}


void GendigCommand(uint8_t SlotID_LSB, uint8_t SlotID_MSB, uint8_t size, uint8_t *receiv){

	// GENDIG command: {COMMAND, COUNT, OPCODE, ZONE_DATA, SLOTID_LSB, SLOTID_MSB, CRC_LSB, CRC_MSB}
	uint8_t GenDig[8];
	uint8_t CRC_receiv[2];
	//{COMMAND, SIZE_WRITE_GENDIG, COMMAND_GENDIG, ZONE_DATA, SlotID_LSB, SlotID_MSB, /*0x33, 0xe8*/ 0x00, 0x00};
	//atCRC(GenDig,sizeof(GenDig));

	GenDig[0] = COMMAND;
	GenDig[1] = SIZE_WRITE_GENDIG;
	GenDig[2] = COMMAND_GENDIG;
	GenDig[3] = ZONE_DATA;
	GenDig[4] = SlotID_LSB;
	GenDig[5] = SlotID_MSB;

	atCRC(GenDig,sizeof(GenDig), CRC_receiv);
	GenDig[6] = CRC_receiv[0] ;
	GenDig[7] = CRC_receiv[1] ;

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, GenDig, sizeof(GenDig), 1000);
	HAL_Delay(40);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}

void MacCommand(uint8_t SlotID_LSB, uint8_t SlotID_MSB, uint16_t size, uint8_t *receiv){

	uint8_t  MAC[8];
	uint8_t CRC_receiv[2];

	//uint8_t Challenge [32] = {0x00};

	 MAC[0]= COMMAND;
	 MAC[1]= 0x07; 	//size 0x27
	 MAC[2]= COMMAND_MAC;
	 MAC[3]= 0x01 ; 	//mode
	 MAC[4]= SlotID_LSB;
	 MAC[5]= SlotID_MSB;
/*
	for(uint8_t i = 0; i <= 32; i++){
		MAC[6 + i] = Challenge[i];
	}
*/
	atCRC(MAC, sizeof(MAC), CRC_receiv);
	MAC[sizeof(MAC) - 2] = CRC_receiv[0] ;
	MAC[sizeof(MAC) - 1] = CRC_receiv[1] ;

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, MAC, sizeof(MAC), 1000);
	HAL_Delay(40);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}


void CheckMacCommand(uint8_t SlotID_LSB, uint8_t SlotID_MSB, uint8_t *ClientResp, uint16_t size, uint8_t *receiv) {
    uint8_t CheckMAC[85] = {0};
    uint8_t CRC_receiv[2];
    uint8_t size_att = 0; // Inicializado para 0

    uint8_t ClientChal[32] = {0};
    uint8_t OtherData[13] = {0x08, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    CheckMAC[0] = 0x03;
    CheckMAC[1] = 0x55;
    CheckMAC[2] = COMMAND_CHECKMAC;
    CheckMAC[3] = 0x01;    // mode
    CheckMAC[4] = SlotID_LSB;
    CheckMAC[5] = SlotID_MSB;

    for (uint8_t i = 0; i <= 77; i++) {
        if (i <= 32) {
            CheckMAC[6 + i] = ClientChal[i];
            size_att = 6 + i;
        } else if (i > 32 && i <= 64) {
            CheckMAC[size_att] = ClientResp[i - 32];
            size_att++; // Correção: era += i, deve ser incremento simples
        } else if (i > 63 && i <= 77) {
            CheckMAC[size_att] = OtherData[i - 65];
            size_att++;
        }
    }

    atCRC(CheckMAC, sizeof(CheckMAC), CRC_receiv);
    CheckMAC[sizeof(CheckMAC) - 2] = CRC_receiv[0];
    CheckMAC[sizeof(CheckMAC) - 1] = CRC_receiv[1];

    HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, CheckMAC, sizeof(CheckMAC), 1000);
    HAL_Delay(40);
    HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
    HAL_Delay(5);
}


void SHACommandInit(uint16_t size, uint8_t *receiv){

	// SHA command: {COMMAND, COUNT, OPCODE, Param1(Mode), Param2 (0x00, 0x00), CRC_LSB, CRC_MSB}
	//uint8_t SHA[] = {COMMAND, 0x07, COMMAND_SHA, SHA_INIT,  Param2_1, Param2_2, /*0x2e, 0x85*/ 0x00, 0x00};
	//atCRC(SHA,sizeof(SHA));
/*
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, SHA, sizeof(SHA), 1000);
	HAL_Delay(30);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5)

	*/
}


void SHACommandCompute(uint8_t *data, uint8_t size, uint8_t *receiv){

	// SHA command: {COMMAND, COUNT, OPCODE, Param1(Mode), Param2 (0x00, 0x00), [data], CRC_LSB, CRC_MSB}
	uint8_t SHA[72];
	uint8_t CRC_receiv[2];

	SHA[0] = COMMAND;
	SHA[1] = 0x47;
	SHA[2] = COMMAND_SHA;
	SHA[3] = SHA_COMPUTE;
	SHA[4] = 0x00;
	SHA[5] = 0x00;

	for (uint8_t i = 0; i < 35; i++) {
		 SHA[i+6] = data[i];
	}

	atCRC(SHA, sizeof(SHA), CRC_receiv);
	SHA[6] = CRC_receiv[0];
	SHA[7] = CRC_receiv[1];

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, SHA, sizeof(SHA), 1000);
	HAL_Delay(30);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}


void TempKeyGen(uint8_t *data, uint8_t *NumIn, uint8_t mode, uint8_t size_in, uint8_t size_out, uint8_t *receive){

	 uint8_t op_mode;
	 op_mode = mode;
	// Tempkey [] = {RandOut, NumIn_receiv, COMMAND_NONCE, NONCE_MODE0, LSB_Param, CRC_LSB, CRC_MSB};
	uint8_t Tempkey[size_in];
	uint8_t LSB_Param = 0x00;
	uint8_t CRC_receiv[2];

	// op_mode = 0x00 -> nonce_gen ; 0x01 -> gendig_gen
	if (op_mode == 0) {
		for (uint8_t i = 0; i<=  32; i++) {
    		Tempkey[i] = data[i+1];
    	}

		for (uint8_t i = 0; i <= 20; i++){
			Tempkey[33 + i] = NumIn[i];
		}
	}

	Tempkey[sizeof(Tempkey) - 5] = COMMAND_NONCE ;
	Tempkey[sizeof(Tempkey) - 4] = NONCE_MODE0;
	Tempkey[sizeof(Tempkey) - 3] = LSB_Param;

	atCRC(Tempkey,sizeof(Tempkey), CRC_receiv);
	Tempkey[sizeof(Tempkey) - 2] = CRC_receiv[0];
	Tempkey[sizeof(Tempkey) - 1] = CRC_receiv[1];

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, Tempkey, size_in, 1000);
	HAL_Delay(30);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receive, size_out, 1000);
	HAL_Delay(5);
}


void ReadEncript(uint8_t *data, uint16_t size, uint8_t *receiv){

	// Read Encript
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, data, 8, 1000); // Send read encript command
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}


void WriteEncript(void){

	// Write command: {COMMAND, COUNT, OPCODE, Param1, Param2_LSB, Param2_MSB, DADOS, CRC_LSB, CRC_MSB}
	uint8_t writeEncript[] = {COMMAND, SIZE_WRITE_DATA, COMMAND_WRITE, 0x82, 0x00, 0x00 , 0xA0, 0xA1, 0xA2, 0xA3, 0xA4,
	0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xA0, 0xA1,
	0xA2, 0xA3,	0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9,  /*0x15, 0xA9*/ 0x00, 0x00};

	//atCRC(writeEncript,sizeof(writeEncript));
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeEncript, sizeof(writeEncript), 1000);
	HAL_Delay(5);
}

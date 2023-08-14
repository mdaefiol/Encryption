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


void atCRC(uint8_t *data, uint8_t size)
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
    data[size - 2] = (uint8_t)(crc_register & 0x00FF);
    data[size - 1] = (uint8_t)(crc_register >> 8);
}


void WakeUp(uint8_t *receiv){

	uint8_t data = 0;

	HAL_I2C_Master_Receive(&hi2c2, 0xFE, &data, sizeof(data), 1000);
	HAL_Delay(5); // 2.5 ms para acordar; 45 ms para entrar em sleep
	// first read: 0 byte read - should receive an ACK

	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, &data, 1, 1000);
	HAL_Delay(5);

	// Read 88bytes
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, &data, sizeof(data), 1000);		// Send 1 byte
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, 4, 1000); 		    	// Receiv: 0x04, 0x11, 0x33, 0x43.
	HAL_Delay(5);
}


void ReadConfig(uint8_t *data, uint16_t size, uint8_t *receiv) {

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, data, 8, 1000); 		    // Send read command
	HAL_Delay(5);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000); 	        // Receive: data packet size, 0x01 0x23..., CRC_LSB, CRC_MSB
	HAL_Delay(5);
}


void WriteConfigZone(void){

	// WritePwd1 + ReadPwd1
	// comando para configuração da zona de configuração do slot: 0x00 e 0x01
	uint8_t configSlot0_1[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x05, 0x00, 0xE9, 0x60, 0xE8, 0x70,  0x68, 0x11};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot0_1, sizeof(configSlot0_1), 1000);
	HAL_Delay(10);

	// DATA0 + DATA1
	// comando para configuração da zona de configuração do slot: 0x02 e 0x03
	uint8_t configSlot2_3[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x06, 0x00,  0xE1, 0x40, 0xA1, 0x80, 0x00, 0x00};
	atCRC(configSlot2_3,sizeof(configSlot2_3));
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot2_3, sizeof(configSlot2_3), 1000);
	HAL_Delay(10);

	// PASSWORD1 + SECRET1
	// comando para configuração da zona de configuração do slot: 0x04 e 0x05
	uint8_t configSlot4_5[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x07, 0x00, 0xC1, 0x75, 0xA0, 0x60,  0x3a, 0x65};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot4_5, sizeof(configSlot4_5), 1000);
	HAL_Delay(10);

	// PASSWORD2 + SECRET2
	// comando para configuração da zona de configuração do slot: 0x06 e 0x07
	uint8_t configSlot6_7[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x08, 0x00,  0xD1, 0x47, 0xB0, 0x40, 0x2d, 0xf0};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot6_7, sizeof(configSlot6_7), 1000);
	HAL_Delay(10);

	// ReadPwd2 + WritePwd2
	// comando para configuração da zona de configuração do slot: 0x08 e 0x09
	uint8_t configSlot8_9[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x09, 0x00, 0xCE,  0x49, 0xCE,  0x49, 0x2b, 0x91};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot8_9, sizeof(configSlot8_9), 1000);
	HAL_Delay(10);

	// DATA[2] + DATA[3]
	// comando para configuração da zona de configuração do slot: 0x10 e 0x11
	uint8_t configSlot10_11[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x0A, 0x00, 0xC8, 0x49, 0x88, 0x89, 0xfa, 0xdd};
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot10_11, sizeof(configSlot10_11), 1000);
	HAL_Delay(10);

	// PASSWORD[3] + SECRET[3]
	// comando para configuração da zona de configuração do slot: 0x12 e 0x13
	uint8_t configSlot12_13[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x0B, 0x00, 0xD8, 0x4D,  0x90, 0x49, 0xfb, 0xaf };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot12_13, sizeof(configSlot12_13), 1000);
	HAL_Delay(10);

	// MASTERKEY + DATA[4]
	// comando para configuração da zona de configuração do slot: 0x14 e 0x15
	// commanda, count, opcode, zone
	uint8_t configSlot14_15[] = {COMMAND, SIZE_WRITE_CONFIG, COMMAND_WRITE, 0x00, 0x0C, 0x00, 0x0E, 0x0E, 0x88, 0x49, 0x2F, 0xEC };
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, configSlot14_15, sizeof(configSlot14_15), 1000);
	HAL_Delay(10);
}


void BlockConfigZone(uint8_t *receiv){

	uint8_t DataReceive_35[35] = {0x00};
	uint8_t DataReceive_7[7] = {0x00};
	uint8_t DataZoneConfig[90] = {0x00};
	uint8_t adress = 0x08;
	uint8_t size = 35;
	uint8_t slot;

	if (size == 35){
		for (uint8_t i = 0; i < 2; i++){
			slot = i*adress;
			uint8_t SendCommand[8] = {0x03, 0x07, 0x02, READ_WRITE_32, slot, 0x00, 0x00, 0x00};
			atCRC(SendCommand,sizeof(SendCommand));
			ReadConfig(SendCommand, size, DataReceive_35);

			uint8_t length = i * 32;
			for (uint8_t j = 0; j <= 32; j++){
				DataZoneConfig[length + j] = DataReceive_35[j+1];
			}
		}
		size = 7;
	}

	if(size == 7){
		for (uint8_t x = 0; x <= 5; x++){
			adress = 0x10;
			slot = x + adress;
			uint8_t SendCommand[8] = {0x03, 0x07, 0x02, READ_WRITE_4, slot, 0x00, 0x00, 0x00};
			atCRC(SendCommand,sizeof(SendCommand));
			ReadConfig(SendCommand, size, DataReceive_7);

			for(uint8_t y = 0; y <= 3; y++){
				uint8_t length = (x*4) + 64 + y;
				DataZoneConfig[length] = DataReceive_7[y+1];
		 	}
		 }
	}


	atCRC(DataZoneConfig, sizeof(DataZoneConfig));

	// Lock command: {COMMAND, COUNT, OPCODE, ZONE, CRC_88_LSB,  CRC_88_MSB, CRC_LSB, CRC_MSB}
	uint8_t blockConfig[] = { COMMAND, SIZE_BLOCK_CONFIG, COMMAND_LOCK, ZONE_CONFIG_LOCK, 0xc5, 0xc6,/*0xc4, 0xe1*/ 0x00, 0x00};
	atCRC(blockConfig,sizeof(blockConfig));
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, blockConfig, sizeof(blockConfig), 1000);
	HAL_Delay(10);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, 1, 1000);
	HAL_Delay(10);

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

	uint8_t writeOTP[11];
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

		writeOTP[sizeof(writeOTP) - 2] = 0x00;
		writeOTP[sizeof(writeOTP) - 1] = 0x00;

		atCRC(writeOTP,sizeof(writeOTP));
		HAL_I2C_Master_Transmit(&hi2c2, 0xC8, writeOTP, sizeof(writeOTP), 1000);
		HAL_Delay(5);
	}
}

void BlockDataZone(void){

	// Lock command: {COMMAND, COUNT, OPCODE, ZONE, CRC_DATA_OTP_LSB, CRC_DATA_OTP_MSB, CRC_LSB, CRC_MSB}
	//Data and OTP Zone: Seus conteúdos são concatenados nessa ordem para criar a entrada para o algoritmo CRC
	uint8_t blockConfig[] = { COMMAND, SIZE_BLOCK_CONFIG, COMMAND_LOCK, ZONE_DATA_LOCK, 0x04, 0x58, /*0x66, 0xc7*/ 0x00, 0x00};
	atCRC(blockConfig,sizeof(blockConfig));
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
	uint8_t noncecommand[28];

	for(uint8_t i = 0; i <= 20; i++){
		 noncecommand[6 + i] = NumIn[i];
	}

    noncecommand[0] = COMMAND;
    noncecommand[1] = SIZE_WRITE_NONCE;
    noncecommand[2] = COMMAND_NONCE;
    noncecommand[3] = NONCE_MODE0;
    noncecommand[4] = 0x00;
    noncecommand[5] = 0x00;
    noncecommand[sizeof(noncecommand) - 2] = 0x00;
    noncecommand[sizeof(noncecommand) - 1] = 0x00;

	atCRC(noncecommand,sizeof(noncecommand));

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, noncecommand, sizeof(noncecommand), 1000);
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(10);
}


void GendigCommand(uint8_t SlotID_LSB, uint8_t SlotID_MSB, uint8_t size, uint8_t *receiv){

	// GENDIG command: {COMMAND, COUNT, OPCODE, ZONE_DATA, SLOTID_LSB, SLOTID_MSB, CRC_LSB, CRC_MSB}
	uint8_t GenDig[] = {COMMAND, SIZE_WRITE_GENDIG, COMMAND_GENDIG, ZONE_DATA, SlotID_LSB, SlotID_MSB, /*0x33, 0xe8*/ 0x00, 0x00};
	atCRC(GenDig,sizeof(GenDig));

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, GenDig, sizeof(GenDig), 1000);
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}


void SHACommandInit(uint16_t size, uint8_t *receiv){

	// SHA command: {COMMAND, COUNT, OPCODE, Param1(Mode), Param2 (0x00, 0x00), CRC_LSB, CRC_MSB}
	uint8_t Param2_1 = 0x00;
	uint8_t Param2_2 = 0x00;

	uint8_t SHA[] = {COMMAND, 0x07, COMMAND_SHA, SHA_INIT, Param2_1, Param2_2, /*0x2e, 0x85*/ 0x00, 0x00};
	atCRC(SHA,sizeof(SHA));

	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, SHA, sizeof(SHA), 1000);
	HAL_Delay(30);
	HAL_I2C_Master_Receive(&hi2c2, I2C_ADDRESS, receiv, size, 1000);
	HAL_Delay(5);
}


void SHACommandCompute(uint8_t *data, uint8_t size, uint8_t *receiv){

	// SHA command: {COMMAND, COUNT, OPCODE, Param1(Mode), Param2 (0x00, 0x00), [data], CRC_LSB, CRC_MSB}
	uint8_t SHA[72];

	SHA[0] = COMMAND;
	SHA[1] = 0x47;
	SHA[2] = COMMAND_SHA;
	SHA[3] = SHA_COMPUTE;
	SHA[4] = 0x00;
	SHA[5] = 0x00;

	for (uint8_t i = 0; i < 35; i++) {
		 SHA[i+6] = data[i];
	}

	SHA[sizeof(SHA) - 2] = 0x00;
	SHA[sizeof(SHA) - 1] = 0x00;
	atCRC(SHA, sizeof(SHA));

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
	Tempkey[sizeof(Tempkey) - 2] = 0x00;
	Tempkey[sizeof(Tempkey) - 1] = 0x00;

	atCRC(Tempkey,sizeof(Tempkey));
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

	atCRC(writeEncript,sizeof(writeEncript));
	HAL_I2C_Master_Transmit(&hi2c2, I2C_ADDRESS, writeEncript, sizeof(writeEncript), 1000);
	HAL_Delay(5);
}

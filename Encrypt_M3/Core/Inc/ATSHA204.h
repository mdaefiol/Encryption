/*
 * ATSHA204.h
 *
 *  Created on: 24 de abr de 2023
 *      Author: oriongames
 */

#ifndef INC_ATSHA204_H_
#define INC_ATSHA204_H_

#define SHA204_WRITE 0x01
#define SHA204_READ 0x03


void WakeUp(void);
void SerialRead(void);


//void sha204c_calculate_crc(uint8_t length, uint8_t *data, uint8_t *crc) ;

#endif /* INC_ATSHA204_H_ */

#ifndef _CRC_H
#define _CRC_H

#include <stdint.h>

uint16_t crc16_update(uint16_t crc,uint8_t data);
uint16_t crc16_update_bytes(uint16_t crc,uint8_t *data,uint32_t len);

#endif
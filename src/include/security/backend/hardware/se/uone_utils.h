/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/
 

#ifndef _UONE_UTILS_H__
#define _UONE_UTILS_H__

#include <stdint.h>

#define HIGH(x)     (uint8_t) (((x) & 0xFF00) >> 8)
#define LOW(x)      (uint8_t) ((x) & 0x00FF)


uint16_t crc16_genibus (uint8_t *src, size_t len);

uint8_t check_CRC16 (uint16_t *crc, uint8_t *sec_readdata, uint16_t *length);

void make_Key_index (uint16_t *index, uint8_t sub_type, uint8_t slot);

uint8_t parse_key_index (uint16_t key_index, uint8_t *sub_type, uint8_t *slot);

int stringToHex(unsigned char *string, unsigned char *hexstring);


#endif      // __UONE_UTILS_H_

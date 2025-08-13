/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/

#ifndef __HAL_UONE_CUSTOM_H__
#define __HAL_UONE_CUSTOM_H__


// Debug Level
extern unsigned char debug_level;

// Define - DEBUG LEVEL
// S #################################################################################
typedef enum {

    DEBUG_LEVEL_NO_DEBUG,   // 0x00
    DEBUG_LEVEL_HAL,        // 0x01
    DEBUG_LEVEL_UONE,       // 0x02
    DEBUG_LEVEL_PROCESS,    // 0x03

} debugLevel;
// E #################################################################################


/* Enumerator */
typedef enum {

    /*  Normal  */
    HAL_SUCCESS,                // 0x00

    /*  HAL ERROR  */
    HAL_NOT_INITIALIZED,        // 0x01
    HAL_INVALID_ARGS,           // 0x02
    HAL_INVALID_SLOT,           // 0x03 - ex. request to save key into cert slot
    HAL_INVALID_LENGTH,         // 0x04
    HAL_BAD_KEY_PAIR,           // 0x05 - ex. public and private keys do not match
    HAL_BAD_CERT,               // 0x06
    HAL_BAD_CERTKEY_PAIR,       // 0x07 - ex. certificate and key do not match
    HAL_NOT_ENOUGH_MEMORY,      // 0x08
    HAL_ALLOC_FAIL,             // 0x09
    HAL_KEY_IN_USE,             // 0x0A
    HAL_CERT_IN_USE,            // 0x0B
    HAL_DATA_IN_USE,            // 0x0C
    HAL_NOT_SUPPORTED,          // 0x0D
    HAL_BUSY,                   // 0x0E
    HAL_FAIL,                   // 0x0F

} hal_result_e;


typedef struct _hal_data {
    uint8_t *data;
    uint32_t data_len;
} hal_data;


typedef enum {
    HAL_KEY_ECC_EDWARD_ED25519,     // 0x00
    HAL_KEY_ECC_EDWARD_X25519,      // 0x01

    HAL_KEY_UNKNOWN,                // 0x02
} hal_key_type;


// Define - Size Values
// S #################################################################################

// EdDSA Signature Buffer Size
#define HAL_MAX_ECDSA_LEN 256

#define HAL_SIZE_EDDSA_SIGN_BUFFER      32

// E #################################################################################


#endif      // __HAL_UONE_CUSTOM_H__

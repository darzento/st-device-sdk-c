/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/
#ifndef __UONE_TYPE_H__
#define __UONE_TYPE_H__

/*
 * Debug Macro
*/
// S #################################################################################

#define UONE_TAG "[UONE-A09]"   // UONE-A09 = Diplomat

#define UONE_LOG printf


// For API Entry.
#define UONE_ENTER     \
  do {      \
    UONE_LOG("\n " UONE_TAG " [INFO] %s() API - In.\n", __FUNCTION__);  \
  } while (0)


// If debug level is "DEBUG_LEVEL_PROCSS(Level.3)" , API entry macro is process.
#define UONE_DEBUG_ENTER   \
    do {    \
        if ( (debug_level) == (DEBUG_LEVEL_PROCESS) ) { \
            UONE_ENTER;    \
        }   \
    } while (0)


// If debug level is greater than "DEBUG_STATS", Log print.
#define UONE_DEBUG_LOG(msg, ret, DEBUG_STATE)     \
    do {    \
        if ( (debug_level) >= (DEBUG_STATE) ) {     \
            UONE_LOG(msg, (ret) ); \
        }   \
    } while (0)


// debug_level check with IN STATE
#define IS_DEBUG_LEVEL(DEBUG_STATE)  ( (debug_level) >= DEBUG_STATE)

// E #################################################################################


/*
 * Define - Key Index Format of Driver Informations.
*/
// S #################################################################################

/* Sub Type */
// Asymmetric Key
#define SUB_TYPE_PRIVATE_KEY    0x00
#define SUB_TYPE_PUBLIC_KEY     0x01
#define SUB_TYPE_KEY_PAIR       0x02

/* Slot */
#define SLOT_ODM_ED25519        0x00    // Factory Persoed EdKey

typedef enum {
    SLOT_EP_KEY_0_NVM         = 0x20,   // 0x20
    SLOT_EP_KEY_1_NVM,                  // 0x21
    SLOT_EP_KEY_2_NVM,                  // 0x22
    SLOT_EP_KEY_3_NVM,                  // 0x23
    SLOT_EP_KEY_4_NVM,                  // 0x24
    SLOT_EP_KEY_5_NVM,                  // 0x25
    SLOT_EP_KEY_6_NVM,                  // 0x26
    SLOT_EP_KEY_7_NVM,                  // 0x27
    SLOT_EP_KEY_8_NVM,                  // 0x28
    SLOT_EP_KEY_9_NVM,                  // 0x29
} slot_ephemeral_key_index;

#define SLOT_EP_KEY_0_RAM       0x30
#define SLOT_EP_KEY_1_RAM       0x31
#define SLOT_EP_KEY_2_RAM       0x32


// eSE External Key Index
#define SE_EXT_SETED_KEY_USED_FLAG      0x0001

// E #################################################################################


/*
 * Storage SFI
*/
// S #################################################################################
typedef enum {
    SLOT_SECURE_STORAGE_1F = 0x1F,
} slot_secure_storage;

// Define - Key Index Mappint Offset
#define SE_KEY_INDEX_MAPPING_EP_NVM         0x1D
#define SE_KEY_INDEX_MAPPING_EP_RAM         0xC0

// Define - Storage Index Mapping Offset
#define SE_STORAG_LENGTH_SIZE               0x02
#define SE_STORAGE_INDEX_MAPPING_SECURE     0x03
// E #################################################################################


// Define - KEY_TYPE
typedef enum {

    KEY_ECC_EDWARD_ED25519,     // 0x00 - Curve for Ed25519
    KEY_ECC_EDWARD_X25519,      // 0x01 - Curve for X25519

    KEY_UNKOWN,                 // 0x02

} key_type;


// UONE Generate Secret Parameters.
#define SE_GENERATE_SECRET_ECDHE        0x01
// #define SE_GENERATE_SECRET_ECDH      0x02    // Now, Not Supported by eSE
// #define SE_GENERATE_SECRET_DH        0x03    // Now, Not Supported by eSE


#endif      // __UONE_TYPE_H_

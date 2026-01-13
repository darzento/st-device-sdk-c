/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/

/*!
 *  @file       uone_custom.h
 *  @brief      Headerfile : util functions to support memset, memcpy, memcmp
 *  @author     
 *  @version    v0.1 : 20xx.xx.xx Init. release version
 */
#ifndef __UONE_CUSTOM_H__
#define __UONE_CUSTOM_H__


#include <stdint.h>

#include "security/backend/hardware/se/Hal_uone_custom.h"
#include "security/backend/hardware/se/uone_type.h"
#include "security/backend/hardware/se/uone_utils.h"


extern uint32_t uone_driver_version;


// Define - I2C Related Value
// S #################################################################################
#define REAL_SE_ADDRESS             0x01
#define SE_ADDRESS                  (REAL_SE_ADDRESS << 1)

#define I2C_MASTER_NUM              I2C_NUM_0   /* I2C port number for master dev */

#define I2C_MASTER_TX_BUF_DISABLE   0           /* I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0           /* I2C master do not need buffer */

#define UB_POWER_CTRL_PIN           4
#define I2C_MASTER_SCL_IO           5           /* gpio number for i2c master clock  */
#define I2C_MASTER_SDA_IO           6           /* gpio number for i2c master data */
#define I2C_MASTER_RESET_PIN        7

#define I2C_MASTER_FREQ_HZ          100000      /* I2C master clock frequency */

#define WRITE_BIT                   I2C_MASTER_WRITE    /* I2C master write */
#define READ_BIT                    I2C_MASTER_READ     /* I2C master read */

#define ACK_CHECK_EN                0x1         /* I2C master will check ack from slave */
#define ACK_CHECK_DIS               0x0         /* I2C master will not check ack from slave */
#define ACK_VAL                     0x0         /* I2C ack value */

#define NACK_VAL                    0x1         /* I2C nack value */

#define I2C_LIMIT_TRANS_TIME_MS     50          /* I2C Transfer Limit Time (ms) */
// E #################################################################################


// Define - I2C Delay & Values
// S #################################################################################
#define POLLING_I2C_SHORT_DELAY
#define I2C_SHORT_DELAY_COUNTER             5
#define I2C_SHORT_DELAY_MS                  5
#define I2C_LONG_DELAY_MS                   (I2C_SHORT_DELAY_MS * 10)

// Delay MS Values for Processing Time of eSE
#define UONE_API_GENERAL                    3       // ms
#define UONE_API_GENERATE_RANDOM            5       // ms

#define UONE_GEN_ASYMMETRIC_KEY             80      // ms
#define UONE_GET_ASYMMETRIC_KEY             40      // ms
#define UONE_SET_ASYMMETRIC_KEY             50      // ms

#define UONE_READ_STORAGE                   30      // ms

#define UONE_EDDSA_SIGNATURE                90      // ms
#define UONE_EDDSA_VERIFY                   150     // ms
#define UONE_ECDH_COMPUTTE_SC               150     // ms

#define ERROR_I2C_TRANSPER                  0x40
#define ERROR_RETRY_COUNT                   5
// E #################################################################################


// Define - Buffer Size Info
// S #################################################################################
#define HAL_MAX_RANDOM_SIEZE            1000

#define MALL_SECURE_STORAGE_MAX_SIXE    240
// E #################################################################################


// Define - UONE Instruction Header
// S #################################################################################
#define UONE_HEADER_SECREU_MASSAGE      0x80
#define UONE_HEADER_LAST_MESSAGE        0x40

/* Command Header */
#define PRODUCT_MANAGE_CODE             0x00

#define READ_BINARY                     0x03
#define SET_KEY                         0x05
#define GET_KEY                         0x06

#define GENERATE_KEY_PAIR_X25519        0x0A
#define GENERATE_SECRET_X25519          0x0B
#define GET_PUBLIC_KEY_X25519           0x0C

#define GET_DATA_STATUS                 0x0F

#define GENERATE_RANDOM                 0x11

#define GENERATE_EDDSA_SIGN             0x1A
#define VERIFY_EDDSA_SIGN               0x1B
#define EXPORT_PUBLIC_KEY_ED            0x1D
// E #################################################################################


// Define - UONE Parameter & Key Type & Object ID
// S #################################################################################
/*  Parameter  */
#define GENERATE_KEY_IN_SE              0x80

#define PARAM_GET_DATA_ORDER_DATE       0x00
#define PARAM_GET_DATA_STATUS           0x01
#define PARAM_GET_DATA_CSN              0x02

#define PARAM_ECC_KEY                   0x04

/*  Key Type  */
#define KEY_TYPE_ECC_X25519             0x040030
#define KEY_TYPE_ECC_ED25519            0x050000

/*  Object ID  */
#define OBJECT_ID_ECC_ED25519           0x00
#define OBJECT_ID_ECC_X25519            0x30

/* Curve Index */
#define CURVE_INDEX_ED25519             0x16
#define CURVE_INDEX_X25519              0x30

/* Key SE Internal Index */
#define KEY_INDEX_USE_ODM               0x00
#define KEY_INDEX_USE_STORAGE           0x01
#define KEY_INDEX_ED25519               0x02
// E #################################################################################


// Define - EdDSA Values
// S #################################################################################

// Size Values
#define SECP256_SIZE                32
#define EDDSA_KEY_SIZE              32
#define EDDSA_IN_MSG_MAX_SIZE       255
#define EDDSA_SIGN_ASN1_MAX_SIZE    72
#define ECDH_X25519_SECRET_K_SIZE   32


// Define - Structure

//  @struct sECC_KEY
//  @brief struct of ECC Key Parameter
struct sECC_KEY {
    uint8_t *privateKey;

    uint8_t *publicKey_x;
    uint8_t x_length;

    uint8_t *publicKey_y;
    uint8_t y_length;
};

//  @struct sECC_SIGN
//  @brief  struct of EdDSA Signature
struct sECC_SIGN {

    uint32_t sign_typd;

    uint8_t *r;
    uint8_t r_length;

    uint8_t *s;
    uint8_t s_length;

};

// E #################################################################################



// Define - UONE Error Code
// S #################################################################################
typedef enum {

	UONE_SUCCESS,   // 0x00

    /*  ERROR CODE FROM SE : 0x1X */
    UONE_S_INSTRUCTION_CODE_INVALID = 0x10,  // 0x10
    UONE_S_WRONG_LEGNTH,                     // 0x11
    UONE_S_INCORRECT_PARAMETER,              // 0x12
    UONE_S_SECURITY_STATUS_NOT_SATISFIED,    // 0x13
    UONE_S_CONDITIONS_NOT_SATISFIED,         // 0x14
    UONE_S_FILE_NOT_FOUND,                   // 0x15
    UONE_S_CRC_INVALID,                      // 0x16
    UONE_S_WRONG_DATA,                       // 0x17
    UONE_S_ECC_OPERATION_ERROR,              // 0x18
    UONE_S_OUT_OF_FILE,                      // 0x19
    UONE_S_FILE_NOT_SELECTED,                // 0x1A
    UONE_S_UNKNOWN_ERROR = 0x1F,             // 0x1F

    /* Error Code From Driver = 0x2x */
	UONE_INCORRECT_PARAMETER = 0x20,         // 0x20
	UONE_INCORRECT_KEY_INDEX,                // 0x21
	UONE_CRC_INVALID,                        // 0x22
    UONE_WRONG_LEGNTH,                       // 0x23
	UONE_WRONG_DATA,                         // 0x24
	UONE_ECC_OPERATION_ERROR,                // 0x25
	UONE_ECC_CURVE_NOT_SUPPORTED,            // 0x26
	UONE_ALGORITHM_NOT_SUPPORTED,            // 0x27
	UONE_STORAGE_IS_EMPTY,                   // 0x28
	UONE_CERTIFICATE_IS_EMPTY,               // 0x29

	UONE_UNKNOWN_ERROR = 0x2F,               // 0x2F

    /* Error Code From Semaphore = 0x3x */
    UONE_SEMAPHORE_CREATE_ERROR = 0x30,      // 0x30
    UONE_SEMAPHORE_TAKE_ERROR,               // 0x31
    UONE_SEMAPHORE_GIVE_ERROR,               // 0x32
    UONE_SEMAPHORE_DELETE_ERROR,             // 0x33

    /* Error Code From Driver - Related I2C = 0x4x */
    UONE_ALLOC_MEMORY_ERROR = 0x40,          // 0x40
    UONE_I2C_SETADDR_ERROR,                  // 0x41
    UONE_I2C_INIT_ERROR,                     // 0x42
    UONE_I2C_DEINIT_ERROR,                   // 0x43
    UONE_I2C_STATUS_ERROR,                   // 0x44
    UONE_I2C_BUSY_ERROR,                     // 0x45
    UONE_I2C_INVALID_PORT_NUMBER,            // 0x46
    UONE_I2C_INVALID_PARAM,                  // 0x47
    UONE_I2C_WRITE_ERROR,                    // 0x48
    UONE_I2C_READ_ERROR,                     // 0x49
    UONE_I2C_ALREADY_INITIALIZED,            // 0x4A
    UONE_I2C_ALREADY_DEINITIALIZED,          // 0x4B

} uone_error;
// E #################################################################################


// Define - UONE Driver Receive Offset & Length Info
// S #################################################################################

/* Offset */
#define UONE_RES_OFFSET_HEADER              0
#define UONE_RES_OFFSET_LENGTH_HIGH         1
#define UONE_RES_OFFSET_LENGTH_LOW          2
#define UONE_RES_OFFSET_DATA                3

#define UONE_RES_SERIAL_NUMBER_TAG          1
#define UONE_RES_SERIAL_NUMBER_LENGTH       2
#define UONE_RES_SERIAL_NUMBER_VALUE        3

/* Length */
#define CRC_LEN                             2

#define UONE_RES_HEADER_LEN                 1
#define UONE_RES_HEADER_AND_LENGTH_LEN      3

#define UONE_SE_ORDER_DATA_LENGTH           4
#define UONE_SE_INFO_LENGTH                 34

// E #################################################################################


// Define - UONE Storage Index & Offset
// S #################################################################################

#define SE_UPDATE_READ_BINARY_OFFSET_MSB    0x80

// E #################################################################################


// Functions

/* I2C APIs */
// S ##################################################################
void uone_reset (void);

uint8_t uone_i2c_init (void);

uint8_t uone_i2c_deinit (void);

uint8_t uone_mutex_init(void);

void uone_mutex_deinit(void);

uint8_t i2c_write (uint8_t *data, uint16_t length);

uint8_t i2c_read (uint8_t *data, uint16_t size);
// E - I2C APIs #######################################################


/* Common APIs */
// S ##################################################################
uint8_t uone_get_status (hal_data *outData, uint8_t data_type);

uint8_t uone_get_product_manage_code (hal_data *pmCode);
// E - Common APIs ####################################################


/* Key Management APIs */
// S ##################################################################
uint8_t uone_generate_asymmetric_key (key_type mode, uint16_t index);

uint8_t uone_get_asymmetric_key (key_type mode, uint16_t index, hal_data *key);

uint8_t uone_remove_asymmetric_key(key_type mode, uint16_t index);

uint8_t uone_set_asymmetric_key (key_type mode, uint16_t index, hal_data *pubKey, hal_data *priKey);
// E - Key Management APIs ############################################


/* Random Number Generate APIs */
uint8_t uone_generate_random (hal_data *outData);


/* EdDSA (Edward Curve) APIs */
// S ##################################################################
uint8_t uone_get_serial_number (hal_data *outData);

uint8_t uone_eddsa_signature (struct sECC_SIGN *ecc_sign, hal_data *in_Hash_Data, uint16_t index);

uint8_t uone_eddsa_verify (struct sECC_SIGN *ecc_sign, hal_data *in_data, uint16_t index);

uint8_t uone_ecdh_compute_shared_secret (struct sECC_KEY *ecc_pubicKey, uint16_t priKey_idx, hal_data *shared_secret);
// E - EdDSA (Edward Curve) APIs ######################################


/* Customer APIs */
// S ##################################################################
uint8_t uone_get_Factory_edKey (hal_data *pubKey);
// E - Customer APIs ##################################################


#endif      /* __UONE_CUSTOM_H__ */

/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "security/backend/hardware/se/Hal_uone.h"
#include "security/backend/hardware/se/Hal_uone_utils.h"
#include "security/backend/hardware/se/uone_custom.h"

// DEBUG LEVEL
unsigned char debug_level = 0;


/*
 * Common APIs
*/
// S #################################################################################

uint8_t UB_hal_init (_IN_ uint8_t debug)
{

    uint8_t ret = 0;

    if ( debug <= DEBUG_LEVEL_PROCESS ) {
        debug_level = debug;
    } else {
        return HAL_INVALID_ARGS;
    }

    UONE_DEBUG_ENTER;

    ret = uone_i2c_init();
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( (("\n . uone_i2c_init Failed : SW = 0x%02X\n")), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( ("\n << UONE Driver Version : %06X >>\n"),
                            (unsigned int)uone_driver_version, DEBUG_LEVEL_HAL);
    }

    return HAL_SUCCESS;
    
}   //  End - uint8_t ub_Hal_init()


uint8_t UB_hal_deinit (void)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    ret = uone_i2c_deinit();
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( (("\n . uone_i2c_deinit Failed : SW = 0x%02X\n")), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( (("\n . uone_i2c_deinit Success : SW = 0x%02X\n")), ret, DEBUG_LEVEL_HAL);
    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_deinit()


uint8_t UB_hal_get_SE_OrderDate (_OUT_ hal_data *se_orderDate)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;


    // Buffer Check
    CHECK_PARAM_NULL(se_orderDate, HAL_INVALID_ARGS);
    
    ret = uone_get_status(se_orderDate, PARAM_GET_DATA_ORDER_DATE);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_status(Order Date) Failed : SW = 0x%02X\n"),
                            ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_status(Order Date) Success. : SW = 0x%02X\n"),
                            ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {
            printf("  . SE_Order Date : ");
            HAL_LOG_PRINT(se_orderDate->data, se_orderDate->data_len);
            printf("  . SE_Order Date Length : %ld bytes\n", se_orderDate->data_len);
        }

    }
    
    return HAL_SUCCESS;
    
}   // End - uint8_t UB_hal_get_SE_OrderData()


uint8_t UB_hal_get_SE_Info (_OUT_ hal_data *se_info)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;


    // Buffer Check
    CHECK_PARAM_NULL(se_info, HAL_INVALID_ARGS);
    
    ret = uone_get_status(se_info, PARAM_GET_DATA_STATUS);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_status(STATUS) Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_status(STATUS) Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {
            printf("  . SE_Info : ");
            HAL_LOG_PRINT(se_info->data, se_info->data_len);
            printf("  . SE_Info Length : %ld bytes\n", se_info->data_len);
        }

    }
    
    return HAL_SUCCESS;

}   // End - int hal_get_SE_Info ()


uint8_t UB_hal_get_SE_CSN (_OUT_ hal_data *se_csn)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;


    // Buffer Check
    CHECK_PARAM_NULL(se_csn, HAL_INVALID_ARGS);
    
    ret = uone_get_status(se_csn, PARAM_GET_DATA_CSN);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_status(CSN) Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_status(CSN) Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {
            printf("  . SE_CSN : ");
            HAL_LOG_PRINT(se_csn->data, se_csn->data_len);
            printf("  . SE_CSN Length : %ld bytes\n", se_csn->data_len);
        }

    }
    
    return HAL_SUCCESS;

}   // End - int hal_get_SE_CSN ()


uint8_t UB_hal_get_product_code (_OUT_ hal_data *se_product_code)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;


    // Buffer Check
    CHECK_PARAM_NULL(se_product_code, HAL_INVALID_ARGS);

    ret = uone_get_product_manage_code(se_product_code);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_product_manage_code Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_product_manage_code Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {
            printf("  . SE Product Manage Code : ");
            HAL_LOG_PRINT(se_product_code->data, se_product_code->data_len);
            printf("  . SE PM Length : %ld bytes\n", se_product_code->data_len);
        }

    }

    return HAL_SUCCESS;

}

// E Common APIs #####################################################################


/*
 *  Random Number Generator API
*/
uint8_t UB_hal_generate_random (_IN_ uint16_t length, _OUT_ hal_data *random)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    // Buffer Check
    CHECK_PARAM_NULL(random, HAL_INVALID_ARGS);

    // Length Check
    if ( length > HAL_MAX_RANDOM_SIEZE ) {
        return HAL_INVALID_LENGTH;
    }

    random->data_len = length;
    ret = uone_generate_random(random);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_generate_random Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( ("\n\n. uone_generate_random Success : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_generate_random


/*
 *  Key Management APIs
*/
// S #################################################################################

uint8_t UB_hal_generate_key (_IN_ uint16_t key_idx)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    ret = uone_generate_asymmetric_key(KEY_ECC_EDWARD_X25519, key_idx);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_generate_asymmetric_key Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( ("\n\n. uone_generate_asymmetric_key Success : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_generate_key()


uint8_t UB_hal_get_key (_IN_ hal_key_type mode, _IN_ uint16_t key_idx, _OUT_ hal_data *key)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    // Buffer Check
    CHECK_PARAM_NULL(key, HAL_INVALID_ARGS);

    switch (mode) {

        case HAL_KEY_ECC_EDWARD_ED25519 : 
        case HAL_KEY_ECC_EDWARD_X25519 :
            {
                ret = uone_get_asymmetric_key(mode, key_idx, key);
            }
            break;

        default :
            return HAL_NOT_SUPPORTED;

    }   // End - switch (mode)

    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_asymmetric_key Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_asymmetric_key Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {

            printf("  . Get Ed25519 Public Key : ");
            HAL_LOG_PRINT(key->data, key->data_len);
            printf("  . Public Key Length : %ld bytes\n", key->data_len);

        }

    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_get_key()


uint8_t UB_hal_remove_key (_IN_ hal_key_type mode, _IN_ uint16_t key_idx)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    switch (mode) {

        case HAL_KEY_ECC_EDWARD_ED25519 :
        case HAL_KEY_ECC_EDWARD_X25519 :
            {
                ret = uone_remove_asymmetric_key(mode, key_idx);
            }
            break;

        default :
            return HAL_NOT_SUPPORTED;
    }

    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_remove_asymmetric_key Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( ("\n\n. uone_remove_asymmetric_key Success : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
    }

    return HAL_SUCCESS;

}   // End - uin8_t UB_hal_remove_key()


uint8_t UB_hal_set_key (_IN_ hal_key_type mode, _IN_ uint16_t key_idx, _IN_ hal_data *pubKey, _IN_ hal_data *priKey)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;


    // Key Parameter Parsing
    switch (mode) {

        case HAL_KEY_ECC_EDWARD_ED25519 :       // EdDSA Sign / Verify - Ed25519
        case HAL_KEY_ECC_EDWARD_X25519 :        // ECDHE - X25519            
            {

                if ( pubKey == NULL ) {     // Just set Private Key only Case. (Public Key is NULL)
                    ret = uone_set_asymmetric_key(mode, key_idx, NULL, priKey);
                } else if ( priKey == NULL ) {  // Just set Public Key only Case. (Private Key is NULL)
                    ret = uone_set_asymmetric_key(mode, key_idx, pubKey, NULL);
                } else {    // Set Private/Public Key Pair Case.
                    ret = uone_set_asymmetric_key(mode, key_idx, pubKey, priKey);
                }

            }
            break;

        default :
            return HAL_NOT_SUPPORTED;

    }   // End - switch (mode)

    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_set_asymmetric_key Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( ("\n\n. uone_set_asymmetric_key Success : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_set_key

// E Key Related APIs ################################################################


/*
 *  EdDSA - Edward Curve APIs
*/
// S #################################################################################

uint8_t UB_hal_get_serial_number (_OUT_ hal_data *se_SN)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;


    // Buffer Check
    CHECK_PARAM_NULL(se_SN, HAL_INVALID_ARGS);

    ret = uone_get_serial_number(se_SN);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_serial_number Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_serial_number Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {

            printf("  . SE_Serial Number : ");
            HAL_LOG_PRINT(se_SN->data, se_SN->data_len);
            printf("  . se_SN Length : %ld bytes\n", se_SN->data_len);

        }

    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_get_serial_number ()


uint8_t UB_hal_eddsa_sign (_IN_ hal_data *in_Msg, _IN_ uint16_t key_idx, _OUT_ hal_data *sign)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    uint8_t sign_r_Buff[HAL_SIZE_EDDSA_SIGN_BUFFER] = { 0 };
    uint8_t sign_s_Buff[HAL_SIZE_EDDSA_SIGN_BUFFER] = { 0 };

    struct sECC_SIGN ecc_sign = { 0x00 /* sign_type */,
                                  sign_r_Buff /* sign r */, HAL_SIZE_EDDSA_SIGN_BUFFER /* r Length */,
                                  sign_s_Buff /* sign s */, HAL_SIZE_EDDSA_SIGN_BUFFER /* s Length */ };


    // Buffer Check
    CHECK_PARAM_NULL(in_Msg, HAL_INVALID_ARGS);
    CHECK_PARAM_NULL(sign, HAL_INVALID_ARGS);

    // Message Length Check
    if ( in_Msg->data_len > EDDSA_IN_MSG_MAX_SIZE ) {
        return HAL_INVALID_LENGTH;
    }

    ret = uone_eddsa_signature(&ecc_sign, in_Msg, key_idx);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_eddsa_signature Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_eddsa_signature Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {

            printf("  . Generated Signature r : ");
            HAL_LOG_PRINT(ecc_sign.r, ecc_sign.r_length);
            printf("  . Sign r Length : %d bytes\n", ecc_sign.r_length);

            printf("  . Generated Signature s : ");
            HAL_LOG_PRINT(ecc_sign.s, ecc_sign.s_length);
            printf("  . Sign s Length : %d bytes\n", ecc_sign.s_length);

        }

    }
    memcpy(sign->data, ecc_sign.r, ecc_sign.r_length);
    memcpy(sign->data + ecc_sign.r_length, ecc_sign.s, ecc_sign.s_length);
    sign->data_len = ecc_sign.r_length + ecc_sign.s_length;

    //Signature Encoding Process
//     hal_mpi r, s;
//     hal_mpi_init(&r);
//     hal_mpi_init(&s);

//     ret = hal_mpi_read_binary(&r, ecc_sign.r, ecc_sign.r_length);
//     if ( ret != HAL_SUCCESS ) {
//         goto cleanup;
//     }

// 	ret = hal_mpi_read_binary(&s, ecc_sign.s, ecc_sign.s_length);
// 	if ( ret != 0 ) {
// 		goto cleanup;
// 	}

//     hal_signature_to_asn1(&r, &s, sign->data, &sign->data_len);

//     hal_mpi_free(&r);
//     hal_mpi_free(&s);


//     if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {

//         printf("\n  . Signature ASN.1 Format : ");
//         HAL_LOG_PRINT(sign->data, sign->data_len);
//         printf("  . ASN.1 Sign Length : %ld bytes\n", sign->data_len);

//     }

    return HAL_SUCCESS;


// cleanup :
//     hal_mpi_free(&r);
//     hal_mpi_free(&s);

   return HAL_FAIL;

}   //  End - uint8_t UB_hal_eddsa_sign()


uint8_t UB_hal_eddsa_verify (_IN_ hal_data *in_Msg, _IN_ hal_data *sign, _IN_ uint16_t key_idx)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    // unsigned char *p = (unsigned char *)sign->data;
    // const unsigned char *end = sign->data + sign->data_len;

    // uint32_t len = 0;
    
    // hal_mpi r, s;

    struct sECC_SIGN ecc_sign;

    uint8_t slot = (key_idx & 0xFF);

    // Buffer Check
    CHECK_PARAM_NULL(in_Msg, HAL_INVALID_ARGS);
    CHECK_PARAM_NULL(sign, HAL_INVALID_ARGS);
    
    // Length Check
    if ( in_Msg->data_len > EDDSA_IN_MSG_MAX_SIZE ) {
        return HAL_INVALID_LENGTH;
    }

    // Key Index Arange Check
    if ( !((slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM)) ) {
        return HAL_INVALID_SLOT;
    }

    memset(&ecc_sign, 0x00, sizeof(struct sECC_SIGN));

    // hal_mpi_init(&r);
    // hal_mpi_init(&s);

    // if ( (ret = hal_asn1_get_tag(&p, end, &len, 0x20 | 0x10)) != 0 ) {
    //     ret = HAL_INVALID_ARGS;
    //     goto cleanup;
    // }

    // if ( (p + len) != end ) {
    //     ret = HAL_INVALID_ARGS;
    //     goto cleanup;
    // }

    // if ( ((ret = hal_asn1_get_mpi(&p, end, &r)) != 0) || ((ret = hal_asn1_get_mpi(&p, end, &s)) != 0) ) {
    //     ret = HAL_INVALID_ARGS;
    //     goto cleanup;
    // }

    // ecc_sign.r_length = hal_mpi_size(&r);
    // ecc_sign.s_length = hal_mpi_size(&s);

    ecc_sign.r_length = sign->data_len / 2;
    ecc_sign.s_length = ecc_sign.r_length;

    ecc_sign.r = (unsigned char *)malloc(ecc_sign.r_length);
    if ( ecc_sign.r == NULL ) {
        ret = HAL_ALLOC_FAIL;
        goto cleanup;
    }
    
    ecc_sign.s = (unsigned char *)malloc(ecc_sign.s_length);
    if ( ecc_sign.s == NULL ) {
        ret = HAL_ALLOC_FAIL;
        goto cleanup;
    }

    // HAL_MPI_CHK(hal_mpi_write_binary(&r, ecc_sign.r, ecc_sign.r_length));
    // HAL_MPI_CHK(hal_mpi_write_binary(&s, ecc_sign.s, ecc_sign.s_length));
    memcpy(ecc_sign.r, sign->data, ecc_sign.r_length);
    memcpy(ecc_sign.s, sign->data + ecc_sign.r_length, ecc_sign.s_length);

    ret = uone_eddsa_verify(&ecc_sign, in_Msg, key_idx);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_eddsa_verify Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {
        UONE_DEBUG_LOG( ("\n\n. uone_eddsa_verify Success : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        ret = HAL_SUCCESS;
    }


    return ret;


cleanup :
    
    // hal_mpi_free(&r);
    // hal_mpi_free(&s);

    if ( ecc_sign.r ) {
        free(ecc_sign.r);
    }
    if ( ecc_sign.s ) {
        free(ecc_sign.s);
    }

    return ret;

}   // End - uint8_t UB_hal_eddsa_verify()


uint8_t UB_hal_ecdh_compute_shared_secret (_IN_ hal_data *ecdh_pubKey, _IN_ uint16_t priKey_idx, _OUT_ hal_data *shared_secret)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;
    struct sECC_KEY ecc_pub;

    CHECK_PARAM_NULL(ecdh_pubKey, HAL_INVALID_ARGS);
    CHECK_PARAM_NULL(shared_secret, HAL_INVALID_ARGS);

    memset(&ecc_pub, 0x00, sizeof(struct sECC_KEY));

    // Key Set
    ecc_pub.publicKey_x = ecdh_pubKey->data;
    ecc_pub.x_length = ecdh_pubKey->data_len;

    ret = uone_ecdh_compute_shared_secret(&ecc_pub, priKey_idx, shared_secret);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_ecdh_compute_shared_secret Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_ecdh_compute_shared_secret Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {

            printf("  . Generated Shared Secret : ");
            HAL_LOG_PRINT(shared_secret->data, shared_secret->data_len);
            printf("  . Shared Secret Length : %ld bytes\n", shared_secret->data_len);

        }

    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_ecdh_compute_shared_secret()


uint8_t UB_hal_get_Factory_Key (_OUT_ hal_data *pubKey)
{

    UONE_DEBUG_ENTER;

    uint8_t ret = 0;

    // Buffer Check
    CHECK_PARAM_NULL(pubKey, HAL_INVALID_ARGS);

    ret = uone_get_Factory_edKey(pubKey);
    if ( ret != UONE_SUCCESS ) {
        UONE_DEBUG_LOG( ("\n\n. uone_get_Factory_edKey Failed : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);
        return ret;
    } else {

        UONE_DEBUG_LOG( ("\n\n. uone_get_Factory_edKey Success. : SW = 0x%02X\n"), ret, DEBUG_LEVEL_HAL);

        if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_HAL) ) {

            printf("  . Get Factory Public Key (Ed25519) : ");
            HAL_LOG_PRINT(pubKey->data, pubKey->data_len);
            printf("  . Public Key Length : %ld bytes\n", pubKey->data_len);

        }

    }

    return HAL_SUCCESS;

}   // End - uint8_t UB_hal_get_Factory_Key(


// E EdDSA - Edward Curve APIs #######################################################

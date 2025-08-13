/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/

#ifndef __HAL_UONE_H__
#define __HAL_UONE_H__


#include "security/backend/hardware/se/Hal_uone_custom.h"


#define _IN_
#define _OUT_


#define CHECK_PARAM_NULL(ptr, err_code)     \
    do {    \
        if ( (ptr) == NULL ) {  \
            return (err_code);  \
        }   \
    } while (0)


#define HAL_LOG_PRINT(data, data_len)       \
    do {        \
        for ( uint16_t i = 0; i < (data_len); i++ ) {       \
            printf("%02X", (data)[i]);      \
        }       \
        printf("\n");       \
    } while (0)
        

/* Common APIs */
// S #################################################################################
uint8_t UB_hal_init (_IN_ uint8_t debug);

uint8_t UB_hal_deinit (void);


uint8_t UB_hal_get_SE_OrderDate (_OUT_ hal_data *se_orderDate);

uint8_t UB_hal_get_SE_Info (_OUT_ hal_data *se_info);

uint8_t UB_hal_get_SE_CSN (_OUT_ hal_data *se_csn);

uint8_t UB_hal_get_product_code (_OUT_ hal_data *se_product_code);
// E - Common APIs ###################################################################


/* Random Number Generate APIs */
uint8_t UB_hal_generate_random (_IN_ uint16_t length, _OUT_ hal_data *random);


/* Key Management APIs */
// S #################################################################################
uint8_t UB_hal_generate_key (_IN_ uint16_t key_idx);

uint8_t UB_hal_get_key (_IN_ hal_key_type mode, _IN_ uint16_t key_idx, _OUT_ hal_data *key);

uint8_t UB_hal_remove_key (_IN_ hal_key_type mode, _IN_ uint16_t key_idx);

uint8_t UB_hal_set_key (_IN_ hal_key_type mode, _IN_ uint16_t key_idx, _IN_ hal_data *pubKey, _IN_ hal_data *priKey);
// E - Key Related APIs ##############################################################


/* EdDSA APIs*/
// S #################################################################################
uint8_t UB_hal_get_serial_number (_OUT_ hal_data *se_SN);

uint8_t UB_hal_eddsa_sign (_IN_ hal_data *in_Msg, _IN_ uint16_t key_idx, _OUT_ hal_data *sign);

uint8_t UB_hal_eddsa_verify (_IN_ hal_data *in_Msg, _IN_ hal_data *sign, _IN_ uint16_t key_idx);

uint8_t UB_hal_ecdh_compute_shared_secret (_IN_ hal_data *ecdh_pubKey, _IN_ uint16_t priKey_idx, _OUT_ hal_data *shared_secret);
// E - EdDSA APIs ####################################################################


/* Customer APIs */
// S #################################################################################
uint8_t UB_hal_get_Factory_Key(_OUT_ hal_data *pubKey);
// E - Customer APIs #################################################################

#endif      // __HAL_UONE_H__

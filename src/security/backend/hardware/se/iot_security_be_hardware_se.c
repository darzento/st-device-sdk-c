/* ***************************************************************************
 *
 * Copyright (c) 2022 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

#include <stdio.h>
#include "iot_debug.h"
#include "security/backend/hardware/se/iot_security_be_hardware_se.h"
#include "security/backend/hardware/se/Hal_uone.h"
#include "security/backend/hardware/se/Hal_uone_custom.h"
#include "security/backend/hardware/se/uone_custom.h"
#include "security/backend/hardware/se/uone_utils.h"
#include "security/backend/hardware/se/uone_type.h"
#include "security/backend/iot_security_be.h"
#include "security/iot_security_error.h"
#include "security/iot_security_storage.h"
#include "security/iot_security_crypto.h"


#define BUFFER_ALLOC_MARGIN		10

// #define TEST

typedef struct iot_security_be_cert2storage_id_map {
	iot_security_cert_id_t cert_id;
	iot_security_storage_id_t storage_id;
} iot_security_be_cert2storage_id_map_t;

static const iot_security_be_cert2storage_id_map_t cert2storage_id_map[] = {
	{ IOT_SECURITY_CERT_ID_ROOT_CA, IOT_NVD_ROOT_CA_CERT },
	{ IOT_SECURITY_CERT_ID_SUB_CA,  IOT_NVD_SUB_CA_CERT },
	{ IOT_SECURITY_CERT_ID_DEVICE,  IOT_NVD_DEVICE_CERT },
};

static inline iot_security_storage_id_t _iot_security_be_software_id_cert2storage(iot_security_cert_id_t cert_id)
{
	iot_security_storage_id_t storage_id;
	const iot_security_be_cert2storage_id_map_t *c2s_id_map_list = cert2storage_id_map;
	int c2s_id_map_list_len = sizeof(cert2storage_id_map) / sizeof(cert2storage_id_map[0]);
	int i;

	IOT_DEBUG("cert id = %d", cert_id);

	for (i = 0; i < c2s_id_map_list_len; i++) {
		if (cert_id == c2s_id_map_list[i].cert_id) {
			storage_id = c2s_id_map_list[i].storage_id;
			IOT_DEBUG("storage id = %d", storage_id);
			return storage_id;
		}
	}

	IOT_ERROR("'%d' is not a supported cert id", cert_id);

	return IOT_NVD_UNKNOWN;
}

iot_error_t iot_security_be_hardware_se_init(void)
{
    uint8_t ret = 0;

    ret = UB_hal_init(DEBUG_LEVEL_NO_DEBUG);
    if ((ret == HAL_SUCCESS) || (ret == UONE_I2C_ALREADY_INITIALIZED)) {
        return IOT_ERROR_NONE;
    }
    return IOT_ERROR_SECURITY_STORAGE_INIT;
}

iot_error_t iot_security_be_hardware_se_deinit(void)
{
    uint8_t ret = 0;

    ret = UB_hal_deinit();
    if ((ret == HAL_SUCCESS) || (ret == UONE_I2C_ALREADY_DEINITIALIZED)){
        return IOT_ERROR_NONE;    
    }
    return IOT_ERROR_SECURITY_STORAGE_DEINIT;
}

iot_error_t iot_security_be_hardware_se_pk_load(iot_security_context_t *context)
{
    uint8_t ret = 0;
    uint8_t *pubkey = (uint8_t*)malloc(EDDSA_KEY_SIZE + BUFFER_ALLOC_MARGIN);
    hal_data pub_key = {pubkey, 0};

    ret = UB_hal_get_Factory_Key(&pub_key);
    if ( ret != HAL_SUCCESS ) {
        IOT_ERROR("fail get public key");
		return IOT_ERROR_SECURITY_KEY_NOT_FOUND;
    }
    
    context->pk_params->type = IOT_SECURITY_KEY_TYPE_ED25519;
    context->pk_params->pk_sign_type = IOT_SECURITY_PK_SIGN_TYPE_RAW;
    context->pk_params->pubkey.len = pub_key.data_len;
    context->pk_params->pubkey.p = pub_key.data;

    return IOT_ERROR_NONE;
}

/**
 * @brief	Calculate a signature
 * @details	Calculate 64bytes raw type ecdsa signature using device private key
 * @param[in]	context reference to the security context
 * @param[in]	input_buf a pointer to a buffer to data for signature
 * @param[out]	sig_buf a pointer to a buffer to store the signature
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_pk_sign(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *sig_buf)
{
    uint8_t ret = 0;
    uint16_t key_index = 0;
    uint8_t* expected_sign = (uint8_t*)malloc(EDDSA_SIGN_ASN1_MAX_SIZE + BUFFER_ALLOC_MARGIN);
    // IOT_ERROR("message length : %d", input_buf->len);
    // IOT_ERROR("message : %s", input_buf->p);
    hal_data message = {input_buf->p, input_buf->len};
    hal_data sign = {expected_sign, EDDSA_SIGN_ASN1_MAX_SIZE + BUFFER_ALLOC_MARGIN};
    make_Key_index(&key_index, SUB_TYPE_PRIVATE_KEY, SLOT_ODM_ED25519);
    ret = UB_hal_eddsa_sign(&message, key_index, &sign);

    sig_buf->len = sign.data_len;
    sig_buf->p = sign.data;
    if (ret != HAL_SUCCESS) {
        return IOT_ERROR_SECURITY_PK_SIGN;
    }

    return IOT_ERROR_NONE;
}

/**
 * @brief	Verify the signature
 * @details	Verify the signature for input_buf with sig_buf
 * @param[in]	context reference to the security context
 * @param[in]	input_buf a pointer to a buffer to data for signature
 * @param[in]	sig_buf a pointer to a buffer to the signature
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_pk_verify(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *sig_buf)
{
    uint8_t ret = 0;
    uint16_t key_index = 0;

    hal_data message = {
        .data = input_buf->p, 
        .data_len = input_buf->len,
    };
    hal_data sig = {
        .data = sig_buf->p, 
        .data_len = sig_buf->len,
    };

    make_Key_index(&key_index, SUB_TYPE_PUBLIC_KEY, SLOT_ODM_ED25519);
    ret = UB_hal_eddsa_verify(&message, &sig, key_index);
    if (ret != HAL_SUCCESS) {
        return IOT_ERROR_SECURITY_PK_VERIFY;
    }

    return IOT_ERROR_NONE;
}

/**
 * @brief	Compute a shared secret
 * @details	Compute a shared secret with peer public key
 * @param[in]	context reference to the security context
 * @param[in]	input_buf a pointer to a buffer to peer public key(65bytes raw type)
 * @param[out]	output_buf a pointer to a buffer to store the shared secret
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_ecdh_compute_shared_secret(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *output_buf)
{
    uint16_t key_index = 0;
    uint8_t ret = 0;

    uint8_t* shared_key = (uint8_t*) malloc(ECDH_X25519_SECRET_K_SIZE + BUFFER_ALLOC_MARGIN);

    hal_data pub_peer = {
        .data = input_buf->p,
        .data_len = input_buf->len,
    };

    hal_data gen_shared_secret = {
        .data = shared_key,
        .data_len = 0,
    };

    make_Key_index(&key_index, SUB_TYPE_PRIVATE_KEY, SLOT_ODM_ED25519);

    ret = UB_hal_ecdh_compute_shared_secret(&pub_peer, key_index, &gen_shared_secret);

    output_buf->len = gen_shared_secret.data_len;
    output_buf->p = gen_shared_secret.data;

    if (ret != HAL_SUCCESS) {
        return IOT_ERROR_SECURITY_ECDH_SHARED_SECRET;
    }

    return IOT_ERROR_NONE;
}

/**
 * @brief	Generate a ephemeral key pair
 * @details	Generate a key pair based on elliptic curve
 * @param[in]	context reference to the security context
 * @param[in]	key_id key identity to specific a ephemeral key pair
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_manager_generate_key(iot_security_context_t *context, iot_security_key_id_t key_id)
{
    uint16_t key_index = 0;
    uint8_t ret = 0;

    make_Key_index(&key_index, SUB_TYPE_KEY_PAIR, SLOT_EP_KEY_0_NVM);

    ret = UB_hal_generate_key(key_index);
    if (ret != HAL_SUCCESS) {
        return IOT_ERROR_SECURITY_MANAGER_KEY_SET;
    }

    return IOT_ERROR_NONE;
}

/**
 * @brief	Remove the generated ephemeral key pair
 * @param[in]	context reference to the security context
 * @param[in]	key_id key identity to specific a ephemeral key pair
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_manager_remove_key(iot_security_context_t *context, iot_security_key_id_t key_id)
{
    if (key_id == IOT_SECURITY_KEY_ID_EPHEMERAL) {
        uint16_t key_index = 0;
        uint8_t ret = 0;

        make_Key_index(&key_index, SUB_TYPE_KEY_PAIR, SLOT_EP_KEY_0_NVM);

        ret = UB_hal_remove_key(HAL_KEY_ECC_EDWARD_ED25519, key_index);
        if (ret != HAL_SUCCESS) {
            return IOT_ERROR_SECURITY_MANAGER_KEY_REMOVE;
        }

    } else if (key_id == IOT_SECURITY_KEY_ID_SHARED_SECRET) {
        
    }

    return IOT_ERROR_NONE;
}

/**
 * @brief	Set the key for signature or encryption
 * @details	Set the key for signature or encryption operation
 * @param[in]	context reference to the security context
 * @param[in]	key_id key identity want to get
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_manager_set_key(iot_security_context_t *context, iot_security_key_id_t key_id)
{
    return IOT_ERROR_NONE;
}

/**
 * @brief	Get the key for signature or encryption
 * @details	Get the parameter required for signature or encryption operation
 * @param[in]	context reference to the security context
 * @param[in]	key_id key identity want to get
 * @param[out]	key_buf a pointer to a buffer to store the key (64bytes raw type, need to fix 65bytes)
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_manager_get_key(iot_security_context_t *context, iot_security_key_id_t key_id, iot_security_buffer_t *key_buf)
{
    uint16_t key_index = 0;
    uint8_t ret = 0;
    uint8_t* buff = (uint8_t*)malloc(EDDSA_KEY_SIZE + BUFFER_ALLOC_MARGIN);

    hal_data get_pubkey = {
        .data = buff,
        .data_len = 0,
    };

    make_Key_index(&key_index, SUB_TYPE_PUBLIC_KEY, SLOT_EP_KEY_0_RAM);

    ret = UB_hal_get_key(HAL_KEY_ECC_EDWARD_ED25519, key_index, &get_pubkey);
    
    key_buf->p = get_pubkey.data;
    key_buf->len = get_pubkey.data_len;

    if (ret != HAL_SUCCESS) {
        return IOT_ERROR_SECURITY_MANAGER_KEY_GET;
    }

    return IOT_ERROR_NONE;
}

/**
 * @brief	Get the certificate
 * @details	Get the certificate from static, factory or eSE
 * @param[in]	context reference to the security context
 * @param[in]	cert_id certificate identity want to get
 * @param[out]	cert_buf a pointer to a buffer to store the raw type certificate
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_manager_get_certificate(iot_security_context_t *context, iot_security_cert_id_t cert_id, iot_security_buffer_t *cert_buf)
{
    iot_error_t err;
	iot_security_storage_id_t storage_id;

	storage_id = _iot_security_be_software_id_cert2storage(cert_id);
	if (storage_id == IOT_NVD_UNKNOWN) {
		IOT_ERROR_DUMP_AND_RETURN(CERT_INVALID_ID, cert_id);
	}

    if (!context->be_context->bsp_fn ||
        !context->be_context->bsp_fn->bsp_fs_store) {
        IOT_ERROR_DUMP_AND_RETURN(BSP_FN_STORE_NULL, 0);
    }

    err = context->be_context->bsp_fn->bsp_fs_load(context->be_context, storage_id, cert_buf);

	if (err) {
		return err;
	}

	return err;
}

/**
 * @brief	Read data from storage
 * @details	a pointer to a function to read the data from storage
 * @param[in]	context reference to the security context
 * @param[in]	storage_id file identity of target to read
 * @param[out]	data_buf a pointer to a security buffer for read data
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_storage_read(iot_security_context_t *context, iot_security_storage_id_t storage_id, iot_security_buffer_t *data_buf)
{
    if (storage_id == IOT_NVD_SERIAL_NUM) {
        uint8_t ret = 0;
        uint8_t* serial_number = (uint8_t*)malloc(30);
        hal_data se_serial_number = {
            .data = serial_number,
            .data_len = 0,
        };
        ret = UB_hal_get_serial_number(&se_serial_number);
#ifdef TEST
        uint8_t* temp_serial_number = (uint8_t*)malloc(30);
        temp_serial_number[0] = 'S';
        temp_serial_number[1] = 'T';
        temp_serial_number[2] = 'D';
        temp_serial_number[3] = 'K';

        for (uint8_t i = 4; i < se_serial_number.data_len + 4; i++) {
            temp_serial_number[i] = se_serial_number.data[i - 4];
        }
        se_serial_number.data = temp_serial_number;
        se_serial_number.data_len = se_serial_number.data_len + 4;
        free(serial_number);
#endif
        data_buf->len = se_serial_number.data_len;
        data_buf->p = se_serial_number.data;

        if (ret != HAL_SUCCESS) {
            return IOT_ERROR_SECURITY_STORAGE_READ;
        }
    } else if (storage_id == IOT_NVD_PUBLIC_KEY) {
        uint8_t ret = 0;
        uint8_t* factory_pub_key;
        factory_pub_key = malloc(EDDSA_KEY_SIZE + BUFFER_ALLOC_MARGIN);

        hal_data pub_key = {
            .data = factory_pub_key,
            .data_len = 0,
        };
        ret = UB_hal_get_Factory_Key(&pub_key);
        
        data_buf->len = pub_key.data_len;
        data_buf->p = pub_key.data;

        if (ret != HAL_SUCCESS) {
            return IOT_ERROR_SECURITY_STORAGE_READ;
        }
    } else {
        iot_error_t err;
        if (!context->be_context->bsp_fn ||
            !context->be_context->bsp_fn->bsp_fs_store) {
            IOT_ERROR_DUMP_AND_RETURN(BSP_FN_STORE_NULL, 0);
        }
        err = context->be_context->bsp_fn->bsp_fs_load(context->be_context, storage_id, data_buf);

        if (err) {
            return err;
        }
    }
    return IOT_ERROR_NONE;
}

/**
 * @brief	Write data to storage
 * @details	a pointer to a function to write the data to storage
 * @param[in]	context reference to the security context
 * @param[in]	storage_id file identity of target to write
 * @param[in]	data_buf a pointer to a security buffer for write data
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_storage_write(iot_security_context_t *context, iot_security_storage_id_t storage_id, iot_security_buffer_t *data_buf)
{
    iot_error_t err;
    if (!context->be_context->bsp_fn ||
        !context->be_context->bsp_fn->bsp_fs_store) {
        IOT_ERROR_DUMP_AND_RETURN(BSP_FN_STORE_NULL, 0);
    }

    if (storage_id == IOT_NVD_SERIAL_NUM) {
        return IOT_ERROR_NONE;
    } else if (storage_id == IOT_NVD_PRIVATE_KEY) {
        return IOT_ERROR_NONE;
    } else if (storage_id == IOT_NVD_PUBLIC_KEY) {
        return IOT_ERROR_NONE;
    }

    err = context->be_context->bsp_fn->bsp_fs_store(context->be_context, storage_id, data_buf);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

/**
 * @brief	Remove data from storage
 * @details	a pointer to a function to remove the data from storage
 * @param[in]	context reference to the security context
 * @param[in]	storage_id file identity of target to remove
 * @retval	IOT_ERROR_NONE success
 */
iot_error_t iot_security_be_hardware_se_storage_remove(iot_security_context_t *context, iot_security_storage_id_t storage_id)
{
    iot_error_t err;
    if (!context->be_context->bsp_fn ||
		!context->be_context->bsp_fn->bsp_fs_remove) 
    {
		IOT_ERROR_DUMP_AND_RETURN(BSP_FN_REMOVE_NULL, 0);
	}

    if (storage_id == IOT_NVD_SERIAL_NUM) {
        return IOT_ERROR_NONE;
    } else if (storage_id == IOT_NVD_PRIVATE_KEY) {
        return IOT_ERROR_NONE;
    } else if (storage_id == IOT_NVD_PUBLIC_KEY) {
        return IOT_ERROR_NONE;
    }

	err = context->be_context->bsp_fn->bsp_fs_remove(context->be_context, storage_id);
	if (err) {
		return err;
	}

    return IOT_ERROR_NONE;
}

/**
 * @brief Generates a user-specified number of random bytes and returns it in a new buffer.
 * @details	Supports random number based on True Random Number Generator.
 * @param [in]  len the number of random bytes
 * @param [out] random a generated random bytes
 * @return IOT_ERROR_NONE if successful
 */
iot_error_t iot_security_be_hardware_se_generate_random(unsigned int len, unsigned char *out_random)
{
    uint8_t ret = 0;
    uint8_t* data_buf = (uint8_t*)malloc(32);
    uint16_t data_len = (uint16_t)len;

    hal_data random = {data_buf, 0};

    ret = UB_hal_generate_random(data_len, &random);
    
    memcpy(out_random, random.data, len);

    if (ret != HAL_SUCCESS) {
        return IOT_ERROR_INIT_FAIL;
    }

    return ret;
}
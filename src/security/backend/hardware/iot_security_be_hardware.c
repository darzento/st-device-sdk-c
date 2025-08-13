/* ***************************************************************************
 *
 * Copyright (c) 2020 Samsung Electronics All Rights Reserved.
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

#include <string.h>

#include "iot_main.h"
#include "iot_debug.h"

#include "port_crypto.h"
#include "security/iot_security_crypto.h"
#include "security/iot_security_ecdh.h"
#include "security/iot_security_manager.h"
#include "security/iot_security_storage.h"
#include "security/backend/iot_security_be.h"
#include "security/iot_security_util.h"
#include "security/backend/hardware/se/iot_security_be_hardware_se.h"

STATIC_FUNCTION
iot_error_t _iot_security_be_check_context_and_params_is_valid(iot_security_context_t *context, iot_security_sub_system_t sub_system)
{
	if (!context) {
		IOT_ERROR("context is null");
		IOT_ERROR_DUMP_AND_RETURN(CONTEXT_NULL, 0);
	}

	if (sub_system & IOT_SECURITY_SUB_PK) {
		if (!context->pk_params) {
			IOT_ERROR("pk params is null");
			IOT_ERROR_DUMP_AND_RETURN(PK_PARAMS_NULL, 0);
		}
	}

	if (sub_system & IOT_SECURITY_SUB_CIPHER) {
		if (!context->cipher_params) {
			IOT_ERROR("cipher params is null");
			IOT_ERROR_DUMP_AND_RETURN(CIPHER_PARAMS_NULL, 0);
		}
	}

	if (sub_system & IOT_SECURITY_SUB_ECDH) {
		if (!context->ecdh_params) {
			IOT_ERROR("ecdh params is null");
			IOT_ERROR_DUMP_AND_RETURN(ECDH_PARAMS_NULL, 0);
		}
	}

	if (sub_system & IOT_SECURITY_SUB_STORAGE) {
		if (!context->storage_params) {
			IOT_ERROR("storage params is null");
			IOT_ERROR_DUMP_AND_RETURN(STORAGE_PARAMS_NULL, 0);
		}
	}

	return IOT_ERROR_NONE;
}

static inline void _iot_security_be_hardware_buffer_free(iot_security_buffer_t *buffer)
{
	if (buffer) {
		if (buffer->p && buffer->len) {
			memset(buffer->p, 0, buffer->len);
			iot_os_free(buffer->p);
		}
		memset(buffer, 0, sizeof(iot_security_buffer_t));
	}
}

static inline void _iot_security_be_hardware_buffer_wipe(const iot_security_buffer_t *input_buf, size_t wiped_len)
{
	if (input_buf && (input_buf->len < wiped_len)) {
		int i;
		for (i = input_buf->len; i < wiped_len; i++) {
			input_buf->p[i] = 0;
		}
	}
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_pk_init(iot_security_context_t *context)
{
	iot_error_t err;

	if (!context) {
		IOT_ERROR_DUMP_AND_RETURN(CONTEXT_NULL, 0);
	}

	err = iot_security_be_hardware_se_pk_load(context);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_pk_deinit(iot_security_context_t *context)
{
	iot_error_t err;
	iot_security_pk_params_t *pk_params;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_PK);
	if (err) {
		return err;
	}

	pk_params = context->pk_params;

	if (pk_params->pubkey.p) {
		_iot_security_be_hardware_buffer_free(&pk_params->pubkey);
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_pk_get_key_type(iot_security_context_t *context, iot_security_key_type_t *key_type)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_PK);
	if (err) {
		return err;
	}

	*key_type = context->pk_params->type;

	IOT_DEBUG("type = %d", *key_type);

	return IOT_ERROR_NONE;
}

#if defined(CONFIG_STDK_IOT_CORE_CRYPTO_SUPPORT_ECDSA)
STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_pk_set_sign_type(iot_security_context_t *context, iot_security_pk_sign_type_t pk_sign_type)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_PK);
	if (err) {
		return err;
	}

	context->pk_params->pk_sign_type = pk_sign_type;

	IOT_DEBUG("type = %d", pk_sign_type);

	return IOT_ERROR_NONE;
}
#endif

// STATIC_FUNCTION
// void _iot_security_be_hardware_asn1_write_int(unsigned char **p, unsigned char *raw, int base_ofs, size_t len)
// {
// 	size_t length;

// 	// TAG : INTEGER
// 	*(*p)++ = MBEDTLS_ASN1_INTEGER;

// 	// LENGTH
// 	length = len;
// 	if (raw[base_ofs] & 0x80) {
// 		length += 1;
// 	}
// 	*(*p)++ = length;

// 	// VALUE
// 	if (raw[base_ofs] & 0x80) {
// 		*(*p)++ = 0x00;
// 	}

// 	memcpy(*p, raw + base_ofs, len);
// 	*p += len;
// }

// STATIC_FUNCTION
// iot_error_t _iot_security_be_hardware_sign_raw_to_der(iot_security_buffer_t *raw_buf, iot_security_buffer_t *der_buf)
// {
//     const int asn1_extra_len = 6;
// 	unsigned char *p;
// 	size_t int_r_len;
// 	size_t int_s_len;
// 	int len;

// 	if (!raw_buf || !der_buf) {
// 		IOT_ERROR("params is null");
// 		return IOT_ERROR_INVALID_ARGS;
// 	}

// 	if (raw_buf->len != 0x40) {
// 		IOT_ERROR("not supported length %d", raw_buf->len);
// 		return IOT_ERROR_INVALID_ARGS;
// 	}

// 	/*
// 	 * Get expected DER buffer size
// 	 */
// 	int_r_len = raw_buf->len / 2;
// 	int_s_len = raw_buf->len - int_r_len;

// 	der_buf->len = raw_buf->len + asn1_extra_len;
// 	if (raw_buf->p[0] & 0x80) {
// 		der_buf->len += 1;
// 	}
// 	if (raw_buf->p[int_r_len] & 0x80) {
// 		der_buf->len += 1;
// 	}

// 	der_buf->p = (unsigned char *)iot_os_malloc(der_buf->len);
// 	if (!der_buf->p) {
// 		IOT_ERROR("failed to malloc for der buf");
// 		return IOT_ERROR_MEM_ALLOC;
// 	}

// 	/*
// 	 * Fill DER buffer
// 	 */
// 	p = der_buf->p;

// 	// TAG : SEQUENCE
// 	*p++ = (MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);

// 	// LENGTH
// 	len = 4 + raw_buf->len;
// 	if (raw_buf->p[0] & 0x80) {
// 		len += 1;
// 	}
// 	if (raw_buf->p[int_r_len] & 0x80) {
// 		len += 1;
// 	}
// 	*p++ = len;

// 	_iot_security_be_hardware_asn1_write_int(&p, raw_buf->p, 0, int_r_len);
// 	_iot_security_be_hardware_asn1_write_int(&p, raw_buf->p, int_r_len, int_s_len);

// 	return IOT_ERROR_NONE;
// }

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_pk_sign(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *sig_buf)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_PK);
	if (err) {
		return err;
	}

	if (!input_buf || !input_buf->p || (input_buf->len == 0)) {
		IOT_ERROR("input buffer is invalid");
		return IOT_ERROR_INVALID_ARGS;
	}

	if (!sig_buf) {
		IOT_ERROR("sig buffer is null");
		return IOT_ERROR_INVALID_ARGS;
	}

	if (context->pk_params->pk_sign_type == IOT_SECURITY_PK_SIGN_TYPE_DER) {
		// err = iot_security_be_hardware_se_pk_sign(context, input_buf, &raw_buf);
		// if (err) {
		// 	return err;
		// }
		// err = _iot_security_be_hardware_sign_raw_to_der(&raw_buf, sig_buf);
		// if (err) {
		// 	return err;
		// }
		return IOT_ERROR_INVALID_ARGS;
	}
	else {
		err = iot_security_be_hardware_se_pk_sign(context, input_buf, sig_buf);
		if (err) {
			return err;
		}
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_pk_verify(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *sig_buf)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_PK);
	if (err) {
		return err;
	}

	if (!input_buf || !input_buf->p || (input_buf->len == 0)) {
		IOT_ERROR("input buffer is invalid");
		return IOT_ERROR_INVALID_ARGS;
	}

	if (!sig_buf || !sig_buf->p || (sig_buf->len == 0)) {
		IOT_ERROR("sig buffer is invalid");
		return IOT_ERROR_INVALID_ARGS;
	}

	err = iot_security_be_hardware_se_pk_verify(context, input_buf, sig_buf);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}


STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_manager_generate_key(iot_security_context_t *context, iot_security_key_id_t key_id)
{
	iot_error_t err;

	if (key_id != IOT_SECURITY_KEY_ID_EPHEMERAL) {
		IOT_ERROR("key id %d is not for a ephemeral", key_id);
		IOT_ERROR_DUMP_AND_RETURN(KEY_INVALID_ID, 0);
	}

	err = iot_security_be_hardware_se_manager_generate_key(context, key_id);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_manager_remove_key(iot_security_context_t *context, iot_security_key_id_t key_id)
{
	iot_error_t err = IOT_ERROR_NONE;

	if (key_id != IOT_SECURITY_KEY_ID_EPHEMERAL && key_id != IOT_SECURITY_KEY_ID_SHARED_SECRET) {
		IOT_ERROR("key id %d is not supported", key_id);
		IOT_ERROR_DUMP_AND_RETURN(KEY_INVALID_ID, 0);
	}

	err = iot_security_be_hardware_se_manager_remove_key(context, key_id);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_manager_set_key(iot_security_context_t *context, iot_security_key_params_t *key_params)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_CIPHER);
	if (err) {
		return err;
	}

	if (!key_params) {
		IOT_ERROR("key params is null");
		IOT_ERROR_DUMP_AND_RETURN(INVALID_ARGS, 0);
	}

	if (key_params->key_id == IOT_SECURITY_KEY_ID_SHARED_SECRET) {
		iot_security_cipher_params_t *cipher_params = context->cipher_params;
		iot_security_cipher_params_t *cipher_set_params = &key_params->params.cipher;

		if (cipher_set_params->key.p && cipher_set_params->key.len) {
			cipher_params->key = cipher_set_params->key;
		}

		if (cipher_set_params->iv.p && cipher_set_params->iv.len) {
			cipher_params->iv = cipher_set_params->iv;
		}

		err = iot_security_be_hardware_se_manager_set_key(context, key_params->key_id);
		if (err) {
			return err;
		}
	} else {
		IOT_ERROR("cannot set key for key index '%d'", key_params->key_id);
		IOT_ERROR_DUMP_AND_RETURN(KEY_INVALID_ID, 0);
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_manager_get_key(iot_security_context_t *context, iot_security_key_id_t key_id, iot_security_buffer_t *key_buf)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_NONE);
	if (err) {
		return err;
	}

	if (key_id == IOT_SECURITY_KEY_ID_SHARED_SECRET) {
		if (!context->cipher_params) {
			IOT_ERROR("cipher params is null");
			IOT_ERROR_DUMP_AND_RETURN(INVALID_ARGS, 0);
		}

		if (!context->cipher_params->key.p || (context->cipher_params->key.len == 0)) {
			IOT_ERROR("shared secret not yet set");
			IOT_ERROR_DUMP_AND_RETURN(KEY_NOT_FOUND, 0);
		}

		key_buf->len = context->cipher_params->key.len;
		key_buf->p = (unsigned char *)iot_os_malloc(key_buf->len);
		if (!key_buf->p) {
			IOT_ERROR("failed to malloc for getting key");
			key_buf->len = 0;
			IOT_ERROR_DUMP_AND_RETURN(MEM_ALLOC, 0);
		}
		memcpy(key_buf->p, context->cipher_params->key.p, key_buf->len);
	}
	else {
		err = iot_security_be_hardware_se_manager_get_key(context, key_id, key_buf);
		if (err) {
			return err;
		}
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_manager_get_certificate(iot_security_context_t *context, iot_security_cert_id_t cert_id, iot_security_buffer_t *cert_buf)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_NONE);
	if (err) {
		return err;
	}

	err = iot_security_be_hardware_se_manager_get_certificate(context, cert_id, cert_buf);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}


STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_cipher_deinit(iot_security_context_t *context)
{
	iot_error_t err;
	iot_security_cipher_params_t *cipher_params;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_CIPHER);
	if (err) {
		return err;
	}

	cipher_params = context->cipher_params;

	if (cipher_params->key.p) {
		_iot_security_be_hardware_manager_remove_key(context, IOT_SECURITY_KEY_ID_SHARED_SECRET);
		_iot_security_be_hardware_buffer_free(&cipher_params->key);
	}

	if (cipher_params->iv.p) {
		_iot_security_be_hardware_buffer_free(&cipher_params->iv);
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_cipher_copy_params(iot_security_buffer_t *src, iot_security_buffer_t *dst)
{
	if (src->p) {
		if (src->len == 0) {
			IOT_ERROR("length of src is zero");
			IOT_ERROR_DUMP_AND_RETURN(INVALID_ARGS, 0);
		}

		if (dst->p) {
			_iot_security_be_hardware_buffer_free(dst);
		}

		dst->p = (unsigned char *)iot_os_malloc(src->len);
		if (!dst->p) {
			IOT_ERROR("failed to malloc for dst params");
			IOT_ERROR_DUMP_AND_RETURN(MEM_ALLOC, 0);
		}

		memcpy(dst->p, src->p, src->len);
		dst->len = src->len;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_cipher_set_params(iot_security_context_t *context, iot_security_cipher_params_t *cipher_set_params)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_CIPHER);
	if (err) {
		return err;
	}

	if (!cipher_set_params) {
		IOT_ERROR("cipher set params is null");
		return IOT_ERROR_INVALID_ARGS;
	}

	if ((cipher_set_params->type > IOT_SECURITY_KEY_TYPE_UNKNOWN) &&
		(cipher_set_params->type < IOT_SECURITY_KEY_TYPE_MAX)) {
		context->cipher_params->type = cipher_set_params->type;
	}

	err = _iot_security_be_hardware_cipher_copy_params(&cipher_set_params->key, &context->cipher_params->key);
	if (err) {
		return err;
	}

	err = _iot_security_be_hardware_cipher_copy_params(&cipher_set_params->iv, &context->cipher_params->iv);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_cipher_aes_encrypt(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *output_buf)
{
	iot_error_t err = IOT_ERROR_NONE;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_CIPHER);
	if (err) {
		return err;
	}

	err = port_crypto_cipher_encrypt(context->cipher_params, input_buf, output_buf);
	if (err) {
		IOT_ERROR("cipher encrypt error %d", err);
		return err;
	}

	return err;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_cipher_aes_decrypt(iot_security_context_t *context, iot_security_buffer_t *input_buf, iot_security_buffer_t *output_buf)
{
	iot_error_t err = IOT_ERROR_NONE;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_CIPHER);
	if (err) {
		return err;
	}

	err = port_crypto_cipher_decrypt(context->cipher_params, input_buf, output_buf);
	if (err) {
		IOT_ERROR("cipher decrypt error %d", err);
		return err;
	}

	return err;
}


STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_ecdh_deinit(iot_security_context_t *context)
{
	iot_error_t err;
	iot_security_ecdh_params_t *ecdh_params;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_ECDH);
	if (err) {
		return err;
	}

	ecdh_params = context->ecdh_params;

	if (ecdh_params->t_seckey.p) {
		_iot_security_be_hardware_buffer_free(&ecdh_params->t_seckey);
	}

	if (ecdh_params->c_pubkey.p) {
		_iot_security_be_hardware_buffer_free(&ecdh_params->c_pubkey);
	}

	if (ecdh_params->salt.p) {
		_iot_security_be_hardware_buffer_free(&ecdh_params->salt);
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_ecdh_copy_params(iot_security_buffer_t *src, iot_security_buffer_t *dst)
{
	if (src->p) {
		if (src->len == 0) {
			IOT_ERROR("length of src is zero");
			IOT_ERROR_DUMP_AND_RETURN(INVALID_ARGS, 0);
		}

		if (dst->p) {
			_iot_security_be_hardware_buffer_free(dst);
		}

		dst->p = (unsigned char *)iot_os_malloc(src->len);
		if (!dst->p) {
			IOT_ERROR("failed to malloc for dst params");
			IOT_ERROR_DUMP_AND_RETURN(MEM_ALLOC, 0);
		}

		memcpy(dst->p, src->p, src->len);
		dst->len = src->len;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_ecdh_set_params(iot_security_context_t *context, iot_security_ecdh_params_t *ecdh_set_params)
{
	iot_error_t err;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_ECDH);
	if (err) {
		return err;
	}

	if (!ecdh_set_params) {
		IOT_ERROR("ecdh set params is null");
		IOT_ERROR_DUMP_AND_RETURN(INVALID_ARGS, 0);
	}

	context->ecdh_params->key_id = ecdh_set_params->key_id;

	err = _iot_security_be_hardware_ecdh_copy_params(&ecdh_set_params->t_seckey, &context->ecdh_params->t_seckey);
	if (err) {
		return err;
	}

	err = _iot_security_be_hardware_ecdh_copy_params(&ecdh_set_params->c_pubkey, &context->ecdh_params->c_pubkey);
	if (err) {
		return err;
	}

	err = _iot_security_be_hardware_ecdh_copy_params(&ecdh_set_params->salt, &context->ecdh_params->salt);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_ecdh_compute_shared_secret(iot_security_context_t *context, iot_security_buffer_t *output_buf)
{
	iot_error_t err;
	iot_security_ecdh_params_t *ecdh_params;
	iot_security_buffer_t pmsecret_buf = { 0 };
	iot_security_buffer_t secret_buf = { 0 };
	iot_security_buffer_t shared_secret_buf = { 0 };

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_ECDH);
	if (err) {
		return err;
	}

	ecdh_params = context->ecdh_params;

	err = iot_security_be_hardware_se_ecdh_compute_shared_secret(context, &ecdh_params->c_pubkey, &pmsecret_buf);
	if (err) {
		return err;
	}

	secret_buf.len = pmsecret_buf.len + ecdh_params->salt.len;
	secret_buf.p = (unsigned char *)iot_os_malloc(secret_buf.len);
	if (!secret_buf.p) {
		IOT_ERROR("failed to malloc for secret");
		err = IOT_ERROR_MEM_ALLOC;
		goto exit_free_pmsecret;
	}

	memcpy(secret_buf.p, pmsecret_buf.p, pmsecret_buf.len);
	memcpy(secret_buf.p + pmsecret_buf.len, ecdh_params->salt.p, ecdh_params->salt.len);

	shared_secret_buf.len = IOT_SECURITY_SHA256_LEN;
	shared_secret_buf.p = (unsigned char *)iot_os_malloc(shared_secret_buf.len);
	if (!shared_secret_buf.p) {
		IOT_ERROR("failed to malloc for shared secret");
		err = IOT_ERROR_MEM_ALLOC;
		goto exit_free_secret;
	}

	err = iot_security_sha256(secret_buf.p, secret_buf.len, shared_secret_buf.p, shared_secret_buf.len);
	if (err) {
		goto exit_free_shared_secret;
	}

	if (context->sub_system & IOT_SECURITY_SUB_CIPHER) {
		iot_security_key_params_t shared_key_params = { 0 };
		shared_key_params.key_id = IOT_SECURITY_KEY_ID_SHARED_SECRET;
		shared_key_params.params.cipher.key = shared_secret_buf;
		err = _iot_security_be_hardware_manager_set_key(context, &shared_key_params);
		if (err) {
			goto exit_free_shared_secret;
		}
	}

	if (output_buf) {
		*output_buf = shared_secret_buf;
	}

	err = IOT_ERROR_NONE;
	goto exit_free_secret;

exit_free_shared_secret:
	_iot_security_be_hardware_buffer_free(&shared_secret_buf);
exit_free_secret:
	_iot_security_be_hardware_buffer_free(&secret_buf);
exit_free_pmsecret:
	_iot_security_be_hardware_buffer_free(&pmsecret_buf);

	return IOT_ERROR_NONE;
}


STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_storage_read(iot_security_context_t *context, iot_security_buffer_t *data_buf)
{
	iot_error_t err;
	iot_security_storage_params_t *storage_params;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_STORAGE);
	if (err) {
		return err;
	}

	storage_params = context->storage_params;

	err = iot_security_be_hardware_se_storage_read(context, storage_params->storage_id, data_buf);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_storage_write(iot_security_context_t *context, iot_security_buffer_t *data_buf)
{
	iot_error_t err;
	iot_security_storage_params_t *storage_params;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_STORAGE);
	if (err) {
		return err;
	}

	storage_params = context->storage_params;

	err = iot_security_be_hardware_se_storage_write(context, storage_params->storage_id, data_buf);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}

STATIC_FUNCTION
iot_error_t _iot_security_be_hardware_storage_remove(iot_security_context_t *context)
{
	iot_error_t err;
	iot_security_storage_params_t *storage_params;

	err = _iot_security_be_check_context_and_params_is_valid(context, IOT_SECURITY_SUB_STORAGE);
	if (err) {
		return err;
	}

	storage_params = context->storage_params;

	err = iot_security_be_hardware_se_storage_remove(context, storage_params->storage_id);
	if (err) {
		return err;
	}

	return IOT_ERROR_NONE;
}


const iot_security_be_funcs_t iot_security_be_hardware_funcs = {
	.pk_init = _iot_security_be_hardware_pk_init,
	.pk_deinit = _iot_security_be_hardware_pk_deinit,
	.pk_set_params = NULL,
	.pk_get_key_type = _iot_security_be_hardware_pk_get_key_type,
#if defined(CONFIG_STDK_IOT_CORE_CRYPTO_SUPPORT_ECDSA)
	.pk_set_sign_type = _iot_security_be_hardware_pk_set_sign_type,
#else
	.pk_set_sign_type = NULL,
#endif
	.pk_sign = _iot_security_be_hardware_pk_sign,
	.pk_verify = _iot_security_be_hardware_pk_verify,

	.cipher_init = NULL,
	.cipher_deinit = _iot_security_be_hardware_cipher_deinit,
	.cipher_set_params = _iot_security_be_hardware_cipher_set_params,
	.cipher_aes_encrypt = _iot_security_be_hardware_cipher_aes_encrypt,
	.cipher_aes_decrypt = _iot_security_be_hardware_cipher_aes_decrypt,

	.ecdh_init = NULL,
	.ecdh_deinit = _iot_security_be_hardware_ecdh_deinit,
	.ecdh_set_params = _iot_security_be_hardware_ecdh_set_params,
	.ecdh_compute_shared_secret = _iot_security_be_hardware_ecdh_compute_shared_secret,

	.manager_init = NULL,
	.manager_deinit = NULL,
	.manager_generate_key = _iot_security_be_hardware_manager_generate_key,
	.manager_remove_key = _iot_security_be_hardware_manager_remove_key,
	.manager_set_key = _iot_security_be_hardware_manager_set_key,
	.manager_get_key = _iot_security_be_hardware_manager_get_key,
	.manager_get_certificate = _iot_security_be_hardware_manager_get_certificate,

	.storage_init = NULL,
	.storage_deinit = NULL,
	.storage_read = _iot_security_be_hardware_storage_read,
	.storage_write = _iot_security_be_hardware_storage_write,
	.storage_remove = _iot_security_be_hardware_storage_remove,
};

iot_security_be_context_t *iot_security_be_init(external_nv_callback external_nv_cb)
{
	iot_error_t err;
	iot_security_be_context_t *be_context;

	be_context = (iot_security_be_context_t *)iot_os_malloc(sizeof(iot_security_be_context_t));
	if (!be_context) {
		IOT_ERROR("failed to malloc for context");
		return NULL;
	}

	memset(be_context, 0, sizeof(iot_security_be_context_t));

	be_context->name = "hardware";
	be_context->fn = &iot_security_be_hardware_funcs;
	be_context->external_device_info_cb = external_nv_cb;

	err = iot_security_be_bsp_init(be_context);
	if (err) {
		IOT_ERROR("iot_security_be_bsp_init = %d", err);
		iot_os_free(be_context);
		return NULL;
	}

	err = iot_security_be_hardware_se_init();
	if (err) {
		IOT_ERROR("iot_security_be_hardware_se_init = %d", err);
		iot_os_free(be_context);
		return NULL;
	}

	IOT_DEBUG("security backend is '%s'", be_context->name);

	return be_context;
}

iot_error_t iot_security_be_deinit(iot_security_be_context_t *be_context)
{
	iot_error_t err = IOT_ERROR_NONE;
	err = iot_security_be_hardware_se_deinit();
	if (err) {
		IOT_ERROR("iot_security_be_hardware_se_deinit = %d", err);
		return IOT_ERROR_INVALID_ARGS;
	}

	if (!be_context) {
		IOT_ERROR("backend context is null");
		return IOT_ERROR_INVALID_ARGS;
	}

	memset(be_context, 0, sizeof(iot_security_be_context_t));

	iot_os_free(be_context);

	return IOT_ERROR_NONE;
}

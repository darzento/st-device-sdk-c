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



void hal_mpi_zeroize (uint64_t *v, uint32_t n)
{
    volatile uint64_t *p = v;
    while (n--) {
        *p++ = 0;
    }
}   // End - static void hal_mpi_zeroize()


void hal_mpi_init (hal_mpi *X)
{
    
    if ( X == NULL ) {
        return;
    }

    X->s = 1;
    X->n = 0;
    X->p = NULL;
    
}   // End - static void hal_mpi_init()


void hal_mpi_free (hal_mpi *X)
{
    
    if ( X == NULL ) {
        return;
    }

    if ( X->p != NULL ) {
        hal_mpi_zeroize(X->p, X->n);
        free(X->p);
    }

    X->s = 1;
    X->n = 0;
    X->p = NULL;
    
}   // End - static void hal_mpi_free()


int hal_mpi_grow (hal_mpi *X, uint32_t nblimbs)
{
    
    uint64_t *p;

    if ( nblimbs > 10000 ) {
        return HAL_ALLOC_FAIL;
    }

    if ( X->n < nblimbs ) {
        if ( (p = (uint64_t *) calloc(nblimbs, ciL)) == NULL ) {
            return HAL_ALLOC_FAIL;
        }

        if (X->p != NULL) {
            memcpy(p, X->p, X->n * ciL);
            hal_mpi_zeroize(X->p, X->n);
            free(X->p);
        }

        X->n = nblimbs;
        X->p = p;
    }

    return HAL_SUCCESS;
    
}   // End - static int hal_mpi_grow()


int hal_mpi_lset (hal_mpi *X, int64_t z)
{
    
    int ret;

    HAL_MPI_CHK(hal_mpi_grow(X, 1));
    memset(X->p, 0, X->n * ciL);

    X->p[0] = (z < 0) ? -z : z;
    X->s = (z < 0) ? -1 : 1;

cleanup:
    return ret;
    
}   // End - static int hal_mpi_lset()


uint32_t hal_clz (const uint64_t x)
{
    
    uint32_t j;
    uint64_t mask = (uint64_t) 1 << (biL - 1);

    for ( j = 0; j < biL; j++ ) {
        if ( x & mask ) {
            break;
        }
        mask >>= 1;
    }

    return j;
    
}   // End - static uint32_t hal_clz()


uint32_t hal_mpi_bitlen (const hal_mpi *X)
{
    
    uint32_t i;
    uint32_t j;

    if ( X->n == 0 ) {
        return 0;
    }

    for ( i = X->n - 1; i > 0; i-- ) {
        if ( X->p[i] != 0 ) {
            break;
        }
    }

    j = biL - hal_clz(X->p[i]);

    return (i * biL) + j;
    
}   // End - static uint32_t hal_mpi_bitlen()


uint32_t hal_mpi_size (const hal_mpi *X)
{
    return (hal_mpi_bitlen(X) + 7) >> 3;
}   // End - static uint32_t hal_mpi_size()


int hal_asn1_get_len (unsigned char **p, const unsigned char *end, uint32_t *len)
{
    
    if ( (end - *p) < 1 ) {
        return HAL_INVALID_ARGS;
    }

    if ( (**p & 0x80) == 0 ) {
        *len = *(*p)++;
    } else {
        switch (**p & 0x7F) {

            case 1:
                {
                    if ( (end - *p) < 2 ) {
                        return HAL_INVALID_ARGS;
                    }

                    *len = (*p)[1];
                    (*p) += 2;
                }
                break;

            case 2:
                {
                    if ( (end - *p) < 3 ) {
                        return HAL_INVALID_ARGS;
                    }
                    *len = ( (uint32_t)(*p)[1] << 8 ) | (*p)[2];
                    (*p) += 3;
                }
                break;

            case 3:
                {
                    if ( (end - *p) < 4 ) {
                        return HAL_INVALID_ARGS;
                    }
                    *len = ((uint32_t)(*p)[1] << 16) | ((uint32_t)(*p)[2] << 8) | (*p)[3];
                    (*p) += 4;
                }
                break;

            case 4:
                {
                    if ( (end - *p) < 5 ) {
                        return HAL_INVALID_ARGS;
                    }
                    *len = ((uint32_t)(*p)[1] << 24) | ((uint32_t)(*p)[2] << 16) | ((uint32_t)(*p)[3] << 8) | (*p)[4];
                    (*p) += 5;
                }
                break;

            default:
                return HAL_INVALID_ARGS;

        }
    }

    if ( *len > (uint32_t)(end - *p) ) {
        return HAL_INVALID_ARGS;
    }

    return HAL_SUCCESS;
    
}   // End - static int hal_asn1_get_len()


int hal_asn1_get_tag (unsigned char **p, const unsigned char *end, uint32_t *len, int tag)
{

    if ( (end - *p) < 1 ) {
        return HAL_INVALID_ARGS;
    }

    if ( **p != tag ) {
        return HAL_INVALID_ARGS;
    }

    (*p)++;

    return hal_asn1_get_len(p, end, len);
    
}   // End - static int hal_asn1_get_tag()



int hal_mpi_read_binary (hal_mpi *X, const unsigned char *buf, uint32_t buflen)
{
    
    int ret;
    uint32_t i;
    uint32_t j;
    uint32_t const limbs = CHARS_TO_LIMBS(buflen);

    /* Ensure that target MPI has exactly the necessary number of limbs */
    if ( X->n != limbs ) {
        hal_mpi_free(X);
        hal_mpi_init(X);
        HAL_MPI_CHK(hal_mpi_grow(X, limbs));
    }

    HAL_MPI_CHK(hal_mpi_lset(X, 0));

    for ( i = buflen, j = 0; i > 0; i--, j++ ) {
        X->p[j / ciL] |= ((uint64_t) buf[i - 1]) << ((j % ciL) << 3);
    }

cleanup:
    return ret;
    
}   // End- static int hal_mpi_read_binary()


int hal_mpi_write_binary (const hal_mpi *X, unsigned char *buf, uint32_t buflen)
{

    uint32_t stored_bytes = X->n * ciL;
    uint32_t bytes_to_copy;
    unsigned char *p;
    uint32_t i;

    if ( stored_bytes < buflen ) {
        bytes_to_copy = stored_bytes;
        p = buf + buflen - stored_bytes;
        memset(buf, 0, buflen - stored_bytes);
    } else {
        bytes_to_copy = buflen;
        p = buf;
        for ( i = bytes_to_copy; i < stored_bytes; i++ ) {
            if (GET_BYTE(X, i) != 0) {
                return HAL_NOT_ENOUGH_MEMORY;
            }
        }
    }

    for ( i = 0; i < bytes_to_copy; i++ ) {
        p[bytes_to_copy - i - 1] = GET_BYTE(X, i);
    }

    return HAL_SUCCESS;

}   // End - static int hal_mpi_write_binary()


int hal_asn1_write_tag (unsigned char **p, unsigned char *start, unsigned char tag)
{
    
    if ( *p - start < 1 ) {
        return -HAL_INVALID_ARGS;
    }

    *--(*p) = tag;

    return 1;

}   // End - static int hal_asn1_write_tag()


int hal_asn1_write_len (unsigned char **p, unsigned char *start, uint32_t len)
{
    
    if (len < 0x80) {
        if (*p - start < 1) {
            return -HAL_INVALID_ARGS;
        }
        *--(*p) = (unsigned char)len;
        return 1;
    }

    if (len <= 0xFF) {
        if (*p - start < 2) {
            return -HAL_INVALID_ARGS;
        }
        *--(*p) = (unsigned char)len;
        *--(*p) = 0x81;
        return 2;
    }

    if (len <= 0xFFFF) {
        if (*p - start < 3) {
            return -HAL_INVALID_ARGS;
        }
        *--(*p) = (len) & 0xFF;
        *--(*p) = (len >> 8) & 0xFF;
        *--(*p) = 0x82;
        return 3;
    }

    if (len <= 0xFFFFFF) {
        if (*p - start < 4) {
            return -HAL_INVALID_ARGS;
        }

        *--(*p) = (len) & 0xFF;
        *--(*p) = (len >> 8) & 0xFF;
        *--(*p) = (len >> 16) & 0xFF;
        *--(*p) = 0x83;
        return 4;
    }

    if (*p - start < 5) {
        return -HAL_INVALID_ARGS;
    }

    *--(*p) = (len) & 0xFF;
    *--(*p) = (len >> 8) & 0xFF;
    *--(*p) = (len >> 16) & 0xFF;
    *--(*p) = (len >> 24) & 0xFF;
    *--(*p) = 0x84;
    
    return 5;
    
}   // End - static int hal_asn1_write_len()


int hal_asn1_write_mpi (unsigned char **p, unsigned char *start, const hal_mpi *X)
{
    
    int ret;
    uint32_t len = 0;

    len = hal_mpi_size(X);

    if ( *p < start || (uint32_t)(*p - start) < len ) {
        return -HAL_INVALID_ARGS;
    }

    (*p) -= len;
    HAL_MPI_CHK(hal_mpi_write_binary(X, *p, len));

    if ( X->s == 1 && **p & 0x80 ) {
        if ( *p - start < 1 ) {
            return HAL_NOT_ENOUGH_MEMORY;
        }
        *--(*p) = 0x00;
        len += 1;
    }

    HAL_ASN1_CHK_ADD(len, hal_asn1_write_len(p, start, len));
    HAL_ASN1_CHK_ADD(len, hal_asn1_write_tag(p, start, 0x02));

    ret = (int)len;

cleanup:
    return ret;
    
}   // End - static int hal_asn1_write_mpi()


int hal_signature_to_asn1 (const hal_mpi *r, const hal_mpi *s, unsigned char *sig, uint32_t *slen)
{
    
    int ret;
    unsigned char buf[HAL_MAX_ECDSA_LEN];
    unsigned char *p = buf + sizeof(buf);
    uint32_t len = 0;

    HAL_ASN1_CHK_ADD(len, hal_asn1_write_mpi(&p, buf, s));
    HAL_ASN1_CHK_ADD(len, hal_asn1_write_mpi(&p, buf, r));
    HAL_ASN1_CHK_ADD(len, hal_asn1_write_len(&p, buf, len));
    HAL_ASN1_CHK_ADD(len, hal_asn1_write_tag(&p, buf, 0x20 | 0x10));

    memcpy(sig, p, len);
    *slen = len;

    return HAL_SUCCESS;
    
}   // End - static int hal_signature_to_asn1()


int hal_asn1_get_mpi (unsigned char **p, const unsigned char *end, hal_mpi *X)
{
    
    int ret;
    uint32_t len;

    if ((ret = hal_asn1_get_tag(p, end, &len, 0x02)) != 0) {
        return ret;
    }

    ret = hal_mpi_read_binary(X, *p, len);

    *p += len;

    return ret;
    
}   // End - static int hal_asn1_get_mpi()

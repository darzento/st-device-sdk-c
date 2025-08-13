/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/

#ifndef __HAL_UONE_UTILS_H__
#define __HAL_UONE_UTILS_H__

#include "security/backend/hardware/se/Hal_uone.h"


#define ciL    (sizeof(uint64_t))
#define biL    (ciL << 3)       /* bits  in limb  */
#define biH    (ciL << 2)       /* half limb size */


#define HAL_ASN1_CHK_ADD(g, f)                      \
    do { if ((ret = f) < 0) return HAL_FAIL; else   \
        g += ret;                                   \
    } while (0)


#define GET_BYTE(X, i)                                                \
    (((X)->p[(i) / ciL] >> (((i) % ciL) * 8)) & 0xff)


#define HAL_MPI_CHK(f) do { if ((ret = f) != 0) { ret = HAL_FAIL; goto cleanup; } } while (0)


#define CHARS_TO_LIMBS(i) ((i) / ciL + ((i) % ciL != 0))


typedef struct _hal_mpi {
    int s;
    uint32_t n;
    uint64_t *p;
} hal_mpi;


/* Util Functions */
void hal_mpi_zeroize (uint64_t *v, uint32_t n);

void hal_mpi_init (hal_mpi *X);

void hal_mpi_free (hal_mpi *X);

int hal_mpi_grow (hal_mpi *X, uint32_t nblimbs);

int hal_mpi_lset (hal_mpi *X, int64_t z);

uint32_t hal_clz (const uint64_t x);

uint32_t hal_mpi_bitlen (const hal_mpi *X);

uint32_t hal_mpi_size (const hal_mpi *X);

int hal_asn1_get_len (unsigned char **p, const unsigned char *end, uint32_t *len);

int hal_asn1_get_tag (unsigned char **p, const unsigned char *end, uint32_t *len, int tag);

int hal_mpi_read_binary (hal_mpi *X, const unsigned char *buf, uint32_t buflen);

int hal_mpi_write_binary (const hal_mpi *X, unsigned char *buf, uint32_t buflen);

int hal_asn1_write_tag (unsigned char **p, unsigned char *start, unsigned char tag);

int hal_asn1_write_len (unsigned char **p, unsigned char *start, uint32_t len);

int hal_asn1_write_mpi (unsigned char **p, unsigned char *start, const hal_mpi *X);

int hal_signature_to_asn1 (const hal_mpi *r, const hal_mpi *s, unsigned char *sig, uint32_t *slen);

int hal_asn1_get_mpi (unsigned char **p, const unsigned char *end, hal_mpi *X);

#endif  // #ifndef __HAL_UONE_CUSTOM_H__

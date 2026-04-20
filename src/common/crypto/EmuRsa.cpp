// ******************************************************************
// *
// *  This file is part of the Cxbx project.
// *
// *  Cxbx and Cxbe are free software; you can redistribute them
// *  and/or modify them under the terms of the GNU General Public
// *  License as published by the Free Software Foundation; either
// *  version 2 of the license, or (at your option) any later version.
// *
// *  This program is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *  GNU General Public License for more details.
// *
// *  You should have recieved a copy of the GNU General Public License
// *  along with this program; see the file COPYING.
// *  If not, write to the Free Software Foundation, Inc.,
// *  59 Temple Place - Suite 330, Bostom, MA 02111-1307, USA.
// *
// *  (c) 2018 ergo720
// *  (c) 2019 Jannik Vogel
// *
// *  All rights reserved
// *
// ******************************************************************

// Acknowledgment:
// verify_hash, RSApkcs1paddingtable and RSA_PUBLIC_KEY are from the
// file xboxlib.c of the xbedump tool (and that file only, GPLv2).
// https://github.com/XboxDev/xbedump/blob/master/xboxlib.c
// mbedtls_swap_endianness is extracted from mbedtls_mpi_read_binary used in the file bignum.h of ReactOS
// https://github.com/reactos/reactos

// xboxlib.c license
/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/**
* \file bignum.h
*
* \brief  Multi-precision integer library
*
*  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
*  SPDX-License-Identifier: GPL-2.0
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*  This file is part of mbed TLS (https://tls.mbed.org)
*/

#define LOG_PREFIX CXBXR_MODULE::RSA

#include "EmuRsa.h"
#include "core\kernel\support\Emu.h" // For EmuLog
#include "tomcrypt.h"
#include "tommath.h"

#define CHK_MP_RET(x) do { int ret = (x); if (ret != MP_OKAY) return false; } while(0)


const unsigned char RSApkcs1paddingtable[3][16] = {
	{ 0x0F, 0x14,0x04,0x00,0x05,0x1A,0x02,0x03,0x0E,0x2B,0x05,0x06,0x09,0x30,0x21,0x30 },
	{ 0x0D, 0x14,0x04,0x1A,0x02,0x03,0x0E,0x2B,0x05,0x06,0x07,0x30,0x1F,0x30,0x00,0x00 },
	{ 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }
};

// Move this to CxbxUtil.h if it's ever needed in other places of the emu as well
void mbedtls_swap_endianness(const unsigned char* in_buf, unsigned char* out_buf, size_t size)
{
	size_t i, j, n;
	uint32_t* out_buf_uint = (uint32_t*)out_buf;

	memset(out_buf_uint, 0, size);

	for (n = 0; n < size; n++)
		if (in_buf[n] != 0)
			break;

	for (i = size, j = 0; i > n; i--, j++) {
		out_buf_uint[j / 4] |= ((uint32_t)in_buf[i - 1]) << ((j % 4) << 3);
	}
}

void init_tom_lib()
{
	static bool need_init = true;
	if (need_init) {
		crypt_mp_init("ltm");
		need_init = false;
	}
}

bool xbox_exp_mod(unsigned char* pA, const unsigned char* pB, const unsigned char* pC, const unsigned char* pD,
	size_t a_size, size_t b_size, size_t c_size, size_t d_size)
{
	mp_int a, b, c, d;
	CHK_MP_RET(mp_init(&a));
	CHK_MP_RET(mp_init(&b));
	CHK_MP_RET(mp_init(&c));
	CHK_MP_RET(mp_init(&d));
	CHK_MP_RET(mp_unpack(&b, 1, MP_LSB_FIRST, b_size, MP_NATIVE_ENDIAN, 0, pB));
	CHK_MP_RET(mp_unpack(&c, 1, MP_LSB_FIRST, c_size, MP_NATIVE_ENDIAN, 0, pC));
	CHK_MP_RET(mp_unpack(&d, 1, MP_LSB_FIRST, d_size, MP_NATIVE_ENDIAN, 0, pD));

	CHK_MP_RET(mp_exptmod(&b, &c, &d, &a));

	CHK_MP_RET(mp_pack(pA, 1, NULL, MP_LSB_FIRST, a_size, MP_NATIVE_ENDIAN, 0, &a));

	return true;
}

bool xbox_rsa_public(const unsigned char* in_buf, unsigned char* out_buf, RSA_PUBLIC_KEY key)
{
	rsa_key tom_key;
	unsigned char in_buf_be[256] = { 0 };
	unsigned char out_buf_be[256] = { 0 };
	unsigned long out_len = 256;
	unsigned char modulus_be[256] = { 0 };
	unsigned char exp_be[4] = { 0 };

	// We must swap the data since libtom expects the data to be in big endian
	mbedtls_swap_endianness (key.KeyData.Modulus, modulus_be, 256);
	mbedtls_swap_endianness (key.KeyData.Exponent, exp_be, 4);
	mbedtls_swap_endianness (in_buf, in_buf_be, 256);
	if (rsa_set_key(modulus_be, 256, exp_be, 4, NULL, 0, &tom_key) != CRYPT_OK) {
		EmuLog(LOG_LEVEL::WARNING, "Failed to load rsa key");
		return false;
	}
	if (rsa_exptmod(in_buf_be, 256, out_buf_be, &out_len, PK_PUBLIC, &tom_key) != CRYPT_OK) {
		EmuLog(LOG_LEVEL::WARNING, "rsa_exptmod failed");
		return false;
	}
	mbedtls_swap_endianness(out_buf_be, out_buf, 256);
	return true;
}

bool verify_hash(const unsigned char* hash, const unsigned char* decryptBuffer, RSA_PUBLIC_KEY key)
{
	unsigned char cmphash[20];
	int a;
	int zero_position = 20;

	// Convert Hash to "Big-Endian Format"
	for (a = 0;a<20;a++) cmphash[a] = hash[19 - a];

	// Compare if the Hash Results (first 20 Bytes) are the same
	if (memcmp(decryptBuffer, cmphash, 20) != 0)   return false;

	unsigned char *pkcspad;
	for (int tableIndex = 0; RSApkcs1paddingtable[tableIndex][0] != 0; tableIndex++) {

		pkcspad = (unsigned char*)RSApkcs1paddingtable[tableIndex];
		int difference = memcmp(pkcspad + 1, &decryptBuffer[20], *pkcspad);

		if (!difference)
		{
			zero_position = *pkcspad + 20;
			break;
		}

	}

	// Padding checking , xbox does exactly the same 

	if (decryptBuffer[zero_position] != 0x00) return false;

	if (decryptBuffer[key.KeyData.ModulusSize] != 0x00) return false;

	if (decryptBuffer[key.KeyData.ModulusSize - 1] != 0x01) return false;

	for (unsigned int i = zero_position + 1; i < (key.KeyData.ModulusSize - 1); i++) {
		if (decryptBuffer[i] != 0xff) return false;
	}

	return true;
}

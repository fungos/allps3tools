
/*
 * SHA1 hash implementation and interface functions
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 */

#ifndef _SHA1_H_
#define _SHA1_H_

#include <inttypes.h>

#define SHA1_MAC_LEN 20

struct SHA1Context
{
	u32 state[5];
	u32 count[2];
	unsigned char buffer[64];
};

typedef struct SHA1Context SHA1_CTX;

void SHA1Init(SHA1_CTX *context);

void SHA1Update(SHA1_CTX *context, const void *data, u32 len);

void SHA1Final(unsigned char digest[20], SHA1_CTX *context);

void sha1_vector(int num_elem, const u8 *addr[], const int *len, u8 *mac);

void hmac_sha1_vector(const u8 *key, int key_len, int num_elem,
	const u8 *addr[], const int *len, u8 *mac);

void hmac_sha1(const u8 *key, int key_len, const u8 *data, int data_len, u8 *mac);

#endif

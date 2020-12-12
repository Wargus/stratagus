/*
 * Copyright (C) 2020  The Stratagus Project
 * Copyright (C) 1999  Descolada (dyn1-tnt9-237.chicago.il.ameritech.net)
 * Copyright (C) 1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <cstdint>
#include <cstdio>
#include <cstring>

/* valid for 0<=n and w>0 , depends on 2's complement */
#define ROTL(x,n,w) (((x)<<((n)&(w-1))) | ((x)>>(((-(n))&(w-1)))))

/* valid for 0<=n and w>0 , uses three mods and an ugly conditional */
/* FIXME: and also a bug because it doesn't work on PPC */
/*#define ROTL(x,n,w) (((n)%(w)) ? (((x)<<((n)%(w))) | ((x)>>((w)-((n)%(w))))) : (x))*/

#define ROTL32(x,n) ROTL(x,n,32)
#define ROTL16(x,n) ROTL(x,n,16)


namespace pvpgn
{
	using bn_basic = std::uint8_t;
	using bn_byte = bn_basic[1];
	using bn_short = bn_basic[2];
	using bn_int = bn_basic[4];
	using bn_long = bn_basic[8];
	using t_hash = std::uint32_t[5];

	int bn_int_set(bn_int * dst, std::uint32_t src)
	{
		if (!dst)
		{
			return -1;
		}

		(*dst)[0] = (std::uint8_t)((src)& 0xff);
		(*dst)[1] = (std::uint8_t)((src >> 8) & 0xff);
		(*dst)[2] = (std::uint8_t)((src >> 16) & 0xff);
		(*dst)[3] = (std::uint8_t)((src >> 24));
		return 0;
	}

	int bn_int_nset(bn_int * dst, std::uint32_t src)
	{
		if (!dst)
			{
				return -1;
			}

		(*dst)[0] = (std::uint8_t)((src >> 24));
		(*dst)[1] = (std::uint8_t)((src >> 16) & 0xff);
		(*dst)[2] = (std::uint8_t)((src >> 8) & 0xff);
		(*dst)[3] = (std::uint8_t)((src)& 0xff);
		return 0;
	}

	std::uint32_t bn_int_get(bn_int const src)
	{
		std::uint32_t temp;

		if (!src)
		{
			return 0;
		}

		temp = ((std::uint32_t)src[0]);
		temp |= ((std::uint32_t)src[1]) << 8;
		temp |= ((std::uint32_t)src[2]) << 16;
		temp |= ((std::uint32_t)src[3]) << 24;
		return temp;
	}

	typedef enum {
		do_blizzard_hash,
		do_sha1_hash
	} t_hash_variant;

	static void hash_init(t_hash * hash);
	static void do_hash(t_hash * hash, std::uint32_t * tmp);
	static void hash_set_16(std::uint32_t * dst, unsigned char const * src, unsigned int count, t_hash_variant hash_variant);


	static void hash_init(t_hash * hash)
	{
		(*hash)[0] = 0x67452301;
		(*hash)[1] = 0xefcdab89;
		(*hash)[2] = 0x98badcfe;
		(*hash)[3] = 0x10325476;
		(*hash)[4] = 0xc3d2e1f0;
	}


	static void do_hash(t_hash * hash, std::uint32_t * tmp, t_hash_variant hash_variant)
	{
		unsigned int i;
		std::uint32_t     a, b, c, d, e, g;

		for (i = 0; i < 64; i++)
		if (hash_variant == do_blizzard_hash)
			tmp[i + 16] = ROTL32(1, tmp[i] ^ tmp[i + 8] ^ tmp[i + 2] ^ tmp[i + 13]);
		else
			tmp[i + 16] = ROTL32(tmp[i] ^ tmp[i + 8] ^ tmp[i + 2] ^ tmp[i + 13], 1);

		a = (*hash)[0];
		b = (*hash)[1];
		c = (*hash)[2];
		d = (*hash)[3];
		e = (*hash)[4];

		for (i = 0; i < 20 * 1; i++)
		{
			g = tmp[i] + ROTL32(a, 5) + e + ((b & c) | (~b & d)) + 0x5a827999;
			e = d;
			d = c;
			c = ROTL32(b, 30);
			b = a;
			a = g;
		}

		for (; i < 20 * 2; i++)
		{
			g = (d ^ c ^ b) + e + ROTL32(g, 5) + tmp[i] + 0x6ed9eba1;
			e = d;
			d = c;
			c = ROTL32(b, 30);
			b = a;
			a = g;
		}

		for (; i < 20 * 3; i++)
		{
			g = tmp[i] + ROTL32(g, 5) + e + ((c & b) | (d & c) | (d & b)) - 0x70e44324;
			e = d;
			d = c;
			c = ROTL32(b, 30);
			b = a;
			a = g;
		}

		for (; i < 20 * 4; i++)
		{
			g = (d ^ c ^ b) + e + ROTL32(g, 5) + tmp[i] - 0x359d3e2a;
			e = d;
			d = c;
			c = ROTL32(b, 30);
			b = a;
			a = g;
		}

		(*hash)[0] += g;
		(*hash)[1] += b;
		(*hash)[2] += c;
		(*hash)[3] += d;
		(*hash)[4] += e;
	}


	/*
	 * Fill 16 elements of the array of 32 bit values with the bytes from
	 * dst up to count in little endian order. Fill left over space with
	 * zeros. In case of SHA1 hash variant a binary 1 is appended after
	 * the actual data.
	 */
	static void hash_set_16(std::uint32_t * dst, unsigned char const * src, unsigned int count,
		t_hash_variant hash_variant)
	{
		unsigned int i;
		unsigned int pos;

		for (pos = 0, i = 0; i < 16; i++)
		{
			dst[i] = 0;

			if (hash_variant == do_blizzard_hash) {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]);
			}
			else {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]) << 24;
				else if (pos == count)
					dst[i] |= ((std::uint32_t)0x80000000);
			}
			pos++;

			if (hash_variant == do_blizzard_hash) {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]) << 8;
			}
			else {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]) << 16;
				else if (pos == count)
					dst[i] |= ((std::uint32_t)0x800000);
			}
			pos++;

			if (hash_variant == do_blizzard_hash) {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]) << 16;
			}
			else {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]) << 8;
				else if (pos == count)
					dst[i] |= ((std::uint32_t)0x8000);
			}
			pos++;

			if (hash_variant == do_blizzard_hash) {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]) << 24;
			}
			else {
				if (pos < count)
					dst[i] |= ((std::uint32_t)src[pos]);
				else if (pos == count)
					dst[i] |= ((std::uint32_t)0x80);
			}
			pos++;
		}
	}


	extern int bnet_hash(t_hash * hashout, unsigned int size, void const * datain)
	{
		std::uint32_t tmp[64 + 16];
		const unsigned char* data;
		unsigned int inc;

		if (!hashout)
		{
			return -1;
		}
		if (size > 0 && !datain)
		{
			return -1;
		}

		hash_init(hashout);

		data = (const unsigned char*)datain;
		while (size > 0)
		{
			if (size > 64)
				inc = 64;
			else
				inc = size;

			hash_set_16(tmp, data, inc, do_blizzard_hash);
			do_hash(hashout, tmp, do_blizzard_hash);

			data += inc;
			size -= inc;
		}

		return 0;
	}

	static void hash_set_length(std::uint32_t * dst, unsigned int size){
		std::uint32_t size_high = 0;
		std::uint32_t size_low = 0;
		unsigned int counter;
		for (counter = 0; counter < size; counter++){
			size_low += 8;
			if (size_low == 0)
				size_high++;
		}

		dst[14] |= ((size_high >> 24) & 0xff) << 24;
		dst[14] |= ((size_high >> 16) & 0xff) << 16;
		dst[14] |= ((size_high >> 8) & 0xff) << 8;
		dst[14] |= ((size_high)& 0xff);

		dst[15] |= ((size_low >> 24) & 0xff) << 24;
		dst[15] |= ((size_low >> 16) & 0xff) << 16;
		dst[15] |= ((size_low >> 8) & 0xff) << 8;
		dst[15] |= ((size_low)& 0xff);
	}

	extern int sha1_hash(t_hash * hashout, unsigned int size, void const * datain)
	{
		std::uint32_t         tmp[64 + 16];
		unsigned char const * data;
		unsigned int          inc;
		unsigned int          orgSize;

		if (!hashout)
		{
			return -1;
		}
		if (size > 0 && !datain)
		{
			return -1;
		}

		hash_init(hashout);
		orgSize = size;

		data = (const unsigned char*)datain;
		while (size > 0)
		{
			if (size >= 64)
				inc = 64;
			else
				inc = size;

			if (size >= 64)
			{
				hash_set_16(tmp, data, inc, do_sha1_hash);
				do_hash(hashout, tmp, do_sha1_hash);
			}
			else if (size > 55){

				hash_set_16(tmp, data, inc, do_sha1_hash);
				do_hash(hashout, tmp, do_sha1_hash);

				// now use blizz variant as we only wanna fill in zeros
				hash_set_16(tmp, data, 0, do_blizzard_hash);
				hash_set_length(tmp, orgSize);
				do_hash(hashout, tmp, do_sha1_hash);
			}
			else{
				hash_set_16(tmp, data, inc, do_sha1_hash);
				hash_set_length(tmp, orgSize);
				do_hash(hashout, tmp, do_sha1_hash);
			}

			data += inc;
			size -= inc;
		}

		return 0;
	}

	extern int little_endian_sha1_hash(t_hash * hashout, unsigned int size, void const * datain)
	{
		bn_int value;
		unsigned int i;
		sha1_hash(hashout, size, datain);
		for (i = 0; i < 5; i++)
		{
			bn_int_nset(&value, (*hashout)[i]);
			(*hashout)[i] = bn_int_get(value);
		}
		return 0;
	}

	extern int hash_eq(t_hash const h1, t_hash const h2)
	{
		unsigned int i;

		if (!h1 || !h2)
		{
			return -1;
		}

		for (i = 0; i < 5; i++) {
			if (h1[i] != h2[i]) {
				return 0;
			}
		}

		return 1;
	}


	extern char const * hash_get_str(t_hash const hash)
	{
		static char  temp[8 * 5 + 1]; /* each of 5 ints to 8 chars + null */
		unsigned int i;

		if (!hash)
		{
			return NULL;
		}

		for (i = 0; i < 5; i++)
			std::sprintf(&temp[i * 8], "%08x", hash[i]);

		return temp;
	}

	extern char const * little_endian_hash_get_str(t_hash const hash)
	{
		bn_int value;
		t_hash be_hash;
		unsigned int i;
		for (i = 0; i < 5; i++)
		{
			bn_int_nset(&value, hash[i]);
			be_hash[i] = bn_int_get(value);
		}
		return hash_get_str(be_hash);
	}


	extern int hash_set_str(t_hash * hash, char const * str)
	{
		unsigned int i;

		if (!hash)
		{
			return -1;
		}
		if (!*hash)
		{
			return -1;
		}
		if (!str)
		{
			return -1;
		}
		if (std::strlen(str) != 5 * 8)
		{
			return -1;
		}

		for (i = 0; i < 5; i++)
		if (std::sscanf(&str[i * 8], "%8x", &(*hash)[i]) != 1)
		{
			return -1;
		}

		return 0;
	}

	void bnhash_to_hash(bn_int const * bnhash, t_hash * hash)
	{
		unsigned int i;

		if (!bnhash)
		{
			return;
		}
		if (!hash)
		{
			return;
		}

		for (i = 0; i < 5; i++)
			(*hash)[i] = bn_int_get(bnhash[i]);
	}


	void hash_to_bnhash(t_hash const * hash, bn_int * bnhash)
	{
		unsigned int i;

		if (!bnhash)
		{
			return;
		}
		if (!hash)
		{
			return;
		}

		for (i = 0; i < 5; i++)
			bn_int_set(&bnhash[i], (*hash)[i]);
	}

}

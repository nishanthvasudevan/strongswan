/**
 * @file diffie_hellman.c
 * 
 * @brief Class to represent a diffie hellman exchange.
 * 
 */

/*
 * Copyright (C) 1998-2002  D. Hugh Redelmeier.
 * Copyright (C) 1999, 2000, 2001  Henry Spencer.
 * Copyright (C) 2005 Jan Hutter, Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <gmp.h> 
#include <stdio.h> 

#include "diffie_hellman.h"

#include <utils/allocator.h>
#include <utils/randomizer.h>
#include <utils/gmp_helper.h>


/**
 * Modulus of Group 1 (MODP_768_BIT)
 */
static u_int8_t group1_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80	,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x3A,0x36,0x20,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/**
 * Modulus of Group 2 (MODP_1024_BIT)
 */
static u_int8_t group2_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE6,0x53,0x81,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/**
 * Modulus of Group 5 (MODP_1536_BIT)
 */
static u_int8_t group5_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
	0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
	0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
	0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
	0xF1,0x74,0x6C,0x08,0xCA,0x23,0x73,0x27,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
/**
 * Modulus of Group 14 (MODP_2048_BIT)
 */
static u_int8_t group14_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
	0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
	0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
	0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
	0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
	0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
	0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
	0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
	0x15,0x72,0x8E,0x5A,0x8A,0xAC,0xAA,0x68,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/**
 * Modulus of Group 15 (MODP_3072_BIT)
 */
static u_int8_t group15_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
	0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
	0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
	0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
	0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
	0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
	0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
	0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
	0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
	0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
	0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
	0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
	0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
	0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
	0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
	0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
	0x4B,0x82,0xD1,0x20,0xA9,0x3A,0xD2,0xCA,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/**
 * Modulus of Group 16 (MODP_4096_BIT)
 */
static u_int8_t group16_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
	0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
	0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
	0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
	0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
	0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
	0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
	0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
	0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
	0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
	0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
	0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
	0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
	0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
	0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
	0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
	0x4B,0x82,0xD1,0x20,0xA9,0x21,0x08,0x01,0x1A,0x72,0x3C,0x12,0xA7,0x87,0xE6,0xD7,
	0x88,0x71,0x9A,0x10,0xBD,0xBA,0x5B,0x26,0x99,0xC3,0x27,0x18,0x6A,0xF4,0xE2,0x3C,
	0x1A,0x94,0x68,0x34,0xB6,0x15,0x0B,0xDA,0x25,0x83,0xE9,0xCA,0x2A,0xD4,0x4C,0xE8,
	0xDB,0xBB,0xC2,0xDB,0x04,0xDE,0x8E,0xF9,0x2E,0x8E,0xFC,0x14,0x1F,0xBE,0xCA,0xA6,
	0x28,0x7C,0x59,0x47,0x4E,0x6B,0xC0,0x5D,0x99,0xB2,0x96,0x4F,0xA0,0x90,0xC3,0xA2,
	0x23,0x3B,0xA1,0x86,0x51,0x5B,0xE7,0xED,0x1F,0x61,0x29,0x70,0xCE,0xE2,0xD7,0xAF,
	0xB8,0x1B,0xDD,0x76,0x21,0x70,0x48,0x1C,0xD0,0x06,0x91,0x27,0xD5,0xB0,0x5A,0xA9,
	0x93,0xB4,0xEA,0x98,0x8D,0x8F,0xDD,0xC1,0x86,0xFF,0xB7,0xDC,0x90,0xA6,0xC0,0x8F,
	0x4D,0xF4,0x35,0xC9,0x34,0x06,0x31,0x99,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/**
 * Modulus of Group 17 (MODP_6144_BIT)
 */
static u_int8_t group17_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
	0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
	0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
	0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
	0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
	0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
	0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
	0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
	0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
	0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
	0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
	0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
	0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
	0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
	0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
	0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
	0x4B,0x82,0xD1,0x20,0xA9,0x21,0x08,0x01,0x1A,0x72,0x3C,0x12,0xA7,0x87,0xE6,0xD7,
	0x88,0x71,0x9A,0x10,0xBD,0xBA,0x5B,0x26,0x99,0xC3,0x27,0x18,0x6A,0xF4,0xE2,0x3C,
	0x1A,0x94,0x68,0x34,0xB6,0x15,0x0B,0xDA,0x25,0x83,0xE9,0xCA,0x2A,0xD4,0x4C,0xE8,
	0xDB,0xBB,0xC2,0xDB,0x04,0xDE,0x8E,0xF9,0x2E,0x8E,0xFC,0x14,0x1F,0xBE,0xCA,0xA6,
	0x28,0x7C,0x59,0x47,0x4E,0x6B,0xC0,0x5D,0x99,0xB2,0x96,0x4F,0xA0,0x90,0xC3,0xA2,
	0x23,0x3B,0xA1,0x86,0x51,0x5B,0xE7,0xED,0x1F,0x61,0x29,0x70,0xCE,0xE2,0xD7,0xAF,
	0xB8,0x1B,0xDD,0x76,0x21,0x70,0x48,0x1C,0xD0,0x06,0x91,0x27,0xD5,0xB0,0x5A,0xA9,
	0x93,0xB4,0xEA,0x98,0x8D,0x8F,0xDD,0xC1,0x86,0xFF,0xB7,0xDC,0x90,0xA6,0xC0,0x8F,
	0x4D,0xF4,0x35,0xC9,0x34,0x02,0x84,0x92,0x36,0xC3,0xFA,0xB4,0xD2,0x7C,0x70,0x26,
	0xC1,0xD4,0xDC,0xB2,0x60,0x26,0x46,0xDE,0xC9,0x75,0x1E,0x76,0x3D,0xBA,0x37,0xBD,
	0xF8,0xFF,0x94,0x06,0xAD,0x9E,0x53,0x0E,0xE5,0xDB,0x38,0x2F,0x41,0x30,0x01,0xAE,
	0xB0,0x6A,0x53,0xED,0x90,0x27,0xD8,0x31,0x17,0x97,0x27,0xB0,0x86,0x5A,0x89,0x18,
	0xDA,0x3E,0xDB,0xEB,0xCF,0x9B,0x14,0xED,0x44,0xCE,0x6C,0xBA,0xCE,0xD4,0xBB,0x1B,
	0xDB,0x7F,0x14,0x47,0xE6,0xCC,0x25,0x4B,0x33,0x20,0x51,0x51,0x2B,0xD7,0xAF,0x42,
	0x6F,0xB8,0xF4,0x01,0x37,0x8C,0xD2,0xBF,0x59,0x83,0xCA,0x01,0xC6,0x4B,0x92,0xEC,
	0xF0,0x32,0xEA,0x15,0xD1,0x72,0x1D,0x03,0xF4,0x82,0xD7,0xCE,0x6E,0x74,0xFE,0xF6,
	0xD5,0x5E,0x70,0x2F,0x46,0x98,0x0C,0x82,0xB5,0xA8,0x40,0x31,0x90,0x0B,0x1C,0x9E,
	0x59,0xE7,0xC9,0x7F,0xBE,0xC7,0xE8,0xF3,0x23,0xA9,0x7A,0x7E,0x36,0xCC,0x88,0xBE,
	0x0F,0x1D,0x45,0xB7,0xFF,0x58,0x5A,0xC5,0x4B,0xD4,0x07,0xB2,0x2B,0x41,0x54,0xAA,
	0xCC,0x8F,0x6D,0x7E,0xBF,0x48,0xE1,0xD8,0x14,0xCC,0x5E,0xD2,0x0F,0x80,0x37,0xE0,
	0xA7,0x97,0x15,0xEE,0xF2,0x9B,0xE3,0x28,0x06,0xA1,0xD5,0x8B,0xB7,0xC5,0xDA,0x76,
	0xF5,0x50,0xAA,0x3D,0x8A,0x1F,0xBF,0xF0,0xEB,0x19,0xCC,0xB1,0xA3,0x13,0xD5,0x5C,
	0xDA,0x56,0xC9,0xEC,0x2E,0xF2,0x96,0x32,0x38,0x7F,0xE8,0xD7,0x6E,0x3C,0x04,0x68,
	0x04,0x3E,0x8F,0x66,0x3F,0x48,0x60,0xEE,0x12,0xBF,0x2D,0x5B,0x0B,0x74,0x74,0xD6,
	0xE6,0x94,0xF9,0x1E,0x6D,0xCC,0x40,0x24,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

/**
 * Modulus of Group 18 (MODP_8192_BIT)
 */
static u_int8_t group18_modulus[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC9,0x0F,0xDA,0xA2,0x21,0x68,0xC2,0x34,
	0xC4,0xC6,0x62,0x8B,0x80,0xDC,0x1C,0xD1,0x29,0x02,0x4E,0x08,0x8A,0x67,0xCC,0x74,
	0x02,0x0B,0xBE,0xA6,0x3B,0x13,0x9B,0x22,0x51,0x4A,0x08,0x79,0x8E,0x34,0x04,0xDD,
	0xEF,0x95,0x19,0xB3,0xCD,0x3A,0x43,0x1B,0x30,0x2B,0x0A,0x6D,0xF2,0x5F,0x14,0x37,
	0x4F,0xE1,0x35,0x6D,0x6D,0x51,0xC2,0x45,0xE4,0x85,0xB5,0x76,0x62,0x5E,0x7E,0xC6,
	0xF4,0x4C,0x42,0xE9,0xA6,0x37,0xED,0x6B,0x0B,0xFF,0x5C,0xB6,0xF4,0x06,0xB7,0xED,
	0xEE,0x38,0x6B,0xFB,0x5A,0x89,0x9F,0xA5,0xAE,0x9F,0x24,0x11,0x7C,0x4B,0x1F,0xE6,
	0x49,0x28,0x66,0x51,0xEC,0xE4,0x5B,0x3D,0xC2,0x00,0x7C,0xB8,0xA1,0x63,0xBF,0x05,
	0x98,0xDA,0x48,0x36,0x1C,0x55,0xD3,0x9A,0x69,0x16,0x3F,0xA8,0xFD,0x24,0xCF,0x5F,
	0x83,0x65,0x5D,0x23,0xDC,0xA3,0xAD,0x96,0x1C,0x62,0xF3,0x56,0x20,0x85,0x52,0xBB,
	0x9E,0xD5,0x29,0x07,0x70,0x96,0x96,0x6D,0x67,0x0C,0x35,0x4E,0x4A,0xBC,0x98,0x04,
	0xF1,0x74,0x6C,0x08,0xCA,0x18,0x21,0x7C,0x32,0x90,0x5E,0x46,0x2E,0x36,0xCE,0x3B,
	0xE3,0x9E,0x77,0x2C,0x18,0x0E,0x86,0x03,0x9B,0x27,0x83,0xA2,0xEC,0x07,0xA2,0x8F,
	0xB5,0xC5,0x5D,0xF0,0x6F,0x4C,0x52,0xC9,0xDE,0x2B,0xCB,0xF6,0x95,0x58,0x17,0x18,
	0x39,0x95,0x49,0x7C,0xEA,0x95,0x6A,0xE5,0x15,0xD2,0x26,0x18,0x98,0xFA,0x05,0x10,
	0x15,0x72,0x8E,0x5A,0x8A,0xAA,0xC4,0x2D,0xAD,0x33,0x17,0x0D,0x04,0x50,0x7A,0x33,
	0xA8,0x55,0x21,0xAB,0xDF,0x1C,0xBA,0x64,0xEC,0xFB,0x85,0x04,0x58,0xDB,0xEF,0x0A,
	0x8A,0xEA,0x71,0x57,0x5D,0x06,0x0C,0x7D,0xB3,0x97,0x0F,0x85,0xA6,0xE1,0xE4,0xC7,
	0xAB,0xF5,0xAE,0x8C,0xDB,0x09,0x33,0xD7,0x1E,0x8C,0x94,0xE0,0x4A,0x25,0x61,0x9D,
	0xCE,0xE3,0xD2,0x26,0x1A,0xD2,0xEE,0x6B,0xF1,0x2F,0xFA,0x06,0xD9,0x8A,0x08,0x64,
	0xD8,0x76,0x02,0x73,0x3E,0xC8,0x6A,0x64,0x52,0x1F,0x2B,0x18,0x17,0x7B,0x20,0x0C,
	0xBB,0xE1,0x17,0x57,0x7A,0x61,0x5D,0x6C,0x77,0x09,0x88,0xC0,0xBA,0xD9,0x46,0xE2,
	0x08,0xE2,0x4F,0xA0,0x74,0xE5,0xAB,0x31,0x43,0xDB,0x5B,0xFC,0xE0,0xFD,0x10,0x8E,
	0x4B,0x82,0xD1,0x20,0xA9,0x21,0x08,0x01,0x1A,0x72,0x3C,0x12,0xA7,0x87,0xE6,0xD7,
	0x88,0x71,0x9A,0x10,0xBD,0xBA,0x5B,0x26,0x99,0xC3,0x27,0x18,0x6A,0xF4,0xE2,0x3C,
	0x1A,0x94,0x68,0x34,0xB6,0x15,0x0B,0xDA,0x25,0x83,0xE9,0xCA,0x2A,0xD4,0x4C,0xE8,
	0xDB,0xBB,0xC2,0xDB,0x04,0xDE,0x8E,0xF9,0x2E,0x8E,0xFC,0x14,0x1F,0xBE,0xCA,0xA6,
	0x28,0x7C,0x59,0x47,0x4E,0x6B,0xC0,0x5D,0x99,0xB2,0x96,0x4F,0xA0,0x90,0xC3,0xA2,
	0x23,0x3B,0xA1,0x86,0x51,0x5B,0xE7,0xED,0x1F,0x61,0x29,0x70,0xCE,0xE2,0xD7,0xAF,
	0xB8,0x1B,0xDD,0x76,0x21,0x70,0x48,0x1C,0xD0,0x06,0x91,0x27,0xD5,0xB0,0x5A,0xA9,
	0x93,0xB4,0xEA,0x98,0x8D,0x8F,0xDD,0xC1,0x86,0xFF,0xB7,0xDC,0x90,0xA6,0xC0,0x8F,
	0x4D,0xF4,0x35,0xC9,0x34,0x02,0x84,0x92,0x36,0xC3,0xFA,0xB4,0xD2,0x7C,0x70,0x26,
	0xC1,0xD4,0xDC,0xB2,0x60,0x26,0x46,0xDE,0xC9,0x75,0x1E,0x76,0x3D,0xBA,0x37,0xBD,
	0xF8,0xFF,0x94,0x06,0xAD,0x9E,0x53,0x0E,0xE5,0xDB,0x38,0x2F,0x41,0x30,0x01,0xAE,
	0xB0,0x6A,0x53,0xED,0x90,0x27,0xD8,0x31,0x17,0x97,0x27,0xB0,0x86,0x5A,0x89,0x18,
	0xDA,0x3E,0xDB,0xEB,0xCF,0x9B,0x14,0xED,0x44,0xCE,0x6C,0xBA,0xCE,0xD4,0xBB,0x1B,
	0xDB,0x7F,0x14,0x47,0xE6,0xCC,0x25,0x4B,0x33,0x20,0x51,0x51,0x2B,0xD7,0xAF,0x42,
	0x6F,0xB8,0xF4,0x01,0x37,0x8C,0xD2,0xBF,0x59,0x83,0xCA,0x01,0xC6,0x4B,0x92,0xEC,
	0xF0,0x32,0xEA,0x15,0xD1,0x72,0x1D,0x03,0xF4,0x82,0xD7,0xCE,0x6E,0x74,0xFE,0xF6,
	0xD5,0x5E,0x70,0x2F,0x46,0x98,0x0C,0x82,0xB5,0xA8,0x40,0x31,0x90,0x0B,0x1C,0x9E,
	0x59,0xE7,0xC9,0x7F,0xBE,0xC7,0xE8,0xF3,0x23,0xA9,0x7A,0x7E,0x36,0xCC,0x88,0xBE,
	0x0F,0x1D,0x45,0xB7,0xFF,0x58,0x5A,0xC5,0x4B,0xD4,0x07,0xB2,0x2B,0x41,0x54,0xAA,
	0xCC,0x8F,0x6D,0x7E,0xBF,0x48,0xE1,0xD8,0x14,0xCC,0x5E,0xD2,0x0F,0x80,0x37,0xE0,
	0xA7,0x97,0x15,0xEE,0xF2,0x9B,0xE3,0x28,0x06,0xA1,0xD5,0x8B,0xB7,0xC5,0xDA,0x76,
	0xF5,0x50,0xAA,0x3D,0x8A,0x1F,0xBF,0xF0,0xEB,0x19,0xCC,0xB1,0xA3,0x13,0xD5,0x5C,
	0xDA,0x56,0xC9,0xEC,0x2E,0xF2,0x96,0x32,0x38,0x7F,0xE8,0xD7,0x6E,0x3C,0x04,0x68,
	0x04,0x3E,0x8F,0x66,0x3F,0x48,0x60,0xEE,0x12,0xBF,0x2D,0x5B,0x0B,0x74,0x74,0xD6,
	0xE6,0x94,0xF9,0x1E,0x6D,0xBE,0x11,0x59,0x74,0xA3,0x92,0x6F,0x12,0xFE,0xE5,0xE4,
	0x38,0x77,0x7C,0xB6,0xA9,0x32,0xDF,0x8C,0xD8,0xBE,0xC4,0xD0,0x73,0xB9,0x31,0xBA,
	0x3B,0xC8,0x32,0xB6,0x8D,0x9D,0xD3,0x00,0x74,0x1F,0xA7,0xBF,0x8A,0xFC,0x47,0xED,
	0x25,0x76,0xF6,0x93,0x6B,0xA4,0x24,0x66,0x3A,0xAB,0x63,0x9C,0x5A,0xE4,0xF5,0x68,
	0x34,0x23,0xB4,0x74,0x2B,0xF1,0xC9,0x78,0x23,0x8F,0x16,0xCB,0xE3,0x9D,0x65,0x2D,
	0xE3,0xFD,0xB8,0xBE,0xFC,0x84,0x8A,0xD9,0x22,0x22,0x2E,0x04,0xA4,0x03,0x7C,0x07,
	0x13,0xEB,0x57,0xA8,0x1A,0x23,0xF0,0xC7,0x34,0x73,0xFC,0x64,0x6C,0xEA,0x30,0x6B,
	0x4B,0xCB,0xC8,0x86,0x2F,0x83,0x85,0xDD,0xFA,0x9D,0x4B,0x7F,0xA2,0xC0,0x87,0xE8,
	0x79,0x68,0x33,0x03,0xED,0x5B,0xDD,0x3A,0x06,0x2B,0x3C,0xF5,0xB3,0xA2,0x78,0xA6,
	0x6D,0x2A,0x13,0xF8,0x3F,0x44,0xF8,0x2D,0xDF,0x31,0x0E,0xE0,0x74,0xAB,0x6A,0x36,
	0x45,0x97,0xE8,0x99,0xA0,0x25,0x5D,0xC1,0x64,0xF3,0x1C,0xC5,0x08,0x46,0x85,0x1D,
	0xF9,0xAB,0x48,0x19,0x5D,0xED,0x7E,0xA1,0xB1,0xD5,0x10,0xBD,0x7E,0xE7,0x4D,0x73,
	0xFA,0xF3,0x6B,0xC3,0x1E,0xCF,0xA2,0x68,0x35,0x90,0x46,0xF4,0xEB,0x87,0x9F,0x92,
	0x40,0x09,0x43,0x8B,0x48,0x1C,0x6C,0xD7,0x88,0x9A,0x00,0x2E,0xD5,0xEE,0x38,0x2B,
	0xC9,0x19,0x0D,0xA6,0xFC,0x02,0x6E,0x47,0x95,0x58,0xE4,0x47,0x56,0x77,0xE9,0xAA,
	0x9E,0x30,0x50,0xE2,0x76,0x56,0x94,0xDF,0xC8,0x1F,0x56,0xE8,0x80,0xB9,0x6E,0x71,
	0x60,0xC9,0x80,0xDD,0x98,0xED,0xD3,0xDF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

typedef struct modulus_info_entry_t modulus_info_entry_t;

/** 
 * Entry of the modulus list
 */
struct modulus_info_entry_t {
	/**
	 * Group number as it is defined in transform_substructure.h
	 */
	diffie_hellman_group_t group;
	
	/**
	 * Pointer to first byte of modulus in (network order)
	 */
	u_int8_t *modulus;
	
	/* 
	 * Length of modulus in bytes
	 */	
	size_t modulus_length;
	
	/* 
	 * Generator value
	 */	
	u_int16_t generator;
};


static modulus_info_entry_t modulus_info_entries[] = {
	{MODP_768_BIT,group1_modulus,sizeof(group1_modulus),2},
	{MODP_1024_BIT,group2_modulus,sizeof(group2_modulus),2},
	{MODP_1536_BIT,group5_modulus,sizeof(group5_modulus),2},
	{MODP_2048_BIT,group14_modulus,sizeof(group14_modulus),2},
	{MODP_3072_BIT,group15_modulus,sizeof(group15_modulus),2},
	{MODP_4096_BIT,group16_modulus,sizeof(group16_modulus),2},
	{MODP_6144_BIT,group17_modulus,sizeof(group17_modulus),2},
	{MODP_8192_BIT,group18_modulus,sizeof(group18_modulus),2},
};

typedef struct private_diffie_hellman_t private_diffie_hellman_t;

/**
 * Private data of an diffie_hellman_t object.
 * 
 */
struct private_diffie_hellman_t {
	/**
	 * public diffie_hellman_t interface
	 */
	diffie_hellman_t public;
	
	/**
	 * Diffie Hellman group number
	 */
	u_int16_t dh_group_number;

	/**
	 * Modulus
	 */
	mpz_t modulus;
	
	/**
	 * Modulus length
	 */
	size_t modulus_length;
	
	/* 
	 * Generator value
	 */	
	u_int16_t generator;

	/**
	 * My prime 
	 */
	mpz_t my_prime;
	
	/**
	 * My public value
	 */
	mpz_t my_public_value;

	/**
	 * Other public value
	 */	
	mpz_t other_public_value;
	
	/**
	 * Shared secret
	 */	
	mpz_t shared_secret;

	/**
	 * True if public modulus is computed and stored in my_public_value
	 */
	bool my_public_value_is_computed;

	/**
	 * True if shared secret is computed and stored in my_public_value
	 */
	bool shared_secret_is_computed;

	/**
	 * helper class for gmp functions
	 */	
	gmp_helper_t *gmp_helper;
	
	/**
	 * Sets the modulus for a specific diffie hellman group
	 * 
	 * @param this			calling object
	 * @return
	 * 						SUCCESS if modulus could be found
	 * 						NOT_FOUND if modulus not supported
	 */
	status_t (*set_modulus) (private_diffie_hellman_t *this);
	
	/**
	 * Makes sure my public value is computed
	 * 
	 * @param this			calling object
	 */
	void (*compute_public_value) (private_diffie_hellman_t *this);

	/**
	 * Computes shared secret (other public value must be available)
	 * 
	 * @param this			calling object
	 */
	void (*compute_shared_secret) (private_diffie_hellman_t *this);
};

/**
 * Implements private_diffie_hellman_t's set_modulus function.
 * See #private_diffie_hellman_t.set_modulus for description.
 */
static status_t set_modulus(private_diffie_hellman_t *this)
{
	int i;
	status_t status = NOT_FOUND;
	
	for (i = 0; i < (sizeof(modulus_info_entries) / sizeof(modulus_info_entry_t)); i++)
	{
		if (modulus_info_entries[i].group == this->dh_group_number)
		{
			chunk_t modulus_chunk;
			modulus_chunk.ptr = modulus_info_entries[i].modulus;
			modulus_chunk.len = modulus_info_entries[i].modulus_length;
			this->gmp_helper->chunk_to_mpz(this->gmp_helper,&(this->modulus),modulus_chunk);
			this->modulus_length = modulus_chunk.len;
			this->generator = modulus_info_entries[i].generator;
			status = SUCCESS;
			break;
		}
	}
	return status;
}

/**
 * Implements diffie_hellman_t's set_other_public_value function.
 * See #diffie_hellman_t.set_other_public_value for description.
 */
static status_t set_other_public_value(private_diffie_hellman_t *this,chunk_t public_value)
{
	this->gmp_helper->chunk_to_mpz(this->gmp_helper,&(this->other_public_value),public_value);
	this->compute_shared_secret(this);		
	return SUCCESS;
}

/**
 * Implements diffie_hellman_t's get_other_public_value function.
 * See #diffie_hellman_t.get_other_public_value for description.
 */
static status_t get_other_public_value(private_diffie_hellman_t *this,chunk_t *public_value)
{
	if (!this->shared_secret_is_computed)
	{
		return FAILED;
	}
	return (this->gmp_helper->mpz_to_chunk(this->gmp_helper,&(this->other_public_value), public_value,this->modulus_length));
}

/**
 * Implements private_diffie_hellman_t's compute_shared_secret function.
 * See #private_diffie_hellman_t.compute_shared_secret for description.
 */
static void compute_shared_secret (private_diffie_hellman_t *this)
{
	/* initialize my public value */
	mpz_init(this->shared_secret);
	/* calculate my public value */
	mpz_powm(this->shared_secret,this->other_public_value,this->my_prime,this->modulus);

	this->shared_secret_is_computed = TRUE;
}


/**
 * Implements private_diffie_hellman_t's compute_public_value function.
 * See #private_diffie_hellman_t.compute_public_value for description.
 */
static void compute_public_value (private_diffie_hellman_t *this)
{
	mpz_t generator;
	/* initialize generator and set it*/
	mpz_init_set_ui (generator,this->generator);
	/* initialize my public value */
	mpz_init(this->my_public_value);
	/* calculate my public value */
	mpz_powm(this->my_public_value,generator,this->my_prime,this->modulus);
	/* generator not used anymore */
	mpz_clear(generator);
	this->my_public_value_is_computed = TRUE;
}

/**
 * Implements diffie_hellman_t's get_my_public_value function.
 * See #diffie_hellman_t.get_my_public_value for description.
 */
static status_t get_my_public_value(private_diffie_hellman_t *this,chunk_t *public_value)
{
	if (this->my_public_value_is_computed == FALSE)
	{
		this->compute_public_value(this);
	}
	return (this->gmp_helper->mpz_to_chunk(this->gmp_helper,&(this->my_public_value), public_value,this->modulus_length));
}

/**
 * Implements diffie_hellman_t's get_shared_secret function.
 * See #diffie_hellman_t.get_shared_secret for description.
 */
static status_t get_shared_secret(private_diffie_hellman_t *this,chunk_t *secret)
{
	if (!this->shared_secret_is_computed)
	{
		return FAILED;
	}
	return (this->gmp_helper->mpz_to_chunk(this->gmp_helper,&(this->shared_secret), secret,this->modulus_length));
}

/**
 * Implements diffie_hellman_t's destroy function.
 * See #diffie_hellman_t.destroy for description.
 */
static status_t destroy(private_diffie_hellman_t *this)
{
	this->gmp_helper->destroy(this->gmp_helper);
	mpz_clear(this->modulus);
	mpz_clear(this->my_prime);
	if (this->my_public_value_is_computed)
	{
		mpz_clear(this->my_public_value);
	}
	if (this->shared_secret_is_computed)
	{
		/* other public value gets initialized together with shared secret */
		mpz_clear(this->other_public_value);
		mpz_clear(this->shared_secret);
	}

	allocator_free(this);
	return SUCCESS;
}


/*
 * Described in header
 */
diffie_hellman_t *diffie_hellman_create(diffie_hellman_group_t dh_group_number)
{
	private_diffie_hellman_t *this = allocator_alloc_thing(private_diffie_hellman_t);
	if ((this == NULL))
	{
		return NULL;
	}
	
	/* public functions */
	this->public.get_shared_secret = (status_t (*)(diffie_hellman_t *, chunk_t *)) get_shared_secret;
	this->public.set_other_public_value = (status_t (*)(diffie_hellman_t *, chunk_t )) set_other_public_value;
	this->public.get_other_public_value = (status_t (*)(diffie_hellman_t *, chunk_t *)) get_other_public_value;
	this->public.get_my_public_value = (status_t (*)(diffie_hellman_t *, chunk_t *)) get_my_public_value;
	this->public.destroy = (status_t (*)(diffie_hellman_t *)) destroy;
	
	/* private functions */
	this->set_modulus = set_modulus;
	this->compute_public_value = compute_public_value;
	this->compute_shared_secret = compute_shared_secret;
	
	/* private variables */
	this->dh_group_number = dh_group_number;
	
	this->gmp_helper = gmp_helper_create();
	
	if (this->gmp_helper == NULL)
	{
		allocator_free(this);
		return NULL;
	}

	/* set this->modulus */	
	if (this->set_modulus(this) != SUCCESS)
	{
		this->gmp_helper->destroy(this->gmp_helper);
		allocator_free(this);
		return NULL;
	}
	    
	if (this->gmp_helper->init_prime(this->gmp_helper,&(this->my_prime),this->modulus_length) != SUCCESS)
	{
		this->gmp_helper->destroy(this->gmp_helper);
		allocator_free(this);
		return NULL;
	}
	this->my_public_value_is_computed = FALSE;
	this->shared_secret_is_computed = FALSE;
	
	return &(this->public);
}

// This part is originally created and released into the public
// domain by Gunnar Selke <gselke@physnet.uni-hamburg.de>.

#ifndef ENDIAN_H
#define ENDIAN_H

//#include <config.h>
#include <stdint.h>
#include <cassert>

/** @file 
 * Miscellanous functions as well as the RESTRICT macro.
 */ 

/**
 * Swaps the endianness (byte order) of a value.
 */
template <typename T>
T swapEndianness(T value)
{
	T result;

	char *src = (char*)&value;
	char *dst = (char*)&result;

	for (size_t i=0; i<sizeof(T); ++i) {
		dst[i] = src[sizeof(T)-i-1];
	}

	return result;
}

/**
 * Returns if this architecture is a big-endian or little-endian system.
 */
inline bool isBigEndian()
{
	uint32_t x = 0x11223344;
	uint8_t x1 = ((uint8_t*)&x)[0];
	uint8_t x2 = ((uint8_t*)&x)[1];
	uint8_t x3 = ((uint8_t*)&x)[2];
	uint8_t x4 = ((uint8_t*)&x)[3];
	if (x1 == 0x11 && x2 == 0x22 && x3 == 0x33 && x4 == 0x44) {
		return true; // big endian
	} else if (x1 == 0x44 && x2 == 0x33 && x3 == 0x22 && x4 == 0x11) {
		return false; // little endian
	} else {
		assert(0 && "Couldnt determine endianness of architecture!");
	}
}

/**
 * Converts the value into big endian byte order (network byte order). Does
 * nothing if the architecture is already big-endian.
 */
template <typename T>
T toBigEndian(T value)
{
	if (isBigEndian()) return value;
	return swapEndianness(value);
}

/**
 * Converts a value in big-endian byte order to host-byte-order.
 */
template <typename T>
T fromBigEndian(T value)
{
	if (isBigEndian()) return value;
	return swapEndianness(value);
}

/**
 * Converts a value in little-endian byte order to host-byte-order.
 */
template <typename T>
T fromLittleEndian(T value)
{
	if (!isBigEndian()) return value;
	return swapEndianness(value);
}

#endif


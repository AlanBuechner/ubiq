#include "Hash.h"

uint32_t crc32(const void* buf, size_t size)
{
	const uint8_t* p = (uint8_t*)buf;
	uint32_t crc;

	crc = ~0U;
	while (size--)
		crc = crc_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return crc ^ ~0U;
}

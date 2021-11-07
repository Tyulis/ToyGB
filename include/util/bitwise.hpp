#ifndef _UTIL_BITWISE_HPP
#define _UTIL_BITWISE_HPP


namespace toygb {
	uint8_t bitset(uint8_t value, int bit);
	uint8_t bitset(uint8_t* value, int bit);
	uint8_t bitreset(uint8_t value, int bit);
	uint8_t bitreset(uint8_t* value, int bit);
}

#endif

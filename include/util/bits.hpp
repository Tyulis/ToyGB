#ifndef _UTIL_BITS_HPP
#define _UTIL_BITS_HPP

// This contraption just tells in a rather optimized/obscure way which bits were changed from low to high :
// old XOR new = changed bits set, unchanged bits clear (01010101 XOR 01010110 -> 00000011)
// (old XOR new) AND new = changed bits that are now set are set, all others are clear : 00000011 & 01010110 = 00000010
#define LOW_TO_HIGH(oldvalue, newvalue) (((oldvalue) ^ (newvalue)) & (newvalue))
// Same but AND-ed with (NOT new) to get bits that went from high to low instead of low to high
#define HIGH_TO_LOW(oldvalue, newvalue) (((oldvalue) ^ (newvalue)) & (~(newvalue)))


#endif

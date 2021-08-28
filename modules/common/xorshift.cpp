#include "xorshift.h"

/* The state word must be initialized to non-zero */
uint32_t XorShift32::GetU()
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint32_t x = a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return a = x;
}

int32_t XorShift32::Get()
{
	uint32_t x = a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return (a = x);
}
#ifndef XORSHIFT_H_GUARD
#define XORSHIFT_H_GUARD
#include <cstdint>

struct XorShift32{
	uint32_t a = 1;
	uint32_t GetU();
	int32_t Get();
};

extern XorShift32 xorShift32;

#endif /* XORSHIFT_H_GUARD */

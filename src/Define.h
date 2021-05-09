#pragma once
#include <stdint.h>
#include <intrin.h>

typedef size_t mem_size_t;

constexpr mem_size_t kMaxSize = 0xffffffff;

#define BYTE * 1
#define KB BYTE * 1024
#define MB KB * 1024
#define GB MB * 1024

constexpr mem_size_t kAlignment = 16 BYTE;
constexpr mem_size_t kPageSize = 4 KB;

enum class PlacementPolicy
{
	kFirstFit,
	kNextFit,
	kBestFit
};

enum class CoalescingPolicy
{
	kImmediate,
	kDeferred
};

inline mem_size_t RoundUp(const mem_size_t& alignment, const mem_size_t& size) noexcept
{
	return (size + alignment - 1) & ~(alignment - 1);
}

inline mem_size_t FindLastBitSetImpl(mem_size_t bits) noexcept
{
	mem_size_t bit = 64;

	if (!bits) bit -= 1;
	if (!(bits & 0xffffffff00000000)) { bits <<= 32; bit -= 32; }
	if (!(bits & 0xffff000000000000)) { bits <<= 16; bit -= 16; }
	if (!(bits & 0xff00000000000000)) { bits <<= 8; bit -= 8; }
	if (!(bits & 0xf000000000000000)) { bits <<= 4; bit -= 4; }
	if (!(bits & 0xc000000000000000)) { bits <<= 2; bit -= 2; }
	if (!(bits & 0x8000000000000000)) { bits <<= 1; bit -= 1; }

	return bit;
}

inline mem_size_t FindFirstBitSet(mem_size_t bits) noexcept
{
	return FindLastBitSetImpl(bits & (~bits + 1)) - 1;
}

inline mem_size_t FindLastBitSet(mem_size_t bits) noexcept
{
	return FindLastBitSetImpl(bits) - 1;
}
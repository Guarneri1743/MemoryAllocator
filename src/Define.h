#pragma once

typedef size_t mem_size_t;
typedef unsigned char byte;
#define MAX_SIZE ((mem_size_t)0xffffffffffffffff)
#define FLAG_MASK ((mem_size_t)0x1)

#define BYTE * (mem_size_t)1
#define KB BYTE * (mem_size_t)1024
#define MB KB * (mem_size_t)1024
#define GB MB * (mem_size_t)1024

constexpr mem_size_t ALIGNMENT = 16 BYTE;
constexpr mem_size_t PAGE_SIZE = 4 KB;

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
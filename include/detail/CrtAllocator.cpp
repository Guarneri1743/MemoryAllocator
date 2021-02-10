#include "CrtAllocator.h"
#include <stdlib.h>

CrtAllocator::CrtAllocator() : Allocator(0, ALIGNMENT, PAGE_SIZE, AllocationPolicy::Dynamic)
{
	capacity_ = 0;
}

CrtAllocator::~CrtAllocator()
{}

void* CrtAllocator::Allocate(const size_type& size)
{
	return malloc(size);
}

void CrtAllocator::Free(void* ptr)
{
	free(ptr);
}
#include "DefaultAllocator.h"
#include <stdlib.h>

DefaultAllocator::DefaultAllocator() : Allocator(0, ALIGNMENT, PAGE_SIZE, AllocationPolicy::Dynamic)
{
	capacity_ = 0;
}

DefaultAllocator::~DefaultAllocator()
{}

void* DefaultAllocator::Allocate(const size_type& size)
{
	return malloc(size);
}

void DefaultAllocator::Free(void* ptr)
{
	free(ptr);
}
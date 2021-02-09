#pragma once
#include "Allocator.h"

template<size_type kPageSize = PAGE_SIZE, size_type kAlignment = ALIGNMENT>
	class DefaultAllocator : public Allocator
{
public:
	DefaultAllocator();
	~DefaultAllocator();
	void* Allocate(const size_type& size);
	void Free(void* ptr);
};

#include "detail/DefaultAllocator.inl"

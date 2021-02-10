#pragma once
#include "Allocator.h"

class DefaultAllocator : public Allocator
{
public:
	DefaultAllocator();
	~DefaultAllocator();

	void* Allocate(const size_type& size);
	void Free(void* ptr);
};
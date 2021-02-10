#pragma once
#include "Allocator.h"

class CrtAllocator : public Allocator
{
public:
	CrtAllocator();
	~CrtAllocator();

	void* Allocate(const size_type& size);
	void Free(void* ptr);
};
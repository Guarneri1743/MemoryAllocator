#pragma once
#include "Define.h"

class CrtAllocator 
{
public:
	CrtAllocator();
	~CrtAllocator();

	void* Allocate(const mem_size_t& size);
	void Free(void* ptr);
};
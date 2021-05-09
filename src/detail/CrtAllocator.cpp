#include "CrtAllocator.h"
#include <stdlib.h>

CrtAllocator::CrtAllocator() 
{
}

CrtAllocator::~CrtAllocator()
{}

void* CrtAllocator::Allocate(const mem_size_t& size)
{
	return malloc(size);
}

void CrtAllocator::Free(void* ptr)
{
	free(ptr);
}
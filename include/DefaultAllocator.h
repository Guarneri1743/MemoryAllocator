#pragma once
#include "Allocator.h"

template<
	size_type kPageSize = PAGE_SIZE,
	size_type kAlignment = ALIGNMENT>
	class DefaultAllocator : public Allocator
{
public:
	DefaultAllocator();
	~DefaultAllocator();

	/// <summary>
	/// allocate memory of requested size
	/// </summary>
	/// <param name="size"></param>
	/// <returns></returns>
	void* Allocate(const size_type& size);

	/// <summary>
	/// free memory
	/// </summary>
	/// <param name="ptr"></param>
	void Free(void* ptr);
};

#include "detail/DefaultAllocator.inl"

#pragma once

typedef unsigned long long size_type;
typedef unsigned char byte;
#define MAX_SIZE ((size_type)0xffffffffffffffff)

#define BYTE * (size_type)1
#define KB BYTE * (size_type)1024
#define MB KB * (size_type)1024
#define GB MB * (size_type)1024

constexpr size_type ALIGNMENT = 8 BYTE;
constexpr size_type PAGE_SIZE = 4 KB;

class Allocator
{
public:
	enum class AllocationPolicy
	{
		kStatic,
		kDynamic
	};

	Allocator(const size_type& capacity, const size_type& alignment, const size_type& page_size, const AllocationPolicy& allocation_policy);

	virtual ~Allocator();

	/// <summary>
	/// allocate memory of requested size
	/// </summary>
	/// <param name="size"></param>
	/// <returns></returns>
	virtual void* Allocate(const size_type& size) = 0;

	/// <summary>
	/// free memory
	/// </summary>
	/// <param name="ptr"></param>
	virtual void Free(void* ptr) = 0;

	size_type RoundUp(const size_type& alignment, const size_type& size);

	/// <summary>
	/// size of freed memory
	/// </summary>
	/// <returns></returns>
	size_type FreedSize();

	/// <summary>
	/// size of allocated memory
	/// </summary>
	/// <returns></returns>
	size_type AllocatedSize();

	/// <summary>
	/// allocator capacity
	/// </summary>
	/// <returns></returns>
	size_type Capacity();

	/// <summary>
	/// peak size of allocation
	/// </summary>
	/// <returns></returns>
	size_type Peak();

protected:
	/// <summary>
	/// allocation policy, static or dynamic
	/// </summary>
	AllocationPolicy allocation_policy_;

	/// <summary>
	/// the capacity of allocator
	/// </summary>
	size_type capacity_;

	/// <summary>
	/// allocated memory size
	/// </summary>
	size_type allocated_;

	/// <summary>
	/// freed memory size
	/// </summary>
	size_type freed_;

	/// <summary>
	/// peak allocation
	/// </summary>
	size_type peak_;

	/// <summary>
	/// alignment
	/// </summary>
	size_type alignment_;

	/// <summary>
	/// page size
	/// </summary>
	size_type page_size_;
};
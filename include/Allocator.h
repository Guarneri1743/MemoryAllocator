#pragma once

typedef unsigned long long size_type;
typedef unsigned char byte;
#define MAX_SIZE ((size_type)0xffffffffffffffff)

#define BYTE * (size_type)1
#define KB BYTE * (size_type)1024
#define MB KB * (size_type)1024
#define GB MB * (size_type)1024

constexpr size_type MIN_ALIGNMENT = 8 BYTE;
constexpr size_type ALIGNMENT = 8 BYTE;
constexpr size_type PAGE_SIZE = 4 KB;
constexpr size_type LARGE_BLOCK_THRESHOLD = 128 BYTE;

class Allocator
{
public:
	enum class AllocationPolicy
	{
		Static,
		Dynamic
	};

	Allocator(const size_type& capacity, const AllocationPolicy& allocation_policy)
	{
		start_ = nullptr;
		capacity_ = capacity;
		allocated_ = 0;
		freed_ = capacity;
		allocation_policy_ = allocation_policy;
		peak_ = 0;
	};

	virtual ~Allocator()
	{
	};

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

	/// <summary>
	/// start address of memory block
	/// </summary>
	/// <returns></returns>
	void* GetNativePtr()
	{
		return start_;
	}

	/// <summary>
	/// size of freed memory
	/// </summary>
	/// <returns></returns>
	size_type FreedSize()
	{
		return freed_;
	}

	/// <summary>
	/// size of allocated memory
	/// </summary>
	/// <returns></returns>
	size_type AllocatedSize()
	{
		return allocated_;
	}

	/// <summary>
	/// allocator capacity
	/// </summary>
	/// <returns></returns>
	size_type Capacity()
	{
		return capacity_;
	}

	/// <summary>
	/// peak size of allocation
	/// </summary>
	/// <returns></returns>
	size_type Peak()
	{
		return peak_;
	}

protected:
	/// <summary>
	/// allocation policy, static or dynamic
	/// </summary>
	AllocationPolicy allocation_policy_;

	/// <summary>
	/// start address
	/// </summary>
	void* start_;

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
};
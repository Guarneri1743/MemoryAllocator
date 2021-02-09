#pragma once
#include "Allocator.h"

template<size_type kPageSize = PAGE_SIZE, size_type kAlignment = ALIGNMENT>
class ExplicitFreeListAllocator : public Allocator
{
public:
	enum class PlacementPolicy
	{
		kFirstFit,
		kNextFit,
		kBestFit
	};

	enum class CoalescingPolicy
	{
		kImmediate,
		kDeferred
	};

	struct Block
	{
		size_type size;
		Block* prev;
		Block* next;
	};

	typedef Block* BlockPointer;

public:
	ExplicitFreeListAllocator(const size_type& capacity);

	ExplicitFreeListAllocator(const size_type& capacity,
					 const PlacementPolicy& placement_policy);

	ExplicitFreeListAllocator(const size_type& capacity,
					 const PlacementPolicy& placement_policy,
					 CoalescingPolicy& coalescing_policy);

	/// <summary>
	/// explicit free list constructor
	/// </summary>
	/// <param name="capacity">the capacity of memory</param>
	/// <param name="placement_policy">placement policy</param>
	/// <param name="coalescing_policy">coalescing policy</param>
	ExplicitFreeListAllocator(const size_type& capacity,
					 const PlacementPolicy& placement_policy,
					 const CoalescingPolicy& coalescing_policy,
					 const AllocationPolicy& allocation_policy);

	/// <summary>
	/// explicit free list destructor
	/// </summary>
	~ExplicitFreeListAllocator();

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

private:

	/// <summary>
	/// placement policy
	/// </summary>
	PlacementPolicy placement_policy_;

	/// <summary>
	/// coalescing policy
	/// </summary>
	CoalescingPolicy coalescing_policy_;

	/// <summary>
	/// free blocks list
	/// </summary>
	BlockPointer free_list_;

	/// <summary>
	/// the last fit block, required by 'NextFit' policy
	/// </summary>
	BlockPointer last_fit_;

	/// <summary>
	/// Allocate a block
	/// </summary>
	/// <param name="size"></param>
	void AllocateBlock(const size_type& size);

	/// <summary>
	/// find a fit block
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void Find(const size_type& aligned_size, BlockPointer& found);

	/// <summary>
	/// first fit
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void FindFirstFit(const size_type& aligned_size, BlockPointer& found);

	/// <summary>
	/// next fit
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void FindNextFit(const size_type& aligned_size, BlockPointer& found);

	/// <summary>
	/// best fit
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void FindBestFit(const size_type& aligned_size, BlockPointer& found);

	/// <summary>
	/// insert a new node after given node
	/// </summary>
	/// <param name="node"></param>
	/// <param name="to_insert"></param>
	void Insert(BlockPointer& node, BlockPointer& to_insert);

	/// <summary>
	/// remove given node from linked list
	/// </summary>
	/// <param name="node"></param>
	void Remove(BlockPointer& node);

	/// <summary>
	/// determine wheather the alignment is the power of 2 or not
	/// </summary>
	/// <param name="alignment"></param>
	/// <returns></returns>
	bool IsAligned(const size_type& alignment);

	/// <summary>
	/// align memory
	/// </summary>
	/// <param name="size"></param>
	/// <param name="alignment"></param>
	/// <returns></returns>
	void Align(const size_type& size, const size_type& alignment, size_type& aligned_size, size_type& padding);

	/// <summary>
	/// delete copy constructor
	/// </summary>
	/// <param name="_allocator"></param>
	ExplicitFreeListAllocator(const ExplicitFreeListAllocator& _allocator) = delete;

	/// <summary>
	/// delete move constructor
	/// </summary>
	/// <param name="_allocator"></param>
	ExplicitFreeListAllocator(ExplicitFreeListAllocator&& _allocator) = delete;
};

#include "detail/ExplicitFreeListAllocator.inl"
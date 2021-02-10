#pragma once
#include "Allocator.h"

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

	struct Span
	{
		size_type size;
		Span* prev;
		Span* next;
	};

	struct Block
	{
		size_type size;
		Block* prev;
		Block* next;
	};

	typedef Span* SpanPointer;
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
							  const size_type& alignment,
							  const size_type& page_size,
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
	/// the head of memory block
	/// </summary>
	BlockPointer block_list_head_;

	/// <summary>
	/// the tail of memory block
	/// </summary>
	BlockPointer block_list_tail_;

	/// <summary>
	/// free span list
	/// </summary>
	SpanPointer free_list_;

	/// <summary>
	/// last fit span
	/// </summary>
	SpanPointer last_fit_;

	/// <summary>
	/// Allocate a block
	/// </summary>
	/// <param name="size"></param>
	void AllocateBlock(const size_type& size, SpanPointer& out_block);

	/// <summary>
	/// find a fit Span
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void Find(const size_type& aligned_size, SpanPointer& found);

	/// <summary>
	/// first fit
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void FindFirstFit(const size_type& aligned_size, SpanPointer& found);

	/// <summary>
	/// next fit
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void FindNextFit(const size_type& aligned_size, SpanPointer& found);

	/// <summary>
	/// best fit
	/// </summary>
	/// <param name="found"></param>
	/// <param name="aligned_size"></param>
	/// <param name="padding"></param>
	void FindBestFit(const size_type& aligned_size, SpanPointer& found);

	/// <summary>
	/// insert a new node after given node
	/// </summary>
	/// <param name="node"></param>
	/// <param name="to_insert"></param>
	void Insert(SpanPointer& node, SpanPointer& to_insert);

	/// <summary>
	/// remove given node from linked list
	/// </summary>
	/// <param name="node"></param>
	void Remove(SpanPointer& span);

	/// <summary>
	/// Merge
	/// </summary>
	/// <param name="node"></param>
	void Merge(const size_type& address, SpanPointer& span);

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
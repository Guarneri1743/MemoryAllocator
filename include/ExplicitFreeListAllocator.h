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

	struct BoundaryTag
	{
		size_type size_and_flag;
	};

	struct Span
	{
		BoundaryTag tag;
		Span* prev;
		Span* next;
	};

	typedef BoundaryTag* BoundaryTagPointer;
	typedef Span* SpanPointer;

public:
	ExplicitFreeListAllocator(const size_type& capacity);

	ExplicitFreeListAllocator(const size_type& capacity,
							  const PlacementPolicy& placement_policy);

	ExplicitFreeListAllocator(const size_type& capacity,
							  const PlacementPolicy& placement_policy,
							  const CoalescingPolicy& coalescing_policy);

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

	/// <summary>
	/// Determine whether a pointer is belong to this allocator
	/// </summary>
	/// <param name="address"></param>
	/// <returns></returns>
	bool Contains(const size_type& address);

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
	/// free span list
	/// </summary>
	SpanPointer free_list_;

	/// <summary>
	/// last fit span
	/// </summary>
	SpanPointer last_fit_;

	/// <summary>
	/// pointer to heap 
	/// </summary>
	void* heap;

	/// <summary>
	/// start address
	/// </summary>
	size_type heap_start_address;

	/// <summary>
	/// end address
	/// </summary>
	size_type heap_end;

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
	/// insert 
	/// </summary>
	/// <param name="address"></param>
	/// <param name="span"></param>
	void InsertToFreeList(const size_type& address, SpanPointer& span);

	/// <summary>
	/// remove given node from linked list
	/// </summary>
	/// <param name="node"></param>
	void RemoveFromFreeList(SpanPointer& span);

	/// <summary>
	/// coalescing
	/// </summary>
	/// <param name="node"></param>
	void Coalesce(SpanPointer& span, SpanPointer& merged_span, size_type& merged_span_address);

	/// <summary>
	/// split
	/// </summary>
	/// <param name="span"></param>
	/// <param name="left_size"></param>
	/// <param name="right_size"></param>
	/// <param name="left"></param>
	/// <param name="right"></param>
	/// <param name="left_addr"></param>
	/// <param name="right_addr"></param>
	void Split(SpanPointer& span, 
			   const size_type& left_size, 
			   const size_type& right_size, 
			   SpanPointer& left, 
			   SpanPointer& right, 
			   size_type& left_addr, 
			   size_type& right_addr);

	SpanPointer CreateSpan(const size_type& address, const size_type& size);

	/// <summary>
	/// Mask out flag
	/// </summary>
	/// <param name="tag"></param>
	/// <returns></returns>
	bool IsFree(const BoundaryTag& tag);

	/// <summary>
	/// Mask out size
	/// </summary>
	/// <param name="tag"></param>
	/// <returns></returns>
	size_type GetSize(const BoundaryTag& tag);

	/// <summary>
	/// set size
	/// </summary>
	/// <param name="tag"></param>
	/// <param name="size"></param>
	void SetSize(BoundaryTag& tag, const size_type& size);

	/// <summary>
	/// set flag
	/// </summary>
	/// <param name="tag"></param>
	/// <param name="allocated"></param>
	void SetFlag(BoundaryTag& tag, bool allocated);

	/// <summary>
	/// set size and flag
	/// </summary>
	/// <param name="tag"></param>
	/// <param name="size"></param>
	/// <param name="allocated"></param>
	void SetSizeAndFlag(BoundaryTag& tag, const size_type& size, bool allocated);

	/// <summary>
	/// set flag and footer
	/// </summary>
	/// <param name="span"></param>
	/// <param name="allocated"></param>
	void SetFlag(SpanPointer& span, bool allocated);

	/// <summary>
	/// set size, flag and footer
	/// </summary>
	/// <param name="span"></param>
	/// <param name="size"></param>
	/// <param name="allocated"></param>
	void SetSizeAndFlag(SpanPointer& span, const size_type& size, bool allocated);

	/// <summary>
	/// set flag and footer
	/// </summary>
	/// <param name="address"></param>
	/// <param name="span"></param>
	/// <param name="allocated"></param>
	void SetFlag(const size_type& address, SpanPointer& span, bool allocated);

	/// <summary>
	/// set size, flag and footer
	/// </summary>
	/// <param name="address"></param>
	/// <param name="span"></param>
	/// <param name="size"></param>
	/// <param name="allocated"></param>
	void SetSizeAndFlag(const size_type& address, SpanPointer& span, const size_type& size, bool allocated);

	/// <summary>
	/// sync footer
	/// </summary>
	/// <param name="address"></param>
	/// <param name="size"></param>
	/// <param name="tag"></param>
	void SyncFooter(const size_type& address, const size_type& size, const BoundaryTag& tag);

	/// <summary>
	/// find left span
	/// </summary>
	/// <param name="current_address"></param>
	/// <param name="left"></param>
	/// <param name="left_address"></param>
	/// <param name="left_size"></param>
	void FindLeftSpan(const size_type& current_address, SpanPointer& left, size_type& left_address, size_type& left_size);

	/// <summary>
	/// find right span
	/// </summary>
	/// <param name="current_address"></param>
	/// <param name="cur_size"></param>
	/// <param name="right"></param>
	/// <param name="right_address"></param>
	/// <param name="right_size"></param>
	void FindRightSpan(const size_type& current_address, const size_type& cur_size, SpanPointer& right, size_type& right_address, size_type& right_size);

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

constexpr size_type MIN_SPAN_SIZE = sizeof(ExplicitFreeListAllocator::Span) + sizeof(ExplicitFreeListAllocator::BoundaryTag) + ALIGNMENT;
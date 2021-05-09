#pragma once
#include "Define.h"

class ExplicitFreeListAllocator 
{
public:
	struct BoundaryTag
	{
		mem_size_t size_and_flag;
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
	ExplicitFreeListAllocator(const mem_size_t& capacity);
	ExplicitFreeListAllocator(const mem_size_t& capacity, const PlacementPolicy& placement_policy);
	ExplicitFreeListAllocator(const mem_size_t& capacity, const PlacementPolicy& placement_policy, const CoalescingPolicy& coalescing_policy);
	~ExplicitFreeListAllocator();

	void* Allocate(const mem_size_t& size);
	void Free(void* ptr);

	bool Contains(const mem_size_t& address);

private:
	PlacementPolicy placement_policy_;
	CoalescingPolicy coalescing_policy_;
	SpanPointer free_list_;
	SpanPointer last_fit_;
	void* heap_;
	mem_size_t heap_start_address_;
	mem_size_t heap_end_;

	void Find(const mem_size_t& aligned_size, SpanPointer& found);
	void FindFirstFit(const mem_size_t& aligned_size, SpanPointer& found);
	void FindNextFit(const mem_size_t& aligned_size, SpanPointer& found);
	void FindBestFit(const mem_size_t& aligned_size, SpanPointer& found);
	void InsertToFreeList(const mem_size_t& address, SpanPointer& span);
	void RemoveFromFreeList(SpanPointer& span);
	void Coalesce(SpanPointer& span, SpanPointer& merged_span, mem_size_t& merged_span_address);
	void Split(SpanPointer& span, 
			   const mem_size_t& left_size, 
			   const mem_size_t& right_size, 
			   SpanPointer& left, 
			   SpanPointer& right, 
			   mem_size_t& left_addr, 
			   mem_size_t& right_addr);

	SpanPointer CreateSpan(const mem_size_t& address, const mem_size_t& size);
	bool IsFree(const BoundaryTag& tag);
	mem_size_t GetSize(const BoundaryTag& tag);
	void SetSize(BoundaryTag& tag, const mem_size_t& size);
	void SetFlag(BoundaryTag& tag, bool allocated);
	void SetSizeAndFlag(BoundaryTag& tag, const mem_size_t& size, bool allocated);
	void SetFlag(SpanPointer& span, bool allocated);
	void SetSizeAndFlag(SpanPointer& span, const mem_size_t& size, bool allocated);
	void SetFlag(const mem_size_t& address, SpanPointer& span, bool allocated);
	void SetSizeAndFlag(const mem_size_t& address, SpanPointer& span, const mem_size_t& size, bool allocated);
	void SyncFooter(const mem_size_t& address, const mem_size_t& size, const BoundaryTag& tag);
	void FindLeftSpan(const mem_size_t& current_address, SpanPointer& left, mem_size_t& left_address, mem_size_t& left_size);
	void FindRightSpan(const mem_size_t& current_address, const mem_size_t& cur_size, SpanPointer& right, mem_size_t& right_address, mem_size_t& right_size);
	void Align(const mem_size_t& size, const mem_size_t& alignment, mem_size_t& aligned_size, mem_size_t& padding);

	ExplicitFreeListAllocator(const ExplicitFreeListAllocator& _allocator) = delete;
	ExplicitFreeListAllocator(ExplicitFreeListAllocator&& _allocator) = delete;
};
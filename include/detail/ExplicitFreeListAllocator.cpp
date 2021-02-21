#include "ExplicitFreeListAllocator.h"
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <string>
#include <iostream>

ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity) :
	ExplicitFreeListAllocator(capacity,
							  ALIGNMENT,
							  PAGE_SIZE,
							  PlacementPolicy::kFirstFit,
							  CoalescingPolicy::kImmediate,
							  AllocationPolicy::kDynamic)
{}


ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity,
													 const PlacementPolicy& placement_policy) :
	ExplicitFreeListAllocator(capacity,
							  ALIGNMENT,
							  PAGE_SIZE,
							  placement_policy,
							  CoalescingPolicy::kImmediate,
							  AllocationPolicy::kDynamic)
{}


ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity,
													 const PlacementPolicy& placement_policy,
													 const CoalescingPolicy& coalescing_policy) :
	ExplicitFreeListAllocator(capacity,
							  ALIGNMENT,
							  PAGE_SIZE,
							  placement_policy,
							  coalescing_policy,
							  AllocationPolicy::kDynamic)
{}


ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity,
													 const size_type& alignment,
													 const size_type& page_size,
													 const PlacementPolicy& placement_policy,
													 const CoalescingPolicy& coalescing_policy,
													 const AllocationPolicy& allocation_policy) : Allocator(capacity, alignment, page_size, allocation_policy)
{
	heap = malloc(capacity);
	heap_start_address = reinterpret_cast<size_type>(heap);
	heap_end = heap_start_address + capacity;
	free_list_ = CreateSpan(heap_start_address, capacity - (sizeof(BoundaryTag) << 1));
	placement_policy_ = placement_policy;
	coalescing_policy_ = coalescing_policy;
	last_fit_ = free_list_;
}

ExplicitFreeListAllocator::~ExplicitFreeListAllocator()
{
	free(heap);
	heap = nullptr;
	free_list_ = nullptr;
	last_fit_ = nullptr;
}

void* ExplicitFreeListAllocator::Allocate(const size_type& size)
{
	assert(size > 0);

	SpanPointer fit_span = nullptr;

	size_type aligned_size, padding;

	Align(size, alignment_, aligned_size, padding);

	Find(aligned_size, fit_span);

	assert(fit_span != nullptr);

	// remove fit_span
	RemoveFromFreeList(fit_span);

	// split the fit span if there is some extra space
	size_type extra_space = GetSize(fit_span->tag) - aligned_size;
	if (extra_space > MIN_SPAN_SIZE)
	{
		SpanPointer left, right;
		size_type left_addr, right_addr;
		Split(fit_span, aligned_size, extra_space - (sizeof(BoundaryTag) << 1), left, right, left_addr, right_addr);
		fit_span = left;
		InsertToFreeList(right_addr, right);
	}

	size_type span_address = reinterpret_cast<size_type>(fit_span);
	SetFlag(span_address, fit_span, true);

	size_type payload_start = span_address + sizeof(BoundaryTag);

	allocated_ += aligned_size;
	freed_ -= aligned_size;
	peak_ = std::max(peak_, allocated_);

	/*size_type btag = payload_start + aligned_size;
	auto tag = reinterpret_cast<BoundaryTagPointer>(btag);
	std::cout << "Allocate: " << span_address << ", " << payload_start << ", " << btag << ", " << btag + sizeof(BoundaryTag) - 1 << " | " << GetSize(fit_span->tag) << ", " << IsFree(fit_span->tag) << " | " << GetSize(*tag) << ", " << IsFree(*tag) << " | " << std::endl;*/

	return reinterpret_cast<void*>(payload_start);
}

void ExplicitFreeListAllocator::Free(void* ptr)
{
	assert(ptr != nullptr);
	assert(Contains(reinterpret_cast<size_type>(ptr)));

	size_type address = reinterpret_cast<size_type>(ptr);
	size_type span_address = address - sizeof(BoundaryTag);

	// put the span into the address-ordered position of free list
	SpanPointer span = reinterpret_cast<SpanPointer>(span_address);

	/*size_type btag = address + GetSize(span->tag);
	auto tag = reinterpret_cast<BoundaryTagPointer>(btag);
	std::cout << "Free: " << span_address << ", " << address << ", " << btag << ", " << btag + sizeof(BoundaryTag) - 1 << " | "  << GetSize(span->tag) << ", " << IsFree(span->tag) << " | " << GetSize(*tag) << ", " << IsFree(*tag) << " | " << std::endl;*/

	size_type size = GetSize(span->tag);
	allocated_ -= size;
	freed_ += size;

	SpanPointer merged_span = nullptr;
	size_type merged_span_address = 0;
	Coalesce(span, merged_span, merged_span_address);
	InsertToFreeList(merged_span_address, merged_span);

	//std::cout << "Dump Free List: " <<std::endl;
	//SpanPointer cur = free_list_;
	//while (cur != nullptr)
	//{
	//	std::cout << "Span: " << reinterpret_cast<size_type>(cur) << ", " << GetSize(cur->tag) << "|" << IsFree(cur->tag) << " | " << reinterpret_cast<size_type>(cur) + GetSize(cur->tag) + (sizeof(BoundaryTag) << 1)<< std::endl;
	//	cur = cur->next;
	//}
}

void ExplicitFreeListAllocator::Find(const size_type& aligned_size, SpanPointer& found)
{
	if (placement_policy_ == PlacementPolicy::kFirstFit)
	{
		FindFirstFit(aligned_size, found);
	}
	else if (placement_policy_ == PlacementPolicy::kNextFit)
	{
		FindNextFit(aligned_size, found);
	}
	else if (placement_policy_ == PlacementPolicy::kBestFit)
	{
		FindBestFit(aligned_size, found);
	}
	else
	{
		FindFirstFit(aligned_size, found);
	}
}

void ExplicitFreeListAllocator::FindFirstFit(const size_type& aligned_size, SpanPointer& found)
{
	SpanPointer cur = free_list_;

	while (cur != nullptr)
	{
		if (GetSize(cur->tag) >= aligned_size)
		{
			found = cur;
			break;
		}
		cur = cur->next;
	}
}

void ExplicitFreeListAllocator::FindNextFit(const size_type& aligned_size, SpanPointer& found)
{
	SpanPointer cur = last_fit_;

	while (cur != nullptr)
	{
		if (GetSize(cur->tag) >= aligned_size)
		{
			found = cur;
			last_fit_ = cur;
			break;
		}
		cur = cur->next;
	}
}

void ExplicitFreeListAllocator::FindBestFit(const size_type& aligned_size, SpanPointer& found)
{
	size_type min_span = MAX_SIZE;
	SpanPointer min_span_pointer = nullptr;
	SpanPointer cur = free_list_;

	while (cur != nullptr)
	{
		size_type size = GetSize(cur->tag);
		if (size >= aligned_size)
		{
			if (size < min_span)
			{
				min_span = size;
				min_span_pointer = cur;
			}
		}
		cur = cur->next;
	}

	found = min_span_pointer;
}

void ExplicitFreeListAllocator::InsertToFreeList(const size_type& address, SpanPointer& span)
{
	assert(span != nullptr);
	assert(span->prev == nullptr);

	SetFlag(address, span, false);

	if (span == free_list_)
	{
		return;
	}

	span->prev = nullptr;
	span->next = free_list_;

	if (free_list_ != nullptr)
	{
		free_list_->prev = span;
	}

	free_list_ = span;
}


void ExplicitFreeListAllocator::RemoveFromFreeList(SpanPointer& span)
{
	if (span != nullptr)
	{
		SpanPointer prev = span->prev;
		SpanPointer next = span->next;

		span->prev = nullptr;
		span->next = nullptr;

		if (prev != nullptr)
		{
			prev->next = next;
		}

		if (next != nullptr)
		{
			next->prev = prev;
		}

		if (span == free_list_)
		{
			if (prev != nullptr)
			{
				free_list_ = prev;
			}
			else if (next != nullptr)
			{
				free_list_ = next;
			}
			else
			{
				free_list_ = nullptr;
			}
		}
	}
}

void ExplicitFreeListAllocator::Coalesce(SpanPointer& span, SpanPointer& merged_span, size_type& merged_span_address)
{
	size_type cur_size = GetSize(span->tag);
	size_type cur_address = reinterpret_cast<size_type>(span);
	
	SpanPointer left, right;
	size_type left_size, right_size;
	size_type left_address, right_address;
	FindLeftSpan(cur_address, left, left_address, left_size);
	FindRightSpan(cur_address, cur_size, right, right_address, right_size);

	bool has_left_span = left != nullptr;
	bool has_right_span = right != nullptr;

	size_type merged_size = cur_size;
	merged_span = span;
	merged_span_address = cur_address;

	if (has_left_span && 
		has_right_span && 
		IsFree(left->tag) && 
		IsFree(right->tag))
	{
		RemoveFromFreeList(right);
		merged_span = left;
		merged_size = left_size + cur_size + GetSize(right->tag) + (sizeof(BoundaryTag) << 2);
		merged_span_address = left_address;
		//std::cout << "Merge LCR: " << left_address << ", " << cur_address << ", " << right_address << ", " << right_address + GetSize(right->tag) + (sizeof(BoundaryTag) << 1) << " | merged: " << merged_span_address << ", size: " << merged_size << ", end: " << merged_span_address + merged_size + (sizeof(BoundaryTag) << 1) <<  std::endl;
	}
	else if (has_left_span && IsFree(left->tag))
	{
		merged_span = left;
		merged_size = left_size + cur_size + (sizeof(BoundaryTag) << 1);
		merged_span_address = left_address;
		//std::cout << "Merge LC: " << left_address << ", " << cur_address << ", " << cur_address + cur_size + (sizeof(BoundaryTag) << 1) << " | merged: " << merged_span_address << ", size: " << merged_size << ", end: " << merged_span_address + merged_size + (sizeof(BoundaryTag) << 1) << std::endl;
	}
	else if (has_right_span && IsFree(right->tag))
	{
		RemoveFromFreeList(right);
		merged_span = span;
		merged_size = cur_size + GetSize(right->tag) + (sizeof(BoundaryTag) << 1);
		merged_span_address = cur_address;
		//std::cout << "Merge CR: " << cur_address << ", " << right_address << ", " << right_address + GetSize(right->tag) + (sizeof(BoundaryTag) << 1) << " | merged: " << merged_span_address << ", size: " << merged_size << ", end: " << merged_span_address + merged_size + (sizeof(BoundaryTag) << 1) << std::endl;
	}

	if (merged_span != nullptr)
	{
		SetSizeAndFlag(merged_span_address, merged_span, merged_size, false);
	}
}

void ExplicitFreeListAllocator::Split(SpanPointer& span, 
									  const size_type& left_size, 
									  const size_type& right_size, 
									  SpanPointer& left, 
									  SpanPointer& right, 
									  size_type& left_addr, 
									  size_type& right_addr)
{
	size_type span_address = reinterpret_cast<size_type>(span);
	left_addr = span_address;
	right_addr = span_address + left_size + (sizeof(BoundaryTag) << 1);

	left = CreateSpan(left_addr, left_size);
	right = CreateSpan(right_addr, right_size);
}

ExplicitFreeListAllocator::SpanPointer ExplicitFreeListAllocator::CreateSpan(const size_type& address, const size_type& size)
{
	SpanPointer new_span = reinterpret_cast<SpanPointer>(address);
	new_span->prev = nullptr;
	new_span->next = nullptr;
	SetSizeAndFlag(address, new_span, size, false);
	return new_span;
}

void ExplicitFreeListAllocator::FindLeftSpan(const size_type& cur_address, SpanPointer& left, size_type& left_address, size_type& left_size)
{
	size_type left_footer_address = cur_address - sizeof(BoundaryTag);

	if (!Contains(left_footer_address))
	{
		left = nullptr;
		left_size = 0;
		return;
	}

	BoundaryTagPointer left_btag = reinterpret_cast<BoundaryTagPointer>(left_footer_address);
	left_size = GetSize(*left_btag);
	left_address = left_footer_address - left_size - sizeof(BoundaryTag);
	left = reinterpret_cast<SpanPointer>(left_address);

	assert(left_size == GetSize(left->tag));
}

void ExplicitFreeListAllocator::FindRightSpan(const size_type& cur_address, const size_type& cur_size, SpanPointer& right, size_type& right_address, size_type& right_size)
{
	right_address = cur_address + cur_size + (sizeof(BoundaryTag) << 1);

	if (!Contains(right_address))
	{
		right = nullptr;
		right_size = 0;
		return;
	}

	right = reinterpret_cast<SpanPointer>(right_address);
	right_size = GetSize(right->tag);
}

void ExplicitFreeListAllocator::SetFlag(SpanPointer& span, bool allocated)
{
	assert(span != nullptr);

	size_type address = reinterpret_cast<size_type>(span);
	SetFlag(address, span, allocated);
}

void ExplicitFreeListAllocator::SetSizeAndFlag(SpanPointer& span, const size_type& size, bool allocated)
{
	assert(span != nullptr);

	size_type address = reinterpret_cast<size_type>(span);
	SetSizeAndFlag(address, span, size, allocated);
}

void ExplicitFreeListAllocator::SetFlag(const size_type& address, SpanPointer& span, bool allocated)
{
	assert(span != nullptr);

	SetFlag(span->tag, allocated);
	SyncFooter(address, GetSize(span->tag), span->tag);
}

void ExplicitFreeListAllocator::SetSizeAndFlag(const size_type& address, SpanPointer& span, const size_type& size, bool allocated)
{
	assert(span != nullptr);

	SetSizeAndFlag(span->tag, size, allocated);
	SyncFooter(address, size, span->tag);
}

inline bool ExplicitFreeListAllocator::IsFree(const BoundaryTag& tag)
{
	return (tag.size_and_flag & FLAG_MASK) == 0;
}

inline size_type ExplicitFreeListAllocator::GetSize(const BoundaryTag& tag)
{
	return tag.size_and_flag & ~FLAG_MASK;
}

inline void ExplicitFreeListAllocator::SetSize(BoundaryTag& tag, const size_type& size)
{
	tag.size_and_flag = (size & ~FLAG_MASK) | (tag.size_and_flag & FLAG_MASK);
}

inline void ExplicitFreeListAllocator::SetFlag(BoundaryTag& tag, bool allocated)
{
	tag.size_and_flag = (allocated ? 0x1 : 0x0) | (tag.size_and_flag & ~FLAG_MASK);
}

inline void ExplicitFreeListAllocator::SetSizeAndFlag(BoundaryTag& tag, const size_type& size, bool allocated)
{
	tag.size_and_flag = (allocated ? 0x1 : 0x0) | (size & ~FLAG_MASK);
}

inline void ExplicitFreeListAllocator::SyncFooter(const size_type& address, const size_type& size, const BoundaryTag& tag)
{
	size_type footer_address = address + sizeof(BoundaryTag) + size;
	BoundaryTagPointer footer = reinterpret_cast<BoundaryTagPointer>(footer_address);
	footer->size_and_flag = tag.size_and_flag;
}

inline bool ExplicitFreeListAllocator::Contains(const size_type& address)
{
	return address >= heap_start_address && address <= heap_end;
}

inline void ExplicitFreeListAllocator::Align(const size_type& size, const size_type& alignment, size_type& aligned_size, size_type& padding)
{
	aligned_size = (size + alignment - 1) & ~(alignment - 1);
	padding = aligned_size - size;
}
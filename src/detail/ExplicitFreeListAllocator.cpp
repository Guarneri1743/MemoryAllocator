#include "ExplicitFreeListAllocator.h"
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

constexpr mem_size_t MIN_SPAN_SIZE = sizeof(ExplicitFreeListAllocator::Span) + sizeof(ExplicitFreeListAllocator::BoundaryTag) + ALIGNMENT;

ExplicitFreeListAllocator::ExplicitFreeListAllocator(const mem_size_t& capacity) :
	ExplicitFreeListAllocator(capacity,
							  PlacementPolicy::kFirstFit,
							  CoalescingPolicy::kImmediate)
{}


ExplicitFreeListAllocator::ExplicitFreeListAllocator(const mem_size_t& capacity,
													 const PlacementPolicy& placement_policy) :
	ExplicitFreeListAllocator(capacity,
							  placement_policy,
							  CoalescingPolicy::kImmediate)
{}

ExplicitFreeListAllocator::ExplicitFreeListAllocator(const mem_size_t& capacity,
													 const PlacementPolicy& placement_policy,
													 const CoalescingPolicy& coalescing_policy)
{
	heap_ = malloc(capacity);
	heap_start_address_ = reinterpret_cast<mem_size_t>(heap_);
	heap_end_ = heap_start_address_ + capacity;
	free_list_ = CreateSpan(heap_start_address_, capacity - (sizeof(BoundaryTag) << 1));
	placement_policy_ = placement_policy;
	coalescing_policy_ = coalescing_policy;
	last_fit_ = free_list_;
}

ExplicitFreeListAllocator::~ExplicitFreeListAllocator()
{
	free(heap_);
	heap_ = nullptr;
	free_list_ = nullptr;
	last_fit_ = nullptr;
}

void* ExplicitFreeListAllocator::Allocate(const mem_size_t& size)
{
	assert(size > 0);

	SpanPointer fit_span = nullptr;

	mem_size_t aligned_size, padding;

	Align(size, ALIGNMENT, aligned_size, padding);

	Find(aligned_size, fit_span);

	assert(fit_span != nullptr);

	// remove fit_span
	RemoveFromFreeList(fit_span);

	// split the fit span if there is some extra space
	mem_size_t extra_space = GetSize(fit_span->tag) - aligned_size;
	if (extra_space > MIN_SPAN_SIZE)
	{
		SpanPointer left, right;
		mem_size_t left_addr, right_addr;
		Split(fit_span, aligned_size, extra_space - (sizeof(BoundaryTag) << 1), left, right, left_addr, right_addr);
		fit_span = left;
		InsertToFreeList(right_addr, right);
	}

	mem_size_t span_address = reinterpret_cast<mem_size_t>(fit_span);
	SetFlag(span_address, fit_span, true);

	mem_size_t payload_start = span_address + sizeof(BoundaryTag);

	return reinterpret_cast<void*>(payload_start);
}

void ExplicitFreeListAllocator::Free(void* ptr)
{
	assert(ptr != nullptr);
	assert(Contains(reinterpret_cast<mem_size_t>(ptr)));

	mem_size_t address = reinterpret_cast<mem_size_t>(ptr);
	mem_size_t span_address = address - sizeof(BoundaryTag);

	// put the span into the address-ordered position of free list
	SpanPointer span = reinterpret_cast<SpanPointer>(span_address);

	mem_size_t size = GetSize(span->tag);

	SpanPointer merged_span = nullptr;
	mem_size_t merged_span_address = 0;
	Coalesce(span, merged_span, merged_span_address);
	InsertToFreeList(merged_span_address, merged_span);
}

void ExplicitFreeListAllocator::Find(const mem_size_t& aligned_size, SpanPointer& found)
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

void ExplicitFreeListAllocator::FindFirstFit(const mem_size_t& aligned_size, SpanPointer& found)
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

void ExplicitFreeListAllocator::FindNextFit(const mem_size_t& aligned_size, SpanPointer& found)
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

void ExplicitFreeListAllocator::FindBestFit(const mem_size_t& aligned_size, SpanPointer& found)
{
	mem_size_t min_span = MAX_SIZE;
	SpanPointer min_span_pointer = nullptr;
	SpanPointer cur = free_list_;

	while (cur != nullptr)
	{
		mem_size_t size = GetSize(cur->tag);
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

void ExplicitFreeListAllocator::InsertToFreeList(const mem_size_t& address, SpanPointer& span)
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

void ExplicitFreeListAllocator::Coalesce(SpanPointer& span, SpanPointer& merged_span, mem_size_t& merged_span_address)
{
	mem_size_t cur_size = GetSize(span->tag);
	mem_size_t cur_address = reinterpret_cast<mem_size_t>(span);
	
	SpanPointer left, right;
	mem_size_t left_size, right_size;
	mem_size_t left_address, right_address;
	FindLeftSpan(cur_address, left, left_address, left_size);
	FindRightSpan(cur_address, cur_size, right, right_address, right_size);

	bool has_left_span = left != nullptr;
	bool has_right_span = right != nullptr;

	mem_size_t merged_size = cur_size;
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
									  const mem_size_t& left_size, 
									  const mem_size_t& right_size, 
									  SpanPointer& left, 
									  SpanPointer& right, 
									  mem_size_t& left_addr, 
									  mem_size_t& right_addr)
{
	mem_size_t span_address = reinterpret_cast<mem_size_t>(span);
	left_addr = span_address;
	right_addr = span_address + left_size + (sizeof(BoundaryTag) << 1);

	left = CreateSpan(left_addr, left_size);
	right = CreateSpan(right_addr, right_size);
}

ExplicitFreeListAllocator::SpanPointer ExplicitFreeListAllocator::CreateSpan(const mem_size_t& address, const mem_size_t& size)
{
	SpanPointer new_span = reinterpret_cast<SpanPointer>(address);
	new_span->prev = nullptr;
	new_span->next = nullptr;
	SetSizeAndFlag(address, new_span, size, false);
	return new_span;
}

void ExplicitFreeListAllocator::FindLeftSpan(const mem_size_t& cur_address, SpanPointer& left, mem_size_t& left_address, mem_size_t& left_size)
{
	mem_size_t left_footer_address = cur_address - sizeof(BoundaryTag);

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

void ExplicitFreeListAllocator::FindRightSpan(const mem_size_t& cur_address, const mem_size_t& cur_size, SpanPointer& right, mem_size_t& right_address, mem_size_t& right_size)
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

	mem_size_t address = reinterpret_cast<mem_size_t>(span);
	SetFlag(address, span, allocated);
}

void ExplicitFreeListAllocator::SetSizeAndFlag(SpanPointer& span, const mem_size_t& size, bool allocated)
{
	assert(span != nullptr);

	mem_size_t address = reinterpret_cast<mem_size_t>(span);
	SetSizeAndFlag(address, span, size, allocated);
}

void ExplicitFreeListAllocator::SetFlag(const mem_size_t& address, SpanPointer& span, bool allocated)
{
	assert(span != nullptr);

	SetFlag(span->tag, allocated);
	SyncFooter(address, GetSize(span->tag), span->tag);
}

void ExplicitFreeListAllocator::SetSizeAndFlag(const mem_size_t& address, SpanPointer& span, const mem_size_t& size, bool allocated)
{
	assert(span != nullptr);

	SetSizeAndFlag(span->tag, size, allocated);
	SyncFooter(address, size, span->tag);
}

inline bool ExplicitFreeListAllocator::IsFree(const BoundaryTag& tag)
{
	return (tag.size_and_flag & FLAG_MASK) == 0;
}

inline mem_size_t ExplicitFreeListAllocator::GetSize(const BoundaryTag& tag)
{
	return tag.size_and_flag & ~FLAG_MASK;
}

inline void ExplicitFreeListAllocator::SetSize(BoundaryTag& tag, const mem_size_t& size)
{
	tag.size_and_flag = (size & ~FLAG_MASK) | (tag.size_and_flag & FLAG_MASK);
}

inline void ExplicitFreeListAllocator::SetFlag(BoundaryTag& tag, bool allocated)
{
	tag.size_and_flag = (allocated ? 0x1 : 0x0) | (tag.size_and_flag & ~FLAG_MASK);
}

inline void ExplicitFreeListAllocator::SetSizeAndFlag(BoundaryTag& tag, const mem_size_t& size, bool allocated)
{
	tag.size_and_flag = (allocated ? 0x1 : 0x0) | (size & ~FLAG_MASK);
}

inline void ExplicitFreeListAllocator::SyncFooter(const mem_size_t& address, const mem_size_t& size, const BoundaryTag& tag)
{
	mem_size_t footer_address = address + sizeof(BoundaryTag) + size;
	BoundaryTagPointer footer = reinterpret_cast<BoundaryTagPointer>(footer_address);
	footer->size_and_flag = tag.size_and_flag;
}

inline bool ExplicitFreeListAllocator::Contains(const mem_size_t& address)
{
	return address >= heap_start_address_ && address <= heap_end_;
}

inline void ExplicitFreeListAllocator::Align(const mem_size_t& size, const mem_size_t& alignment, mem_size_t& aligned_size, mem_size_t& padding)
{
	aligned_size = RoundUp(alignment, size);
	padding = aligned_size - size;
}
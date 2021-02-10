#include "ExplicitFreeListAllocator.h"
#include <stdlib.h>
#include <assert.h>
#include <algorithm>

ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity) : ExplicitFreeListAllocator(capacity,
																												   ALIGNMENT,
																												   PAGE_SIZE,
																												   PlacementPolicy::kFirstFit,
																												   CoalescingPolicy::kImmediate,
																												   AllocationPolicy::kDynamic)
{}


ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity,
															const PlacementPolicy& placement_policy) : ExplicitFreeListAllocator(capacity,
																																 ALIGNMENT,
																																 PAGE_SIZE,
																																 placement_policy,
																																 CoalescingPolicy::kImmediate,
																																 AllocationPolicy::kDynamic)
{}


ExplicitFreeListAllocator::ExplicitFreeListAllocator(const size_type& capacity,
															const PlacementPolicy& placement_policy,
															CoalescingPolicy& coalescing_policy) : ExplicitFreeListAllocator(capacity,
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
													 const AllocationPolicy& allocation_policy) : Allocator(capacity, page_size, alignment, allocation_policy)
{
	block_list_head_ = nullptr;
	block_list_tail_ = nullptr;
	AllocateBlock(capacity, free_list_);
	placement_policy_ = placement_policy;
	coalescing_policy_ = coalescing_policy;
	last_fit_ = free_list_;
}


ExplicitFreeListAllocator::~ExplicitFreeListAllocator()
{
	BlockPointer block = block_list_head_;
	while (block != nullptr)
	{
		BlockPointer next = block->next;
		free(reinterpret_cast<void*>(block));
		block = next;
	}
	block_list_head_ = nullptr;
	block_list_tail_ = nullptr;
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

	if (allocation_policy_ == AllocationPolicy::kDynamic)
	{
		// allocate new span and search again
		if (fit_span == nullptr)
		{
			size_type span_capacity = std::max(aligned_size, page_size_);
			SpanPointer new_span;
			AllocateBlock(span_capacity, new_span);
			capacity_ += span_capacity;
			freed_ += span_capacity;
			Insert(new_span, free_list_);
			free_list_ = new_span;
			Find(aligned_size, fit_span);
		}
	}

	assert(fit_span != nullptr);

	// split the fit span if there is some extra space
	size_type extra_space = fit_span->size - aligned_size;
	if (extra_space > alignment_)
	{
		size_type new_span_address = reinterpret_cast<size_type>(fit_span) + aligned_size;
		SpanPointer new_span = reinterpret_cast<SpanPointer>(new_span_address);
		new_span->prev = nullptr;
		new_span->next = nullptr;
		new_span->size = extra_space;
		fit_span->size = aligned_size;
		Insert(fit_span, new_span);
	}

	// remove fit_span
	Remove(fit_span);

	size_type span_address = reinterpret_cast<size_type>(fit_span);

	size_type payload_start = span_address + sizeof(Span);

	allocated_ += aligned_size;
	freed_ -= aligned_size;
	peak_ = std::max(peak_, allocated_);

	return reinterpret_cast<void*>(payload_start);
}


void ExplicitFreeListAllocator::Free(void* ptr)
{
	assert(ptr != nullptr);

	size_type address = reinterpret_cast<size_type>(ptr);
	size_type span_address = address - sizeof(Span);

	// put the span into the address-ordered position of free list
	SpanPointer span = reinterpret_cast<SpanPointer>(span_address);

	allocated_ -= span->size;
	freed_ += span->size;

	if (free_list_ == nullptr)
	{
		free_list_ = span;
	}
	else
	{
		SpanPointer cur = free_list_;
		SpanPointer prev = nullptr;
		while (cur != nullptr)
		{
			size_type free_list_node_address = reinterpret_cast<size_type>(cur);
			if (free_list_node_address > span_address)
			{
				if (prev == nullptr)
				{
					Insert(span, cur);
					if (cur == free_list_)
					{
						free_list_ = span;
					}
				}
				else
				{
					Insert(prev, span);
				}

				Merge(span_address, span);

				break;
			}
			prev = cur;
			cur = cur->next;
		}
	}
}


void ExplicitFreeListAllocator::AllocateBlock(const size_type& size, SpanPointer& out_span)
{
	void* buffer = malloc(size);

	assert(buffer != nullptr);

	size_type buffer_address = reinterpret_cast<size_type>(buffer);
	size_type span_address = buffer_address + sizeof(Block);
	BlockPointer block = reinterpret_cast<BlockPointer>(buffer);

	block->size = size;
	block->prev = nullptr;
	block->next = nullptr;

	if (block_list_head_ != nullptr)
	{
		if (block_list_tail_ != nullptr)
		{
			block_list_tail_->next = block;
			block->prev = block_list_tail_;
			block_list_tail_ = block;
		}
		else
		{
			block_list_head_->next = block;
			block->prev = block_list_head_;
			block_list_tail_ = block;
		}
	}
	else
	{
		block_list_head_ = block;
	}

	out_span = reinterpret_cast<SpanPointer>(span_address);

	if (out_span != nullptr)
	{
		out_span->size = size;
		out_span->prev = nullptr;
		out_span->next = nullptr;
	}
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
		if (cur->size >= aligned_size)
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
		if (cur->size >= aligned_size)
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
		if (cur->size >= aligned_size)
		{
			if (cur->size < min_span)
			{
				min_span = cur->size;
				min_span_pointer = cur;
			}
		}
		cur = cur->next;
	}

	found = min_span_pointer;
}


void ExplicitFreeListAllocator::Align(const size_type& size, const size_type& alignment, size_type& aligned_size, size_type& padding)
{
	size_type n = size / alignment;
	padding = size - alignment * n;

	size_type metadata_size = sizeof(Span);

	if (padding < metadata_size)
	{
		size_type required_space = metadata_size - padding;
		size_type meta_n = required_space / alignment;
		size_type meta_padding = required_space - alignment * meta_n;
		padding += required_space + meta_padding;
	}

	aligned_size = size + padding;
}


void ExplicitFreeListAllocator::Insert(SpanPointer& span, SpanPointer& to_insert)
{
	if (to_insert != nullptr && span != nullptr)
	{
		SpanPointer next = span->next;
		if (next != nullptr)
		{
			next->prev = to_insert;
			to_insert->next = next;
		}
		span->next = to_insert;
		to_insert->prev = span;
	}
}


void ExplicitFreeListAllocator::Remove(SpanPointer& span)
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


void ExplicitFreeListAllocator::Merge(const size_type& address, SpanPointer& span)
{
	if (span->next != nullptr)
	{
		size_type next_address = reinterpret_cast<size_type>(span->next);
		if (address + span->size == next_address)
		{
			span->size += span->next->size;
			Remove(span->next);
		}
	}

	if (span->prev != nullptr)
	{
		size_type prev_address = reinterpret_cast<size_type>(span->prev);
		if (prev_address + span->prev->size == address)
		{
			span->prev->size += span->size;
			Remove(span);
		}
	}
}
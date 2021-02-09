// header "ExplicityFreeList.h"
#include <assert.h>

template<size_type kPageSize, size_type kAlignment>
inline ExplicitFreeListAllocator<kPageSize, kAlignment>::ExplicitFreeListAllocator(const size_type& capacity) : ExplicitFreeListAllocator(capacity,
																											  PlacementPolicy::kFirstFit,
																											  CoalescingPolicy::kImmediate,
																											  AllocationPolicy::Static)
{}

template<size_type kPageSize, size_type kAlignment>
inline ExplicitFreeListAllocator<kPageSize, kAlignment>::ExplicitFreeListAllocator(const size_type& capacity, 
																 const PlacementPolicy& placement_policy) : ExplicitFreeListAllocator(capacity,
																															 placement_policy,
																															 CoalescingPolicy::kImmediate,
																															 AllocationPolicy::Static)
{}

template<size_type kPageSize, size_type kAlignment>
inline ExplicitFreeListAllocator<kPageSize, kAlignment>::ExplicitFreeListAllocator(const size_type& capacity, 
																 const PlacementPolicy& placement_policy, 
																 CoalescingPolicy& coalescing_policy) : ExplicitFreeListAllocator(capacity,
																														 placement_policy,
																														 coalescing_policy,
																														 AllocationPolicy::Static)
{}

template<size_type kPageSize, size_type kAlignment>
ExplicitFreeListAllocator<kPageSize, kAlignment>::ExplicitFreeListAllocator(const size_type& capacity, 
														  const PlacementPolicy& placement_policy, 
														  const CoalescingPolicy& coalescing_policy, 
														  const AllocationPolicy& allocation_policy) : Allocator(capacity, allocation_policy)
{
	start_ = malloc(capacity);

	assert(start_ != nullptr);

	placement_policy_ = placement_policy;
	coalescing_policy_ = coalescing_policy;

	free_list_ = reinterpret_cast<BlockPointer>(start_);
	if (free_list_ != nullptr)
	{
		free_list_->size = capacity;
		free_list_->prev = nullptr;
		free_list_->next = nullptr;
	}
	last_fit_ = free_list_;
}

template<size_type kPageSize, size_type kAlignment>
ExplicitFreeListAllocator<kPageSize, kAlignment>::~ExplicitFreeListAllocator()
{
	if (start_ != nullptr)
	{
		free(start_);
		start_ = nullptr;
	}
	free_list_ = nullptr;
	last_fit_ = nullptr;
}

template<size_type kPageSize, size_type kAlignment>
void* ExplicitFreeListAllocator<kPageSize, kAlignment>::Allocate(const size_type& size)
{
	assert(size > 0);

	BlockPointer fit_block = nullptr;

	size_type aligned_size, padding;

	Align(size, kAlignment, aligned_size, padding);

	Find(aligned_size, fit_block);

	assert(fit_block != nullptr);

	// split the fit block if there is some extra space
	size_type extra_space = fit_block->size - aligned_size;
	if (extra_space > kAlignment)
	{
		size_type new_block_address = reinterpret_cast<size_type>(fit_block) + aligned_size;
		BlockPointer new_block = reinterpret_cast<BlockPointer>(new_block_address);
		new_block->prev = nullptr;
		new_block->next = nullptr;
		new_block->size = extra_space;
		fit_block->size = aligned_size;
		Insert(fit_block, new_block);
	}
	
	// remove fit_block
	Remove(fit_block);

	size_type block_address = reinterpret_cast<size_type>(fit_block);

	size_type payload_start = block_address + sizeof(Block);

	allocated_ += aligned_size;
	freed_ -= aligned_size;

	peak_ = std::max(peak_, allocated_);


	//std::cout << "allocate: " << size << ", aligned_size: " << aligned_size << ", padding: " << padding << endl;
	//std::cout << "fit_block: " << block_address << ", size: " << fit_block->size << endl;
	//std::cout << "payload: " << payload_start << ", delta: " << payload_start - block_address << endl;

	//auto cur = free_list_;
	//while (cur != nullptr)
	//{
	//	std::cout << "free_list: " << reinterpret_cast<size_type>(cur) << ": " << cur->size << endl;
	//	cur = cur->next;
	//}
	//std::cout << endl;

	return reinterpret_cast<void*>(payload_start);
}

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::Free(void* ptr)
{
	assert(ptr != nullptr);

	size_type address = reinterpret_cast<size_type>(ptr);
	size_type block_address = address - sizeof(Block);

	// unordered insertion
	BlockPointer block = reinterpret_cast<BlockPointer>(block_address);

	allocated_ -= block->size;
	freed_ += block->size;

	if (free_list_ == nullptr)
	{
		free_list_ = block;
	}
	else
	{
		BlockPointer cur = free_list_;
		BlockPointer prev = nullptr;
		while (cur != nullptr)
		{
			size_type free_list_node_address = reinterpret_cast<size_type>(cur);
			if (free_list_node_address > block_address)
			{
				if (prev == nullptr)
				{
					Insert(block, cur);
					if (cur == free_list_)
					{
						free_list_ = block;
					}
				}
				else
				{
					Insert(prev, block);
				}

				// try merge
				if (block->next != nullptr)
				{
					size_type next_address = reinterpret_cast<size_type>(block->next);
					if (block_address + block->size == next_address)
					{
						block->size += block->next->size;
						Remove(block->next);
					}
				}

				if (block->prev != nullptr)
				{
					size_type prev_address = reinterpret_cast<size_type>(block->prev);
					if (prev_address + block->prev->size == block_address)
					{
						block->prev->size += block->size;
						Remove(block);
					}
				}

				break;
			}
			prev = cur;
			cur = cur->next;
		}
	}

	//std::cout << "free: " << address << ", " << block->size << endl;

	//auto cur = free_list_;
	//while (cur != nullptr)
	//{
	//	std::cout << "free_list: " << reinterpret_cast<size_type>(cur) << ": " << cur->size << endl;
	//	cur = cur->next;
	//}
	//std::cout << endl;
}

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::AllocateBlock(const size_type& size)
{

}

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::Find(const size_type& aligned_size, BlockPointer& found)
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

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::FindFirstFit(const size_type& aligned_size, BlockPointer& found)
{
	BlockPointer cur = free_list_;

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

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::FindNextFit(const size_type& aligned_size, BlockPointer& found)
{
	BlockPointer cur = last_fit_;

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

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::FindBestFit(const size_type& aligned_size, BlockPointer& found)
{
	size_type min_block = MAX_SIZE;
	BlockPointer min_block_pointer = nullptr;
	BlockPointer cur = free_list_;

	while (cur != nullptr)
	{
		if (cur->size >= aligned_size)
		{
			if (cur->size < min_block)
			{
				min_block = cur->size;
				min_block_pointer = cur;
			}
		}
		cur = cur->next;
	}

	found = min_block_pointer;
}

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::Align(const size_type& size, const size_type& alignment, size_type& aligned_size, size_type& padding)
{
	size_type n = size / alignment;
	padding = size - alignment * n;

	size_type metadata_size = sizeof(Block);

	if (padding < metadata_size)
	{
		size_type required_space = metadata_size - padding;
		size_type meta_n = required_space / alignment;
		size_type meta_padding = required_space - alignment * meta_n;
		padding += required_space + meta_padding;
	}

	aligned_size = size + padding;
}

template<size_type kPageSize, size_type kAlignment>
bool ExplicitFreeListAllocator<kPageSize, kAlignment>::IsAligned(const size_type& alignment)
{
	return (alignment & (alignment - 1)) == 0;
}

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::Insert(BlockPointer& node, BlockPointer& to_insert)
{
	if (to_insert != nullptr && node != nullptr)
	{
		BlockPointer next = node->next;
		if (next != nullptr)
		{
			next->prev = to_insert;
			to_insert->next = next;
		}
		node->next = to_insert;
		to_insert->prev = node;
	}
}

template<size_type kPageSize, size_type kAlignment>
void ExplicitFreeListAllocator<kPageSize, kAlignment>::Remove(BlockPointer& node)
{
	if (node != nullptr)
	{
		BlockPointer prev = node->prev;
		BlockPointer next = node->next;

		node->prev = nullptr;
		node->next = nullptr;

		if (prev != nullptr)
		{
			prev->next = next;
		}

		if (next != nullptr)
		{
			next->prev = prev;
		}

		if (node == free_list_)
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

// header "ExplicityFreeList.h"
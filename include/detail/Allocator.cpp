#include "Allocator.h"
#include <algorithm>

Allocator::Allocator(const size_type& capacity, const size_type& alignment, const size_type& page_size, const AllocationPolicy& allocation_policy)
{
	alignment_ = alignment;
	page_size_ = page_size;
	capacity_ = std::max(RoundUp(alignment, capacity), page_size);
	allocated_ = 0;
	freed_ = capacity;
	allocation_policy_ = allocation_policy;
	peak_ = 0;
};

Allocator::~Allocator()
{}

size_type Allocator::RoundUp(const size_type& alignment, const size_type& size)
{
	return (size + alignment - 1) & ~(alignment - 1);
}

size_type Allocator::FreedSize()
{
	return freed_;
}

size_type Allocator::AllocatedSize()
{
	return allocated_;
}

size_type Allocator::Capacity()
{
	return capacity_;
}

size_type Allocator::Peak()
{
	return peak_;
}
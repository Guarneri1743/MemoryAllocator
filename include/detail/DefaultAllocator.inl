template<size_type kPageSize, size_type kAlignment>
DefaultAllocator<kPageSize, kAlignment>::DefaultAllocator() : Allocator(0, AllocationPolicy::Dynamic)
{}

template<size_type kPageSize, size_type kAlignment>
DefaultAllocator<kPageSize, kAlignment>::~DefaultAllocator()
{}

template<size_type kPageSize, size_type kAlignment>
void* DefaultAllocator<kPageSize, kAlignment>::Allocate(const size_type& size)
{
	return malloc(size);
}

template<size_type kPageSize, size_type kAlignment>
void DefaultAllocator<kPageSize, kAlignment>::Free(void* ptr)
{
	free(ptr);
}
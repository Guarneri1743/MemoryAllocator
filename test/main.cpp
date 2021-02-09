#include <iostream>
#include <string>
#include "ExplicitFreeListAllocator.h"
#include "DefaultAllocator.h"
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;

class Statistics
{
public:
	Statistics(string title)
	{
		this->title_ = title;
	}

	string title_;
	double allocation_time_;
	double free_time_;
	size_type peak_;
	size_type capacity_;
	size_type execution_times_;

	void Dump()
	{
		cout << "[" << title_ << "]" << endl;
		cout << "Execution Time: " << execution_times_ << " Times" << endl;
		cout << "Allocation Time: " << setprecision(6) << allocation_time_ << " ms" << endl;
		cout << "Free Time: " << setprecision(6) << free_time_ << " ms" << endl;
		cout << "Allocation Time Per Execution: " << setprecision(6) << allocation_time_ / (double)execution_times_ << " ms/Time" <<endl;
		cout << "Free Time Per Execution: " << setprecision(6) << free_time_ / (double)execution_times_ << " ms/Time" << endl;
		if (peak_ > 0)
		{
			cout << "Peak: " << peak_ << " Byte(" << peak_ / (1 MB) << " MB)" << endl;
		}
		if (capacity_ > 0)
		{
			cout << "Capacity: " << capacity_ << " Byte(" << capacity_ / (1 MB) << " MB)" << endl;
		}
		cout << "===========================================================================" << endl;
	}
};

Statistics AllocateAndFree(string title, Allocator* allocator, vector<size_type> allocation_sizes)
{
	Statistics ret(title);

	auto start = chrono::steady_clock::now();
	vector<void*> addresses;
	for (auto& size : allocation_sizes)
	{
		addresses.emplace_back(allocator->Allocate(size));
	}
	ret.allocation_time_ = (double)(chrono::steady_clock::now() - start).count() / 1e+3f;

	start = chrono::steady_clock::now();
	for (auto& addr : addresses)
	{
		allocator->Free(addr);
	}
	ret.free_time_ = (double)(chrono::steady_clock::now() - start).count() / 1e+3f;
	ret.peak_ = allocator->Peak();
	ret.capacity_ = allocator->Capacity();
	ret.execution_times_ = allocation_sizes.size();

	return ret;
}

Statistics RandomAllocateAndFree(string title, Allocator* allocator, vector<size_type> allocation_sizes)
{
	Statistics ret(title);

	auto start = chrono::steady_clock::now();
	vector<void*> addresses;
	for (int i = 0; i < (int)allocation_sizes.size(); i++)
	{
		const int index = rand() % allocation_sizes.size();
		addresses.emplace_back(allocator->Allocate(allocation_sizes[index]));
	}
	ret.allocation_time_ = (double)(chrono::steady_clock::now() - start).count() / 1e+3f;

	start = chrono::steady_clock::now();
	for (auto& addr : addresses)
	{
		allocator->Free(addr);
	}
	ret.free_time_ = (double)(chrono::steady_clock::now() - start).count() / 1e+3f;
	ret.peak_ = allocator->Peak();
	ret.capacity_ = allocator->Capacity();
	ret.execution_times_ = allocation_sizes.size();

	return ret;
}

int main()
{
	Allocator* allocator1 = new ExplicitFreeListAllocator<>(1024 MB);
	Allocator* allocator2 = new ExplicitFreeListAllocator<>(1024 MB);
	Allocator* default_allocator = new DefaultAllocator<>();

	// below 128 bytes
	vector<size_type> small_allocation_sizes = { 1 BYTE, 3 BYTE, 4 BYTE, 7 BYTE, 8 BYTE, 16 BYTE, 27 BYTE, 32 BYTE, 57 BYTE, 64 BYTE, 77 BYTE, 96 BYTE};

	// above 128 bytes
	vector<size_type> large_allocation_sizes = { 128 BYTE, 1 KB, 2 KB, 4 KB, 16 KB, 64 KB, 1 MB, 4 MB, 32 MB, 64 MB };

	AllocateAndFree("Small Size Allocation(ExplicitFreeListAllocator)", allocator1, small_allocation_sizes).Dump();
	RandomAllocateAndFree("Random Small Size Allocation(ExplicitFreeListAllocator)", allocator2, small_allocation_sizes).Dump();
	AllocateAndFree("Fixed Small Size Allocation(malloc/free)", default_allocator, small_allocation_sizes).Dump();
	RandomAllocateAndFree("Random Small Size Allocation(malloc/free)", default_allocator, small_allocation_sizes).Dump();

	AllocateAndFree("Fixed Large Size Allocation(ExplicitFreeListAllocator)", allocator1, large_allocation_sizes).Dump();
	RandomAllocateAndFree("Random Large Size Allocation(ExplicitFreeListAllocator)", allocator2, large_allocation_sizes).Dump();
	AllocateAndFree("Fixed Large Size Allocation(malloc/free)", default_allocator, large_allocation_sizes).Dump();
	RandomAllocateAndFree("Random Large Size Allocation(malloc/free)", default_allocator, large_allocation_sizes).Dump();

	delete allocator1;
	delete allocator2;
	delete default_allocator;

	return 0;
}

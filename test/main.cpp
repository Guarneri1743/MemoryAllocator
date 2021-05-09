#include <iostream>
#include <string>
#include "ExplicitFreeListAllocator.h"
#include "CrtAllocator.h"
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
	mem_size_t execution_times_;

	void Dump()
	{
		cout << "===========================================================================" << endl;
		cout << "[" << title_ << "]" << endl;
		cout << "Execution Time: " << execution_times_ << " Times" << endl;
		cout << "Allocation Time: " << setprecision(6) << allocation_time_ << " ms" << endl;
		cout << "Free Time: " << setprecision(6) << free_time_ << " ms" << endl;
		cout << "Allocation Time Per Execution: " << setprecision(6) << allocation_time_ / (double)execution_times_ << " ms/Time" <<endl;
		cout << "Free Time Per Execution: " << setprecision(6) << free_time_ / (double)execution_times_ << " ms/Time" << endl;
		cout << "===========================================================================" << endl;
	}
};

Statistics AllocateAndFree(string title, ExplicitFreeListAllocator* allocator, vector<mem_size_t> allocation_sizes)
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
	ret.execution_times_ = allocation_sizes.size();

	return ret;
}

Statistics RandomAllocateAndFree(string title, ExplicitFreeListAllocator* allocator, vector<mem_size_t> allocation_sizes)
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
	ret.execution_times_ = allocation_sizes.size();

	return ret;
}

int main()
{
	ExplicitFreeListAllocator* allocator1 = new ExplicitFreeListAllocator(128 MB);
	ExplicitFreeListAllocator* allocator2 = new ExplicitFreeListAllocator(128 MB);
	CrtAllocator* default_allocator = new CrtAllocator();

	// below 128 bytes
	vector<mem_size_t> small_allocation_sizes = { 1 BYTE, 3 BYTE, 4 BYTE, 7 BYTE, 8 BYTE, 16 BYTE, 27 BYTE, 32 BYTE, 57 BYTE, 64 BYTE, 77 BYTE, 96 BYTE};

	// above 128 bytes
	vector<mem_size_t> large_allocation_sizes = { 128 BYTE, 1 KB, 2 KB, 4 KB, 16 KB, 64 KB, 1 MB, 4 MB, 32 MB, 64 MB };

	AllocateAndFree("Small Size Allocation(ExplicitFreeListAllocator)", allocator1, small_allocation_sizes).Dump();
	RandomAllocateAndFree("Random Small Size Allocation(ExplicitFreeListAllocator)", allocator2, small_allocation_sizes).Dump();

	delete allocator1;
	delete allocator2;
	delete default_allocator;

	return 0;
}

#include "TwoLevelSegregateFit.h"
#include <assert.h>
#include <cmath>
#include <math.h>
#include <algorithm>

namespace Tlsf
{
	Pool::Pool(void* ptr, mem_size_t pool_size)
	{
		assert(ptr != nullptr && pool_size != 0);
		void* heap_start_ = ptr;
		mem_size_t heap_end_addr = reinterpret_cast<mem_size_t>(ptr) + pool_size;
		heap_end_ = reinterpret_cast<void*>(heap_end_addr);
	}

	Pool::~Pool()
	{}

	void* Pool::Alloc(mem_size_t size)
	{
		return nullptr;
	}

	void Pool::Free(void* ptr)
	{}

	void Pool::MappingSearch(mem_size_t size, mem_size_t& fl, mem_size_t& sl)
	{
		size = size + (kTlsfOne << (FindLastBitSet(size) - kTlsfSli)) - kTlsfOne;
		fl = FindLastBitSet(size);
		sl = (size >> (fl - kTlsfSli)) ^ (kTlsfOne << kTlsfSli);
	}

	Block* Pool::SearchSuitableBlock(mem_size_t& fl, mem_size_t& sl)
	{
		mem_size_t sl_bitmap = sl_bitmap_[fl] & (~kTlsfZero << sl);
		if (sl_bitmap == kTlsfZero)
		{
			mem_size_t fl_bitmap = fl_bitmap_ & (~kTlsfZero << (fl + kTlsfOne));
			if (fl_bitmap == kTlsfZero)
			{
				return nullptr;
			}

			fl = FindFirstBitSet(fl_bitmap);
			sl_bitmap = sl_bitmap_[fl];
		}

		assert(sl_bitmap != 0);
		sl = FindFirstBitSet(sl_bitmap);

		return blocks_[fl][sl];
	}

	void Pool::Remove(Block* block)
	{
		mem_size_t fl, sl;
		MappingSearch(GetSize(block), fl, sl);
		RemoveFreeBlock(block, fl, sl);
	}

	void Pool::Insert(Block* block)
	{
		mem_size_t fl, sl;
		MappingSearch(GetSize(block), fl, sl);
		InsertFreeBlock(block, fl, sl);
	}

	bool Pool::CanSplit(Block* block, mem_size_t size)
	{
		return GetSize(block) >= sizeof(Block) + size;
	}

	void Pool::Split(Block* block, mem_size_t size)
	{
		
	}

	void Pool::Coalesce(Block* lhs, Block* rhs)
	{}

	void Pool::CoalesceLeft(Block* block)
	{}

	void Pool::CoalesceRight(Block* block)
	{}

	void Pool::RemoveFreeBlock(Block* block, mem_size_t fl, mem_size_t sl)
	{
		Block* prev = block->prev;
		Block* next = block->next;

		assert(prev != nullptr && next != nullptr);

		next->prev = prev;
		prev->next = next;

		if (blocks_[fl][sl] == block)
		{
			// update block list head
			blocks_[fl][sl] = next;

			if (nullptr == next)
			{
				// clear second level bitmap
				sl_bitmap_[fl] &= ~(kTlsfOne << sl);

				if (sl_bitmap_[fl] != 0)
				{
					// clear first level bitmap
					fl_bitmap_ &= ~(kTlsfOne << fl);
				}
			}
		}
	}

	void Pool::InsertFreeBlock(Block* block, mem_size_t fl, mem_size_t sl)
	{
		// insert the block at the head of free list
		Block* head = blocks_[fl][sl];
		block->next = head;
		block->prev = nullptr;
		head->prev = block;

		// update block list head
		blocks_[fl][sl] = block;

		// update second bitmap
		fl_bitmap_ |= (kTlsfOne << fl);
		sl_bitmap_[fl] |= (kTlsfOne << sl);
	}
}
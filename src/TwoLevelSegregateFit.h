#pragma once
#include "Define.h"
#include <cstddef>

namespace Tlsf
{
	struct Block
	{
		Block* prev_phys_block;
		mem_size_t size;
		Block* prev;
		Block* next;
	};

	constexpr mem_size_t kTlsfFli = static_cast<mem_size_t>(30);
	constexpr mem_size_t kTlsfSli = static_cast<mem_size_t>(5);
	constexpr mem_size_t kTlsfOne = static_cast<mem_size_t>(1);
	constexpr mem_size_t kTlsfZero = static_cast<mem_size_t>(0);
	constexpr mem_size_t kBlockFreeBit = kTlsfOne << kTlsfZero;
	constexpr mem_size_t kBlockPrevFreeBit = kTlsfOne << kTlsfOne;
	constexpr mem_size_t kBlockFreeBits = kBlockFreeBit | kBlockPrevFreeBit;
	constexpr mem_size_t kBlockPayloadOffset = offsetof(Block, size) + sizeof(mem_size_t);
	constexpr mem_size_t kMinBlockSize = sizeof(Block) - sizeof(Block*);
	constexpr mem_size_t kMaxBlockSize = kTlsfOne << kTlsfFli;

	class Pool
	{
	private:
		mem_size_t fl_bitmap_;
		mem_size_t sl_bitmap_[kTlsfFli];
		Block* blocks_[kTlsfFli][kTlsfSli];

		void* heap_start_;
		void* heap_end_;
		mem_size_t pool_size_;

	public:
		Pool(void* ptr, mem_size_t pool_size);
		~Pool();

		void* Alloc(mem_size_t size);
		void Free(void* ptr);

	private:
		void MappingSearch(mem_size_t size, mem_size_t& fl, mem_size_t& sl);
		Block* SearchSuitableBlock(mem_size_t& fl, mem_size_t& sl);
		void Remove(Block* block);
		void Insert(Block* block);
		void Split(Block* block, mem_size_t size);
		void Coalesce(Block* lhs, Block* rhs);
		void CoalesceLeft(Block* block);
		void CoalesceRight(Block* block);
		bool CanSplit(Block* block, mem_size_t size);
		void RemoveFreeBlock(Block* block, mem_size_t fl, mem_size_t sl);
		void InsertFreeBlock(Block* block, mem_size_t fl, mem_size_t sl);

		inline void* BlockPtr2PayloadPtr(Block* block) noexcept
		{
			return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(block) + kBlockPayloadOffset);
		}

		inline void* PayloadPtr2BlockPtr(void* payload) noexcept
		{
			return reinterpret_cast<Block*>(reinterpret_cast<unsigned char*>(payload) - kBlockPayloadOffset);
		}

		inline Block* GetNextBlock(void* ptr, mem_size_t size)
		{
			return reinterpret_cast<Block*>(reinterpret_cast<ptrdiff_t>(ptr) + size);
		}

		inline mem_size_t GetSize(Block* block) noexcept
		{
			return block->size & ~kBlockFreeBits;
		}

		inline bool IsBlockFree(Block* block) noexcept
		{
			return (block->size & kBlockFreeBit) == kTlsfOne;
		}

		inline bool IsPrevBlockFree(Block* block) noexcept
		{
			return (block->size & kBlockPrevFreeBit) == kTlsfOne;
		}

		inline mem_size_t SetSize(Block* block, mem_size_t size) noexcept
		{
			block->size = size | (block->size & kBlockFreeBits);
		}

		inline bool SetBlockFree(Block* block, bool is_free) noexcept
		{
			block->size = is_free ? (block->size | kBlockFreeBit) : (block->size & ~kBlockFreeBit);
		}

		inline bool SetPrevBlockFree(Block* block, bool is_free) noexcept
		{
			block->size = is_free ? (block->size | kBlockPrevFreeBit) : (block->size & ~kBlockPrevFreeBit);
		}
	};
}
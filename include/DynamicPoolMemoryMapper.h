#pragma once

#include <algorithm>

#include "NativeType.h"
#include "CoreAllocatorHelper.h"
#include "LinkedMemoryUtility.h"

namespace CHRONE::MEMORY
{

template<
	class Allocator,
	class ChunkType,
	Uint32 ChunkCount>
	class DynamicPoolMemoryMapper final
{
public:
	DynamicPoolMemoryMapper() = default;
	~DynamicPoolMemoryMapper();

	DynamicPoolMemoryMapper(const DynamicPoolMemoryMapper&) = delete;
	DynamicPoolMemoryMapper(DynamicPoolMemoryMapper&&) = delete;

	explicit DynamicPoolMemoryMapper(Allocator allocator);

	DynamicPoolMemoryMapper&	operator=(const DynamicPoolMemoryMapper&) = delete;
	DynamicPoolMemoryMapper&	operator=(DynamicPoolMemoryMapper&&) = delete;

	void	Reserve(Uint32 chunkCount);

	ChunkType*	MapMemory();
	void	MapMemory(Uint32 count, ChunkType** chunks);

	void	UnmapMemory(ChunkType* chunk);
	void	UnmapMemory(Uint32 count, ChunkType** chunks);

	void	UnmapAllMemory();
	void	Reset();

private:

	using RealChunkType = std::conditional_t < sizeof(void*) < sizeof(ChunkType), ChunkType, PtrSize > ;
	using RealChunkPtrType = RealChunkType*;
	static constexpr auto	realChunkSize{ sizeof(RealChunkType) };
	static constexpr auto	ptrByteSize{ sizeof(PtrSize) };
	static constexpr auto	poolSize{ ptrByteSize + ChunkCount * realChunkSize };

	void	_ReservePools(Uint32 poolCount);
	void	_ReserveChunks(Uint32 chunksCount);
	void*	_ReserveLinkedPool();

	void	_MapMemory(Uint32 count, ChunkType** chunks);
	void	_UnmapMemory(Uint32 count, ChunkType** chunks);

	void	_UnmapAllMemory();
	void	_Reset();

	Uint32	_ComputeMissingChunkCount(Uint32 count) const;

	Uint32	_poolsCount{ 0u };
	void*	_memoryBegin{ nullptr };
	void*	_memoryEnd{ nullptr };

	Uint32	_freeChunksCount{ 0u };
	RealChunkPtrType	_freeChunksBegin{ nullptr };
	RealChunkPtrType	_freeChunksEnd{ nullptr };

	Allocator	_allocator{};
};


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::~DynamicPoolMemoryMapper()
{
	Reset();
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::DynamicPoolMemoryMapper(
	Allocator allocator):
	_allocator{std::move(allocator)}
{
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::Reserve(
	Uint32 chunkCount)
{
	_ReserveChunks(chunkCount);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline ChunkType* 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::MapMemory()
{
	ChunkType*	allocation{ nullptr };

	_MapMemory(1, &allocation);

	return allocation;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::MapMemory(
	Uint32 count, 
	ChunkType** chunks)
{
	if (count <= 0u || !chunks) { return; }
	_MapMemory(count, chunks);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::UnmapMemory(
	ChunkType* chunk)
{
	if (!chunk) { return; }
	_UnmapMemory(1, &chunk);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::UnmapMemory(
	Uint32 count, 
	ChunkType** chunks)
{
	if (count <= 0u || !chunks) { return; }
	_UnmapMemory(count, chunks);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::UnmapAllMemory()
{
	if (!_memoryBegin) { return; }
	_UnmapAllMemory();
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::Reset()
{
	if (!_memoryBegin) { return; }
	_Reset();
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_ReservePools(
	Uint32 poolCount)
{
	Uint32 const	addedPoolCount{ poolCount };

	if (!_memoryBegin || !_freeChunksBegin)
	{
		void*	unlinkedPool{ _ReserveLinkedPool() };

		if (!_memoryBegin) { _memoryBegin = unlinkedPool; }
		else { LinkedMemoryUtility::WritePtrToMemory(_memoryEnd, unlinkedPool); }

		_memoryEnd = unlinkedPool;
		--poolCount;
		_freeChunksBegin = reinterpret_cast<RealChunkPtrType>(
			reinterpret_cast<PtrSize*>(unlinkedPool) + 1);
		_freeChunksEnd = std::addressof(_freeChunksBegin[ChunkCount - 1]);
	}

	for (Uint32 index{ 0u }; index < poolCount; ++index)
	{
		void*	allocatedPool{ _ReserveLinkedPool() };
		RealChunkPtrType	poolChunksBegin{ reinterpret_cast<RealChunkPtrType>(
			reinterpret_cast<PtrSize*>(allocatedPool) + 1) };

		LinkedMemoryUtility::WritePtrToMemory(_memoryEnd, allocatedPool);
		LinkedMemoryUtility::WritePtrToMemory(_freeChunksEnd, poolChunksBegin);

		_memoryEnd = allocatedPool;
		_freeChunksEnd = poolChunksBegin;
	}

	_poolsCount += addedPoolCount;
	_freeChunksCount += (addedPoolCount * ChunkCount);

	LinkedMemoryUtility::WriteNullToMemory(_freeChunksEnd);
	LinkedMemoryUtility::WriteNullToMemory(_memoryEnd);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_ReserveChunks(
	Uint32 chunksCount)
{
	if (Uint32 const allocationChunkCount{ 
		_ComputeMissingChunkCount(chunksCount) })
	{
		Uint32 const	allocatedPool{ 
			((allocationChunkCount - 1u) / ChunkCount) + 1u };

		_ReservePools(allocatedPool);
	}
}

template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void* 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_ReserveLinkedPool()
{
	void*	allocatedPool{ 
		CoreAllocatorHelper<Allocator>::Allocate(_allocator, poolSize) };

	if (allocatedPool)
	{
		RealChunkPtrType	chunks{
			reinterpret_cast<RealChunkPtrType>(
				reinterpret_cast<PtrSize*>(allocatedPool) + 1) };

		LinkedMemoryUtility::LinkPtrs(ChunkCount, chunks, realChunkSize);
	}
	
	return allocatedPool;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_MapMemory(
	Uint32 count, ChunkType** chunks)
{
	_ReserveChunks(count);

	chunks[0] = nullptr;

	for (auto index{ 0u }; index < count; ++index)
	{
		chunks[index] = reinterpret_cast<ChunkType*>(_freeChunksBegin);
		_freeChunksBegin = reinterpret_cast<RealChunkType*>(
			*reinterpret_cast<PtrSize*>(_freeChunksBegin));
	}

	_freeChunksCount -= count;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_UnmapMemory(
	Uint32 count, 
	ChunkType** chunks)
{
	if (!_freeChunksBegin)
	{
		_freeChunksBegin = reinterpret_cast<RealChunkPtrType>(chunks[0]);
		_freeChunksEnd = _freeChunksBegin;
	}

	LinkedMemoryUtility::WritePtrToMemory(_freeChunksEnd, chunks[0]);
	LinkedMemoryUtility::LinkPtrs(count, chunks, realChunkSize);
	_freeChunksEnd = reinterpret_cast<RealChunkPtrType>(chunks[count - 1]);
	LinkedMemoryUtility::WriteNullToMemory(_freeChunksEnd);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_UnmapAllMemory()
{
	void*	currentPoolMemory{ _memoryBegin };
	_freeChunksEnd = _freeChunksBegin;

	for (Uint32 poolIndex{ 0u }; poolIndex < _poolsCount; ++poolIndex)
	{
		PtrSize*	poolHeader{ reinterpret_cast<PtrSize*>(currentPoolMemory) };
		RealChunkPtrType	poolChunksBegin{ 
			reinterpret_cast<RealChunkPtrType>(poolHeader + 1) };

		LinkedMemoryUtility::WritePtrToMemory(_freeChunksEnd, poolChunksBegin);
		LinkedMemoryUtility::LinkPtrs(ChunkCount, poolChunksBegin);
		currentPoolMemory = reinterpret_cast<void*>(*poolHeader);
		_freeChunksEnd = poolChunksBegin;
	}

	_freeChunksCount = _poolsCount * ChunkCount;
	WriteMemoryNullPtr(_freeChunksEnd);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_Reset()
{
	void*	currentPoolMemory{ _memoryBegin };

	for (Uint32 poolIndex{ 0u }; poolIndex < _poolsCount; ++poolIndex)
	{
		PtrSize*	poolHeader{ reinterpret_cast<PtrSize*>(currentPoolMemory) };
		CoreAllocatorHelper<Allocator>::Deallocate(_allocator, currentPoolMemory);
		currentPoolMemory = reinterpret_cast<void*>(*poolHeader);
	}

	_poolsCount = 0u;
	_memoryBegin = nullptr;
	_memoryEnd = nullptr;

	_freeChunksCount = 0u;
	_freeChunksBegin = nullptr;
	_freeChunksEnd = nullptr;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline Uint32 
DynamicPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_ComputeMissingChunkCount(
	Uint32 count) const
{
	const Int64	countDiff{ 
		static_cast<Int64>(_freeChunksCount) - static_cast<Int64>(count) };

	return countDiff < 0 ? static_cast<Uint32>(std::abs(countDiff)) : 0u;
}

}
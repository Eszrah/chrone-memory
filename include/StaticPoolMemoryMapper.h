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
class StaticPoolMemoryMapper final
{
public:
	StaticPoolMemoryMapper() = default;
	~StaticPoolMemoryMapper();

	StaticPoolMemoryMapper(const StaticPoolMemoryMapper&) = delete;
	StaticPoolMemoryMapper(StaticPoolMemoryMapper&&) = delete;

	explicit StaticPoolMemoryMapper(Allocator allocator);

	StaticPoolMemoryMapper&	operator=(const StaticPoolMemoryMapper&) = delete;
	StaticPoolMemoryMapper&	operator=(StaticPoolMemoryMapper&&) = delete;

	void	Initialize();

	ChunkType*	MapMemory();
	void	MapMemory(Uint32 count, ChunkType** chunks);

	void	UnmapMemory(ChunkType* chunk);
	void	UnmapMemory(Uint32 count, ChunkType** chunks);

	void	UnmapAllMemory();
	void	Reset();

private:

	using RealChunkType = std::conditional_t<sizeof(void*) < sizeof(ChunkType), ChunkType, PtrSize>;
	using RealChunkPtrType = RealChunkType*;
	static constexpr auto	realChunkSize{ sizeof(RealChunkType) };
	static constexpr auto	poolSize{ ChunkCount * realChunkSize };

	void	_MapMemory(Uint32 count, ChunkType** chunks);
	void	_UnmapMemory(Uint32 count, ChunkType** chunks);

	void	_AllocateChunks();
	void	_AssembleChunks();
	void	_UnmapAllMemory();
	void	_Reset();

	bool	_HasEnoughChunks(Uint32 count) const;

	RealChunkPtrType	_memory{ nullptr };

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
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::~StaticPoolMemoryMapper()
{
	Reset();
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::StaticPoolMemoryMapper(
	Allocator allocator):
	_allocator{std::move(allocator)}
{
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::Initialize()
{
	_AllocateChunks();
}


template<
	class Allocator,
	class ChunkType, 
	Uint32 ChunkCount>
inline ChunkType* 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::MapMemory()
{
	ChunkType*	allocation{ nullptr };

	_AllocateChunks();

	if (_HasEnoughChunks(1))
	{
		_MapMemory(1, &allocation);
	}

	return allocation;
}

template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::MapMemory(
	Uint32 count, 
	ChunkType** chunks)
{
	_AllocateChunks();

	if (_HasEnoughChunks(count))
	{
		_MapMemory(count, chunks);
	}
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::UnmapMemory(
	ChunkType* chunk)
{
	if (!chunk || !_memory) { return; }
	_UnmapMemory(1, &chunk);
}


template<class Allocator, class ChunkType, Uint32 ChunkCount>
inline void StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::UnmapMemory(Uint32 count, ChunkType** chunks)
{
	if (count <= 0u || chunks || !_memory) { return; }
	_UnmapMemory(count, chunks);
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::UnmapAllMemory()
{
	if (!_memory) { return; }
	_UnmapAllMemory();
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::Reset()
{
	if (!_memory) { return; }
	_Reset();
}


template<
	class Allocator,
	class ChunkType,
	Uint32 ChunkCount>
inline void
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_MapMemory(
		Uint32 count,
		ChunkType** chunks)
{
	chunks[0] = nullptr;

	for (auto index{ 0u }; index < count; ++index)
	{
		chunks[index] = reinterpret_cast<ChunkType*>(_freeChunksBegin);
		_freeChunksBegin = reinterpret_cast<RealChunkType*>(*reinterpret_cast<PtrSize*>(_freeChunksBegin));
	}

	_freeChunksCount -= count;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_UnmapMemory(
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
	_freeChunksCount += count;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_AllocateChunks()
{
	if (!_memory)
	{
		_memory = static_cast<RealChunkPtrType>(CoreAllocatorHelper<Allocator>::Allocate(_allocator, poolSize));

		if (_memory)
		{
			_AssembleChunks();
		}
	}
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_AssembleChunks()
{
	_freeChunksBegin = _memory;
	LinkedMemoryUtility::LinkPtrs(ChunkCount, _freeChunksBegin, realChunkSize);
	_freeChunksEnd = _memory + ChunkCount - 1;
	LinkedMemoryUtility::WriteNullToMemory(_freeChunksEnd);
	_freeChunksCount = ChunkCount;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_UnmapAllMemory()
{
	_AssembleChunks();
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline void 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_Reset()
{
	CoreAllocatorHelper<Allocator>::Deallocate(_allocator, _memory);
	_memory = nullptr;
	_freeChunksCount = 0u;
	_freeChunksBegin = nullptr;
	_freeChunksEnd = nullptr;
}


template<
	class Allocator, 
	class ChunkType, 
	Uint32 ChunkCount>
inline bool 
StaticPoolMemoryMapper<Allocator, ChunkType, ChunkCount>::_HasEnoughChunks(
	Uint32 count) const
{
	return _freeChunksCount && count && static_cast<Int64>(_freeChunksCount) - static_cast<Int64>(count) >= 0;
}

}
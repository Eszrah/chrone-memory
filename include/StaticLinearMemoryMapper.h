#pragma once

#include <utility>

#include "NativeType.h"
#include "CoreAllocatorHelper.h"

namespace CHRONE::MEMORY
{

template<
	class Allocator,
	Uint32 size>
class StaticLinearMemoryMapper
{
public:
	StaticLinearMemoryMapper() = default;
	~StaticLinearMemoryMapper();

	StaticLinearMemoryMapper(const StaticLinearMemoryMapper&) = delete;
	StaticLinearMemoryMapper(StaticLinearMemoryMapper&&) = delete;

	explicit StaticLinearMemoryMapper(Allocator allocator);

	StaticLinearMemoryMapper&	operator=(const StaticLinearMemoryMapper&) = delete;
	StaticLinearMemoryMapper&	operator=(StaticLinearMemoryMapper&&) = delete;


	void	Initialize();
	inline void*	MapMemory(Uint32 byteCount);
	inline void	MapMemory(Uint32 byteCount, Uint32 count, void** memory);

	inline void	UnmapAllMemory();
	inline void	Reset();

private:

	inline void	_MapMemory(Uint32 byteCount, Uint32 count, void** memory);
	inline bool	_HasEnoughMemory(Uint32 byteCount) const;

	Uint32	_size{ 0u };
	Uint32	_offset{ 0u };
	UChar*	_memory{ nullptr };
	Allocator	_allocator{};
};


template<
	class Allocator, 
	Uint32 size>
inline 
StaticLinearMemoryMapper<Allocator, size>::~StaticLinearMemoryMapper()
{
	Reset();
}


template<
	class Allocator, 
	Uint32 size>
inline 
StaticLinearMemoryMapper<Allocator, size>::StaticLinearMemoryMapper(
	Allocator allocator):
	_allocator{std::move(allocator)}
{
}


template<
	class Allocator,
	Uint32 size>
inline void 
StaticLinearMemoryMapper<Allocator, size>::Initialize()
{
	if (!_memory)
	{
		_memory = static_cast<UChar*>(CoreAllocatorHelper<Allocator>::Allocate(_allocator, size));

		if (_memory) 
		{ 
			_size = size; 
			_offset = 0u;
		}
	}
}


template<
	class Allocator,
	Uint32 size>
inline void* 
StaticLinearMemoryMapper<Allocator, size>::MapMemory(
	Uint32 byteCount)
{
	void*	allocation{ nullptr };

	if (!_HasEnoughMemory(byteCount)) { return allocation; }

	_MapMemory(byteCount, 1, &allocation);
	return allocation;
}


template<
	class Allocator, 
	Uint32 size>
inline void 
StaticLinearMemoryMapper<Allocator, size>::MapMemory(
	Uint32 byteCount, 
	Uint32 count, 
	void** memory)
{
	if (!memory || !count || !_HasEnoughMemory(count * byteCount)) { return; }
	_MapMemory(byteCount, count, &allocation);
}


template<
	class Allocator,
	Uint32 size>
inline void 
StaticLinearMemoryMapper<Allocator, size>::UnmapAllMemory()
{
	if (!_memory) { return; }
	_size = 0u;
	_offset = 0u;
}


template<
	class Allocator,
	Uint32 size>
inline void 
StaticLinearMemoryMapper<Allocator, size>::Reset()
{
	if (!_memory) { return; }

	CoreAllocatorHelper<Allocator>::Deallocate(_allocator, _memory);
	_size = 0u;
	_offset = 0u;
	_memory = nullptr;
}


template<
	class Allocator, 
	Uint32 size>
inline void 
StaticLinearMemoryMapper<Allocator, size>::_MapMemory(
	Uint32 byteCount, 
	Uint32 count, 
	void** memory)
{
	for (Uint32 index{ 0u }; index < count; ++index)
	{
		memory[index] = _memory + _offset;
		_size -= byteCount;
		_offset += byteCount;
	}
}


template<
	class Allocator,
	Uint32 size>
inline bool 
StaticLinearMemoryMapper<Allocator, size>::_HasEnoughMemory(
	Uint32 byteCount) const
{
	return _size && byteCount && static_cast<Int32>(_size) - static_cast<Int32>(byteCount) > 0;
}

}
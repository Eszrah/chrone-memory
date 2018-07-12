#pragma once

#include <cstdlib>

namespace CHRONE::MEMORY
{

struct MallocCoreAllocator final
{
	using ByteCountType = size_t;

	inline void*	Allocate(ByteCountType byteCount)
	{
		return malloc(byteCount * sizeof(char));
	}

	inline void	Deallocate(void* memory)
	{
		free(memory);
	}
};

struct NewCoreAllocator final
{
	using ByteCountType = size_t;

	inline void*	Allocate(ByteCountType byteCount)
	{
		return new char[byteCount];
	}

	inline void	Deallocate(void* memory)
	{
		delete[] memory;
	}
};

}

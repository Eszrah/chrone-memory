#pragma once

#include <memory>

#include "NativeType.h"

namespace CHRONE::MEMORY
{

struct LinkedMemoryUtility
{
	static void	LinkPtrs(const Uint32 count, void* memory, const Uint32 byteOffset);

	static void	LinkPtrs(const Uint32 count, void** memory);

	static void	WritePtrToMemory(void* chunkL, void const* chunkR);

	static void	WriteNullToMemory(void* memory);
};


inline void
LinkedMemoryUtility::LinkPtrs(
	const Uint32 count, 
	void* memory,
	const Uint32 byteOffset)
{
	char*	byteMemory{ static_cast<char*>(memory) };

	for (Uint32 index{ 0u }; index < count - 1; ++index)
	{
		WritePtrToMemory(std::addressof(byteMemory[index*byteOffset]), std::addressof(byteMemory[index*byteOffset + byteOffset]));
	}
}


inline void 
LinkedMemoryUtility::LinkPtrs(
	const Uint32 count, 
	void** memory)
{
	for (Uint32 index{ 0u }; index < count - 1; ++index)
	{
		WritePtrToMemory(memory[index], memory[index + 1]);
	}
}


inline void
LinkedMemoryUtility::WritePtrToMemory(
	void* chunkL,
	void const* chunkR)
{
	PtrSize*	nextLink{ reinterpret_cast<PtrSize*>(chunkL) };
	*nextLink = reinterpret_cast<PtrSize>(chunkR);
}


inline void 
LinkedMemoryUtility::WriteNullToMemory(
	void* memory)
{
	WritePtrToMemory(memory, NULL);
}

}
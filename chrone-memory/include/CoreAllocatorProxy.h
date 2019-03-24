#pragma once

#include "NativeType.h"

namespace chrone::memory
{

struct CoreAllocatorProxy 
{

	inline static void* 
		Allocate(
		void* /*allocatorData*/, 
		Uint32 byteCount)
	{
		return new Char[byteCount];
	}

	inline static void 
		Deallocate(
		void* /*allocatorData*/, 
		void* memory)
	{
		delete[] memory;
	}

};

}
#pragma once

#include <utility>

#include "NativeType.h"
#include "CoreAllocatorProxy.h"

namespace chrone::memory
{

struct LinearAllocatorProxy
{
	void*	(*Allocate)(void*, Uint32) { CoreAllocatorProxy::Allocate };
	void	(*Deallocate)(void*, void*) { CoreAllocatorProxy::Deallocate };
	void*	allocatorData{ nullptr };
};


struct StaticLinearMemoryMapper
{
	StaticLinearMemoryMapper(Uint32	const size, 
		LinearAllocatorProxy allocatorProxy):
		size{ size },
		offset{ size },
		memory{ nullptr },
		allocatorProxy{ allocatorProxy }
	{}

	Uint32	size{ 0u };
	Uint32	offset{ 0u };
	Char*	memory{ nullptr };
	LinearAllocatorProxy	allocatorProxy{};
};


struct DynamicLinearMemoryMapper
{
	DynamicLinearMemoryMapper(Uint32 const size,
		LinearAllocatorProxy allocatorProxy) :
		blockSize{ size },
		begin{ nullptr },
		end{ nullptr },
		current{ nullptr },
		allocatorProxy{ allocatorProxy }
	{}

	struct BufferNode 
	{
		Uint32	offset{ 0u };
		Char*	memory{ nullptr };
		BufferNode*	next{};
	};

	Uint32	blockSize{ 0u };
	BufferNode*	begin{ nullptr };
	BufferNode*	end{ nullptr };
	BufferNode*	current{ nullptr };
	LinearAllocatorProxy	allocatorProxy{};
};


}
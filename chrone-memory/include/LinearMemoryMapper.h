#pragma once

#include <utility>

#include "NativeType.h"
#include "CoreAllocatorProxy.h"

namespace chrone::memory
{

struct Buffer 
{
	Uint32	size{ 0u };
	Uint32	offset{ 0u };
	Char*	memory{ nullptr };
};


struct LinearAllocatorProxy
{
	void*	(*Allocate)(void*, Uint32) { CoreAllocatorProxy::Allocate };
	void	(*Deallocate)(void*, void*) { CoreAllocatorProxy::Deallocate };
	void*	allocatorData{ nullptr };
};


struct StaticLinearMemoryMapper
{
	Buffer	buffer;
	LinearAllocatorProxy	allocatorProxy{};
};


struct DynamicLinearMemoryMapper
{
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
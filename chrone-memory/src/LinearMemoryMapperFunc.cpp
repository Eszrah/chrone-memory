#include "LinearMemoryMapperFunc.h"

#include "LinearMemoryMapper.h"


namespace chrone::memory
{

DynamicLinearMemoryMapper::BufferNode*
_AllocateBufferNode(LinearAllocatorProxy allocator, Uint32 blockSize);

Char* 
_MapMemory(Char* memory, Uint32& offset, Uint32 byteCount);

static void
_Allocate(DynamicLinearMemoryMapper& mapper);


void
LinearMemoryMapperFunc::Allocate(StaticLinearMemoryMapper& mapper)
{
	Uint32 const size{ mapper.size };
	Char const* const memory{ mapper.memory };

	if (memory != nullptr || size == 0) { return; }

	Char* const bufferMemory{ static_cast<Char*>(mapper.allocatorProxy.Allocate(
		mapper.allocatorProxy.allocatorData, size)) };

	mapper.memory = bufferMemory;
	mapper.offset = bufferMemory == nullptr ? size : 0u;
}


Char*
LinearMemoryMapperFunc::MapMemory(StaticLinearMemoryMapper& mapper,
	Uint32 const byteCount)
{
	Uint32 const size{ mapper.size };

	if (mapper.memory == nullptr)
	{
		Allocate(mapper);

		if (mapper.memory == nullptr) { return nullptr; }
	}

	if ((size - mapper.offset) < byteCount) { return nullptr; }

	return _MapMemory(mapper.memory, mapper.offset, byteCount);
}


void
LinearMemoryMapperFunc::Clear(StaticLinearMemoryMapper& mapper)
{
	if (mapper.memory == nullptr) { return; }
	mapper.offset = 0;
}


void
LinearMemoryMapperFunc::Reset(StaticLinearMemoryMapper& mapper)
{
	if (mapper.memory == nullptr) { return; }
	LinearAllocatorProxy const& allocator{ mapper.allocatorProxy };
	allocator.Deallocate(allocator.allocatorData, mapper.memory);
	mapper = StaticLinearMemoryMapper{ 0u, allocator };
}


void
LinearMemoryMapperFunc::Allocate(DynamicLinearMemoryMapper& mapper)
{
	Uint32 const blockSize{ mapper.blockSize };

	if (mapper.begin != nullptr || blockSize == 0u) { return; }
	_Allocate(mapper);
}


Char*
LinearMemoryMapperFunc::MapMemory(DynamicLinearMemoryMapper& mapper,
	Uint32 const byteCount)
{
	Uint32 const blockSize{ mapper.blockSize };

	if (byteCount > blockSize || blockSize == 0u) { return nullptr; }

	if (mapper.begin == nullptr)
	{
		_Allocate(mapper);
		if (mapper.begin == nullptr) { return nullptr; }
	}

	DynamicLinearMemoryMapper::BufferNode*	mappedBufferNode{ mapper.current };
	Uint32 const offset{ mappedBufferNode->offset };

	if ((blockSize - offset) < byteCount)
	{
		DynamicLinearMemoryMapper::BufferNode*	nextMappedBufferNode{
			mappedBufferNode->next };

		if (nextMappedBufferNode == nullptr)
		{
			nextMappedBufferNode =
				_AllocateBufferNode(mapper.allocatorProxy, blockSize);

			if (nextMappedBufferNode == nullptr) { return nullptr; }

			mapper.end->next = nextMappedBufferNode;
			mapper.end = nextMappedBufferNode;
			mappedBufferNode = nextMappedBufferNode;
		}
		else
		{
			mappedBufferNode = mappedBufferNode->next;
		}
	}

	return _MapMemory(mappedBufferNode->memory,
		mappedBufferNode->offset, byteCount);
}


void
LinearMemoryMapperFunc::Clear(DynamicLinearMemoryMapper& mapper)
{
	DynamicLinearMemoryMapper::BufferNode*	begin{ mapper.begin };

	while (begin != nullptr)
	{
		begin->offset = 0u;
		begin = begin->next;
	}

	mapper.current = mapper.begin;
}


void
LinearMemoryMapperFunc::Reset(DynamicLinearMemoryMapper& mapper)
{
	DynamicLinearMemoryMapper::BufferNode*	begin{ mapper.begin };
	DynamicLinearMemoryMapper::BufferNode*	next{ nullptr };

	while (begin != nullptr)
	{
		next = begin->next;
		mapper.allocatorProxy.Deallocate(
			mapper.allocatorProxy.allocatorData, begin);
		begin = next;
	}

	mapper.begin = nullptr;
	mapper.end = nullptr;
	mapper.current = nullptr;
}

static void
_Allocate(DynamicLinearMemoryMapper& mapper)
{
	LinearAllocatorProxy const& allocator{ mapper.allocatorProxy };

	DynamicLinearMemoryMapper::BufferNode* const	bufferNode{
	_AllocateBufferNode(allocator, mapper.blockSize) };

	mapper.begin = bufferNode;
	mapper.end = bufferNode;
	mapper.current = bufferNode;
}


Char*
_MapMemory(Char* memory, 
	Uint32& offset,
	Uint32 byteCount)
{
	Char*	ptr{ memory + offset };
	offset += byteCount;
	return ptr;
}


DynamicLinearMemoryMapper::BufferNode*
_AllocateBufferNode(LinearAllocatorProxy allocator,
	Uint32 blockSize)
{
	const Uint32	allocatedByteCount{
		sizeof(DynamicLinearMemoryMapper::BufferNode) + blockSize };

	Char*	bufferNodeMemory = static_cast<Char*>(allocator.Allocate(
		allocator.allocatorData, allocatedByteCount));

	Char*	bufferMemory{ bufferNodeMemory +
		sizeof(DynamicLinearMemoryMapper::BufferNode) };

	DynamicLinearMemoryMapper::BufferNode*	bufferNode{
		reinterpret_cast<DynamicLinearMemoryMapper::BufferNode*>(bufferNodeMemory) };

	if (bufferNode != nullptr)
	{
		*bufferNode = { 0u, bufferMemory, nullptr };
	}

	return bufferNode;
}

}
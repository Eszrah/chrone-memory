#include "LinearMemoryMapperFunc.h"

#include "LinearMemoryMapper.h"


namespace chrone::memory
{

DynamicLinearMemoryMapper::BufferNode*
_AllocateBufferNode(DynamicLinearMemoryMapper mapper, Uint32 blockSize);

Char* _MapMemory(Char* memory, Uint32& offset, Uint32 byteCount);

bool _HasEnoughMemory(Uint32 size, Uint32 offset, Uint32 byteCount);



void
LinearMemoryMapperFunc::Initialize(StaticLinearMemoryMapper& mapper,
	Uint32 blockSize)
{
	if (mapper.buffer.memory != nullptr || blockSize == 0u) { return; }
	mapper.buffer = Buffer{ blockSize, 0u,
			static_cast<Char*>(mapper.allocatorProxy.Allocate(
				mapper.allocatorProxy.allocatorData, blockSize)) };
}


Char*
LinearMemoryMapperFunc::MapMemory(StaticLinearMemoryMapper& mapper,
	Uint32 byteCount)
{
	if (_HasEnoughMemory(mapper.buffer.size, 
		mapper.buffer.offset, byteCount) == false) { return nullptr; }

	return _MapMemory(mapper.buffer.memory, mapper.buffer.offset, byteCount);
}


void
LinearMemoryMapperFunc::Clear(StaticLinearMemoryMapper& mapper)
{
	if (mapper.buffer.memory == nullptr) { return; }
	mapper.buffer.offset = 0;
}


void
LinearMemoryMapperFunc::Reset(StaticLinearMemoryMapper& mapper)
{
	if (mapper.buffer.memory == nullptr) { return; }
	mapper.allocatorProxy.Deallocate(
		mapper.allocatorProxy.allocatorData, mapper.buffer.memory);
	mapper.buffer = Buffer{};
}


void
LinearMemoryMapperFunc::Initialize(DynamicLinearMemoryMapper& mapper,
	Uint32 blockSize)
{
	if (mapper.begin != nullptr) { return; }

	const Uint32	allocatedByteCount{
	sizeof(DynamicLinearMemoryMapper::BufferNode) + blockSize };

	DynamicLinearMemoryMapper::BufferNode*	bufferNode{ 
		_AllocateBufferNode(mapper, blockSize) };

	mapper.blockSize = blockSize;
	mapper.begin = mapper.end = mapper.current = bufferNode;
	mapper.begin->next = nullptr;
}


Char*
LinearMemoryMapperFunc::MapMemory(DynamicLinearMemoryMapper& mapper,
	Uint32 byteCount)
{
	DynamicLinearMemoryMapper::BufferNode*	mappedBufferNode{ mapper.current };
	const Uint32	blockSize{ mapper.blockSize };

	//if (mappedBufferNode == nullptr || 
	//	_HasEnoughMemory(blockSize, mappedBufferNode->offset, byteCount)
	//	== false)
	//{
	//	mappedBufferNode = mappedBufferNode == nullptr ?
	//		_AllocateBufferNode(mapper, blockSize) : mappedBufferNode->next;

	//	if (mappedBufferNode == nullptr ||
	//		_HasEnoughMemory(blockSize, mappedBufferNode->offset, byteCount)
	//		== false)
	//	{

	//	}

	//	return _MapMemory(mappedBuffer, byteCount);
	//}

	return nullptr;
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


bool
_HasEnoughMemory(Uint32 size,
	Uint32 offset,
	Uint32 byteCount)
{
	return size && byteCount && (size - offset) >= byteCount;
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
_AllocateBufferNode(DynamicLinearMemoryMapper mapper,
	Uint32 blockSize)
{
	const Uint32	allocatedByteCount{
sizeof(DynamicLinearMemoryMapper::BufferNode) + blockSize };

	Char*	bufferNodeMemory = static_cast<Char*>(mapper.allocatorProxy.Allocate(
		mapper.allocatorProxy.allocatorData, allocatedByteCount));

	Char*	bufferMemory{ bufferNodeMemory +
		sizeof(DynamicLinearMemoryMapper::BufferNode) };

	DynamicLinearMemoryMapper::BufferNode*	bufferNode{
		reinterpret_cast<DynamicLinearMemoryMapper::BufferNode*>(bufferNodeMemory) };

	bufferNode->offset = 0;
	bufferNode->memory = bufferMemory;
	return bufferNode;
}

}
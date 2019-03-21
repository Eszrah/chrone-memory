#pragma once

#include "NativeType.h"

namespace chrone::memory
{

struct StaticLinearMemoryMapper;
struct DynamicLinearMemoryMapper;

struct LinearMemoryMapperFunc 
{

void
Initialize(StaticLinearMemoryMapper& mapper,
	Uint32 blockSize);


Char*
MapMemory(StaticLinearMemoryMapper& mapper,
	Uint32 byteCount);


void
Clear(StaticLinearMemoryMapper& mapper);


void
Reset(StaticLinearMemoryMapper& mapper);


void
Initialize(DynamicLinearMemoryMapper& mapper,
	Uint32 blockSize);


Char*
MapMemory(DynamicLinearMemoryMapper& mapper,
	Uint32 byteCount);


void
Clear(DynamicLinearMemoryMapper& mapper);


void
Reset(DynamicLinearMemoryMapper& mapper);


};

}
#pragma once

#include "NativeType.h"

namespace chrone::memory
{

struct StaticLinearMemoryMapper;
struct DynamicLinearMemoryMapper;
struct LinearAllocatorProxy;

struct LinearMemoryMapperFunc 
{

static void
Allocate(StaticLinearMemoryMapper& mapper);


static Char*
MapMemory(StaticLinearMemoryMapper& mapper,
	Uint32 const byteCount);


static void
Clear(StaticLinearMemoryMapper& mapper);


static void
Reset(StaticLinearMemoryMapper& mapper);


static void
Allocate(DynamicLinearMemoryMapper& mapper);


static Char*
MapMemory(DynamicLinearMemoryMapper& mapper,
	Uint32 const byteCount);


static void
Clear(DynamicLinearMemoryMapper& mapper);


static void
Reset(DynamicLinearMemoryMapper& mapper);


};

}
#pragma once

namespace CHRONE::MEMORY
{

template<class T>
struct CoreAllocatorHelper
{
	using ByteAmountType = size_t;

	inline static void*	Allocate(T& allocator, ByteAmountType byteCount);
	inline static void	Deallocate(T& allocator, void* memory);
};

template<class T>
inline void * CoreAllocatorHelper<T>::Allocate(T& allocator, ByteAmountType byteCount)
{
	return allocator.Allocate(byteCount);
}

template<class T>
inline void CoreAllocatorHelper<T>::Deallocate(T& allocator, void* memory)
{
	allocator.Deallocate(memory);
}

}
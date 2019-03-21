#pragma once

namespace chrone::memory
{

struct MemoryConstructor
{
	using InstanceCountType = size_t;

	template<class T, class... Args>
	inline static void	Construct(T* instance, Args&&... args);

	template<class T, class... Args>
	inline static void	ConstructRange(InstanceCountType count, T* instance, Args&&... args);

	template<class T>
	inline static void	Destruct(T* instance);

	template<class T>
	inline static void	DestructRange(InstanceCountType count, T* instance);
};


template<
	class T, 
	class ...Args>
inline void 
MemoryConstructor::Construct(
	T * instance, 
	Args && ...args)
{
	new(instance) T(std::forward<Args>(args)...);
}


template<
	class T, 
	class ...Args>
inline void 
MemoryConstructor::ConstructRange(
	InstanceCountType count, 
	T* instance, 
	Args && ...args)
{
	for (auto index{ 0u }; index < count; ++index)
	{
		Construct(instance + index, std::forward<Args>(args)...);
	}
}


template<class T>
inline void 
MemoryConstructor::Destruct(
	T* instance)
{
	(*instance).~T();
}


template<class T>
inline void MemoryConstructor::DestructRange(InstanceCountType count, T * instance)
{
	for (auto index{ 0u }; index < count; ++index)
	{
		Destruct(instance + index);
	}
}

}
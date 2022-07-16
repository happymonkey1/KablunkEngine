#pragma once
#ifndef KABLUNK_CORE_SINGLETON_H
#define KABLUNK_CORE_SINGLETON_H

#include "Kablunk/Core/KablunkAPI.h"

#include <typeindex>
#include <memory>
#include <mutex>

namespace Kablunk
{
	// typedef for singleton instance function
	using GetStaticInstanceFuncT = void*(*)();

	void get_shared_instance(const std::type_index & type_index, GetStaticInstanceFuncT get_static_instance, void** instance);

	// #TODO compile time check to make sure constructor is private
	template <typename T>
	class Singleton
	{
	public:
		Singleton() = delete;
		Singleton(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;

		inline static T* get() { return get_instance(); }
		inline static void init() { get_instance()->init(); }
		inline static void shutdown() { get_instance()->shutdown(); }
	private:
		static T* get_instance()
		{
			static void* instance = nullptr;
			if (!instance)
				get_shared_instance(typeid(T), &get_static_instance, &instance);

			return reinterpret_cast<T*>(instance);
		}

		static void* get_static_instance()
		{
			static T* instance = new T();
			return reinterpret_cast<void*>(instance);
		}
	};

	/*
	* Abstract class for classes that will be singletons.
	* Don't forget to use SINGLETON_CONSTRUCTOR(ClassName) and SINGLETON_FRIEND(ClassName)!
	*/
	class ISingleton
	{
	public:
		virtual ~ISingleton() = default;
		virtual void init() = 0;
		virtual void shutdown() = 0;
	};
	 
# define SINGLETON_CONSTRUCTOR(T) T::T() { }
# define SINGLETON_FRIEND(T) friend class Singleton<T>;

}

#endif

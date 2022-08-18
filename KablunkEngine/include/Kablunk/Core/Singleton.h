#pragma once
#ifndef KABLUNK_CORE_SINGLETON_H
#define KABLUNK_CORE_SINGLETON_H

#include "Kablunk/Core/KablunkAPI.h"

#ifndef KB_SINGLETON_INTERNAL_IMPL
#	ifndef KB_DISTRIBUTION
#		include <boost/interprocess/detail/intermodule_singleton.hpp>
#	else
#		include <boost/serialization/singleton.hpp>
#	endif
#else
#	include <typeinfo>
#	include <typeindex>
#	include <memory>
#	include <mutex>
#endif

namespace Kablunk
{
#ifndef KB_SINGLETON_INTERNAL_IMPL
#	ifndef KB_DISTRIBUTION
		template <typename T>
		using Singleton = boost::interprocess::ipcdetail::intermodule_singleton<T>;
#	else
		template <typename T>
		class Singleton
		{
		public:
			static T& get() { return boost::serialization::singleton<T>::get_instance(); }
		};
#	endif

#	define SINGLETON_GET_FUNC(T) static inline T& get() { return Singleton<T>::get(); }
#else
	// typedef for singleton instance function
	using GetStaticInstanceFuncT = void*(*)();

	KB_API void get_shared_instance(const std::type_index & type_index, GetStaticInstanceFuncT get_static_instance, void** instance);

	// #TODO compile time check to make sure constructor is private
	template <typename T>
	class Singleton
	{
	public:
		Singleton() = delete;
		Singleton(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;

		// return a pointer to the singleton of this class
		static T* get();
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
	class KB_API ISingleton
	{
	public:
		virtual ~ISingleton() = default;
		virtual void init() = 0;
		virtual void shutdown() = 0;
	};

	template <typename T>
	T* Singleton<T>::get()
	{
		static_assert(std::is_base_of<ISingleton, T>::value, "Class is not an ISingleton!"); 
		return get_instance();
	}
	 
# define SINGLETON_CONSTRUCTOR(T) T::T() { }
# define SINGLETON_FRIEND(T) friend class Singleton<T>;
#endif

}

#endif

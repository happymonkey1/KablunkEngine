#pragma once
#ifndef KABLUNK_CORE_SINGLETON_H
#define KABLUNK_CORE_SINGLETON_H

#include "Kablunk/Core/KablunkAPI.h"

// #NOTE if KB_SINGLETON_INTERNAL_IMPL defined, the singleton implementation does not support crossing dll boundaries
//       this means that runtime dll loading will fail
#define KB_SINGLETON_INTERNAL_IMPL

#ifndef KB_SINGLETON_INTERNAL_IMPL
#	ifndef KB_DISTRIBUTION
#		pragma warning(push, 0)
#		include <boost/interprocess/detail/intermodule_singleton.hpp>
#		pragma warning(pop)
#	else
#		pragma warning(push, 0)
#		include <boost/serialization/singleton.hpp>
#		pragma warning(pop)
#	endif
#else
#	include <typeinfo>
#	include <typeindex>
#	include <memory>
#	include <mutex>
#endif

namespace kb
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

#else

	// #TODO compile time check to make sure constructor is private
	template <typename T>
	class Singleton
	{
	public:
		Singleton() = delete;
		Singleton(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;

		// return a pointer to the singleton of this class
        inline static auto get() -> T&
        {
            static T* instance = nullptr;
            if (!instance)
                instance = new T{};

            return *instance;
        }
	};
	 
#   define SINGLETON_CONSTRUCTOR(T) T::T() { }
#   define SINGLETON_FRIEND(T) friend class Singleton<T>;
#	define SINGLETON_GET_FUNC(T) static inline T& get() { return Singleton<T>::get(); }
#endif

}

#endif

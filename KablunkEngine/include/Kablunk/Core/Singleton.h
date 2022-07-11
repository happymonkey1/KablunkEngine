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

		inline static void destroy()
		{
			get_instance()->shutdown();
		}
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

	class KB_API ISingleton
	{
	public:
		virtual ~ISingleton() { shutdown(); }
	protected:
		virtual void init() {};
		virtual void shutdown() {};
	};
	 
# define SINGLETON_CONSTRUCTOR(T) T::T() { init(); }

}

#endif

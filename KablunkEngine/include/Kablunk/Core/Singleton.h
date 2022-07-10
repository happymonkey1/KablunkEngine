#pragma once
#ifndef KABLUNK_CORE_SINGLETON_H
#define KABLUNK_CORE_SINGLETON_H

#include "Kablunk/Core/KablunkAPI.h"

#include <typeindex>
#include <memory>
#include <mutex>

namespace Kablunk
{

	namespace Internal
	{
		struct SingletonManager
		{
			void* obj = nullptr;
			// #TODO should this be Kablunk::IntrusiveRef?
			std::shared_ptr<std::mutex> mutex = std::make_shared<std::mutex>();
		};

		static std::mutex& get_singleton_mutex();
		SingletonManager* get_singleton_manager(const std::type_index& type_index);
	}

	using GetStaticInstanceFuncT = void*(*)();

	template <typename... Args>
	void get_shared_instance(const std::type_index& type_index, GetStaticInstanceFuncT get_static_instance, void** instance, Args&&... args)
	{
		Internal::SingletonManager* manager = nullptr;

		// grab manager that holds mutex and the instance
		{
			std::lock_guard<std::mutex> lock(Internal::get_singleton_mutex());
			// exit if instance is already initialized
			if (*instance)
				return;

			manager = Internal::get_singleton_manager(type_index);
		}

		// create instance
		{
			std::lock_guard<std::mutex> lock(*manager->mutex);
			if (!manager->obj)
				manager->obj = get_static_instance(std::forward<Args>(args)...);
		}

		// save instance
		{
			std::lock_guard<std::mutex> lock(Internal::get_singleton_mutex());
			*instance = manager->obj;
		}
	}

	template <typename T>
	class Singleton
	{
	public:
		template <typename... Args>
		inline static T* get()
		{
			return get_instance(std::forward<Args>(args)...);
		}
	private:
		template <typename... Args>
		static T* get_instance(Args&&... args)
		{
			static void* instance = nullptr;
			if (!instance)
				get_shared_instance(typeid(T), &get_static_instance, &instance, std::forward<Args>(args)...);

			return reinterpret_cast<T*>(instance);
		}

		template <typename... Args>
		static void* get_static_instance(Args&&... args)
		{
			static T* instance = new T(std::forward<Args>(args)...);
			return reinterpret_cast<void*>(instance);
		}
	};

}

#endif

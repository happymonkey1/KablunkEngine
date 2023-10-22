#include <kablunkpch.h>

#include "Kablunk/Core/Singleton.h"

#include <typeinfo>
#include <typeindex>
#include <unordered_map>

namespace kb
{
#ifndef KB_SINGLETON_INTERNAL_IMPL

#else
	namespace Internal
	{
		struct SingletonManager
		{
			void* obj = nullptr;
			// #TODO should this be kb::IntrusiveRef?
			std::shared_ptr<std::mutex> mutex = std::make_shared<std::mutex>();
		};

		static std::mutex& get_singleton_mutex()
		{
			static std::mutex s_singleton_mutex;
			return s_singleton_mutex;
		}

		static SingletonManager* get_singleton_manager(const std::type_index& type_index)
		{
			static std::unordered_map<std::type_index, SingletonManager> s_manager_map;

			auto it = s_manager_map.find(type_index);
			if (it != s_manager_map.end())
				return &it->second;

			it = s_manager_map.emplace(type_index, SingletonManager{}).first;
			return &it->second;
		}
	}

	KB_API void get_shared_instance(const std::type_index& type_index, GetStaticInstanceFuncT get_static_instance, void** instance)
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
				manager->obj = get_static_instance();
		}

		// save instance
		{
			std::lock_guard<std::mutex> lock(Internal::get_singleton_mutex());
			*instance = manager->obj;
		}
	}
#endif
}

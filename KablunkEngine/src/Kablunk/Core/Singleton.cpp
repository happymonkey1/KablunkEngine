#include <kablunkpch.h>

#include "Kablunk/Core/Singleton.h"

#include <typeinfo>
#include <typeindex>
#include <unordered_map>

namespace Kablunk::Internal
{
		std::mutex& get_singleton_mutex()
		{
			static std::mutex s_singleton_mutex;
			return s_singleton_mutex;
		}

		SingletonManager* get_singleton_manager(const std::type_index& type_index)
		{
			static std::unordered_map<std::type_index, SingletonManager> s_manager_map;

			auto it = s_manager_map.find(type_index);
			if (it != s_manager_map.end())
				return &it->second;

			it = s_manager_map.emplace(type_index, SingletonManager{}).first;
			return &it->second;
		}
}

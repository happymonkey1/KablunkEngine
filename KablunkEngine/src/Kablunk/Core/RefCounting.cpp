#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/RefCounting.h"

#include <mutex>
#include <unordered_set>

namespace kb
{
	static std::unordered_set<void*> s_live_references;
	static std::mutex s_live_references_mutex;

	namespace Internal
	{
		void AddToLiveReferences(void* instance)
		{
			std::scoped_lock<std::mutex> lock(s_live_references_mutex);
			KB_CORE_ASSERT(instance, "trying to add nullptr to live reference map!");
			s_live_references.insert(instance);
		}

		void RemoveFromLiveReferences(void* instance)
		{
			std::scoped_lock<std::mutex> lock(s_live_references_mutex);
			KB_CORE_ASSERT(instance, "trying to remove nullptr to live reference map!");
			KB_CORE_ASSERT(s_live_references.find(instance) != s_live_references.end(), "reference not found in map!");
			s_live_references.erase(instance);
		}

		bool IsLive(void* instance)
		{
			KB_CORE_ASSERT(instance, "reference is null");
			return s_live_references.find(instance) != s_live_references.end();
		}
	}
	

}

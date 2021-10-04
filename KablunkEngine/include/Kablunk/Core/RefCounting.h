#ifndef KABLUNK_CORE_REF_COUNTING_H
#define KABLUNK_CORE_REF_COUNTING_H

#include <memory>

namespace Kablunk
{
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template <typename T>
	class WeakRef
	{
	public:
		WeakRef(const Ref<T>& ref)
			: m_ptr{ ref.get() } {}

		WeakRef(T* ptr)
			: m_ptr{ ptr } {}

		// #TODO make sure pointer is actually valid since this is a weak ref
	
		T* operator->() { return m_ptr; }
		T& operator*() { return *m_ptr; }

		T* get() { return m_ptr; }
	private:
		T* m_ptr;
	};
}

#endif

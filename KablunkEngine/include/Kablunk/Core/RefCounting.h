#ifndef KABLUNK_CORE_REF_COUNTING_H
#define KABLUNK_CORE_REF_COUNTING_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/CoreTypes.h"

#include <atomic>
#include <memory>
#include <cassert>
#include <iostream>
#include <mutex>
#include <unordered_set>
#include <type_traits>

namespace kb
{

// #TODO 1. refactor into CRTP interface (will cause a MAJOR refactor throughout the whole engine) for better performance
//       less indirection, less cache misses,
//       2. disable weak ptrs by default since they introduce overhead of maintaining global pointer cache
//       3. compile time option / multiple types for single vs multi-threaded to remove un-necessary atomic operations
//       example / reference implementation for refactor https://github.com/gershnik/intrusive_shared_ptr/

class RefCounted
{
public:
	// #NOTE guarantee that RefCounted has a vtable, so that memory does not become misaligned (by 8 bytes) when downcasting
	virtual ~RefCounted() = default;

	KB_FORCE_INLINE auto inc_ref() const -> void { m_ref_count.fetch_add(1, std::memory_order_relaxed); }
	KB_FORCE_INLINE auto dec_ref() const -> u32 { return m_ref_count.fetch_sub(1, std::memory_order_release); }
private:
	mutable std::atomic<u32> m_ref_count = 0;
};

namespace Internal
{ // start namespace ::Internal
	void AddToLiveReferences(void* instance);
	void RemoveFromLiveReferences(void* instance);
	bool IsLive(void* instance);
} // end namespace ::Internal

namespace concepts
{ // start namespace ::concepts

template <typename T>
concept is_ref_counted = std::is_base_of<RefCounted, T>::value || std::is_same<T, std::nullptr_t>::value;

} // end namespace ::concepts

template <typename T>
class ref
{
public:
	constexpr ref() : m_ptr{ nullptr } {}
	constexpr ref(std::nullptr_t n) : m_ptr { nullptr } {}
    constexpr ref(T* ptr) : m_ptr{ ptr }
	{
		static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

		IncRef();
	}

	template <typename T2>
    constexpr ref(const ref<T2>& other)
	{
		m_ptr = static_cast<T*>(other.m_ptr);

		IncRef();
	}

	template <typename T2>
    constexpr ref(ref<T2>&& other)
	{
		m_ptr = static_cast<T*>(other.m_ptr);
		other.m_ptr = nullptr;
	}

    constexpr static ref<T> CopyWithoutIncrement(const ref<T>& other)
	{
		ref<T> new_ref = nullptr;
		new_ref->m_ptr = other->m_ptr;

		return new_ref;
	}

    constexpr ~ref()
	{
		DecRef();
	}

    constexpr ref(const ref<T>& other) : m_ptr{ other.m_ptr }
	{
		IncRef();
	}

    constexpr ref& operator=(std::nullptr_t)
	{
		DecRef();
		m_ptr = nullptr;
		return *this;
	}

    constexpr ref& operator=(const ref<T>& other)
	{
		other.IncRef();
		DecRef();

		m_ptr = other.m_ptr;
		return *this;
	}

	template <typename T2>
    constexpr ref& operator=(const ref<T2>& other)
	{
		other.IncRef();
		DecRef();

		m_ptr = static_cast<T*>(other.m_ptr);
		return *this;
	}

	template <typename T2>
    constexpr ref& operator=(ref<T2>&& other)
	{
		DecRef();

		m_ptr = static_cast<T*>(other.m_ptr);
		other.m_ptr = nullptr;
		return *this;
	}

	constexpr operator bool() { return m_ptr != nullptr; }
	constexpr operator bool() const { return m_ptr != nullptr; }

	T* operator->() { return m_ptr; }
    constexpr const T* operator->() const { return m_ptr; }

	T& operator*() { return *m_ptr; }
    constexpr const T& operator*() const { return *m_ptr; }

	T* get() { return m_ptr; }
    constexpr const T* get() const { return m_ptr; }

    constexpr void reset(T* ptr = nullptr)
	{
		DecRef();
		m_ptr = ptr;
	}

	template <typename T2>
    constexpr ref<T2> As() const
	{
		return ref<T2>(*this);
	}

	template <typename... Args>
    constexpr static ref<T> Create(Args&&... args)
	{
		return ref<T>(new T(std::forward<Args>(args)...));
	}

	// ptr comparison, not value
    constexpr bool operator==(const ref<T> other) const
	{
		return m_ptr == other.m_ptr;
	}

    constexpr bool operator!=(const ref<T> other) const
	{
		return !(*this == other);
	}

    constexpr bool Equals(const ref<T>& other) const
	{
		if (!m_ptr || !other.m_ptr)
			return false;

		return *m_ptr == *other.m_ptr;
	}

private:
    constexpr KB_FORCE_INLINE auto IncRef() const -> void
	{
		if (m_ptr)
		{
			m_ptr->inc_ref();
			Internal::AddToLiveReferences((void*)m_ptr);
		}
	}

    constexpr KB_FORCE_INLINE auto DecRef() const -> void
	{
		if (!m_ptr)
			return;
			
			
		if (!m_ptr->dec_ref())
		{
            std::atomic_thread_fence(std::memory_order_acquire);
			delete m_ptr;
			Internal::RemoveFromLiveReferences((void*)m_ptr);
		}

        m_ptr = nullptr;
	}

	template <typename T2>
	friend class ref;

	template <typename T2>
	friend class WeakRef;

	mutable T* m_ptr;
};

template <typename T>
class WeakRef
{
public:
	WeakRef() = default;
	//WeakRef(IntrusiveRef<T> ref) : m_ptr{ ref.get() } { }
	WeakRef(const ref<T>& ref) : m_ptr{ ref.m_ptr } { }
	WeakRef(ref<T>& ref) : m_ptr{ ref.m_ptr } { }
	WeakRef(T* ptr) : m_ptr{ ptr } { }

	~WeakRef() = default;

	// #TODO make sure pointer is valid in the live reference map
	bool Valid() const { return m_ptr ? Internal::IsLive(m_ptr) : false; }

	T* operator->() { return m_ptr; }
	T& operator*() { return *m_ptr; }
	operator bool() const { return Valid(); }

    // get the raw pointer
	T* get() { return m_ptr; }
private:
	T* m_ptr = nullptr;
};

// ==============
//   type alias
// ==============

// view for a intrusive ref counted pointer
template <typename T>
using weak_ref = WeakRef<T>;

// ==============
}

#endif

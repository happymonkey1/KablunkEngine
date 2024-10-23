#ifndef KABLUNK_CORE_REF_COUNTING_H
#define KABLUNK_CORE_REF_COUNTING_H

#include "Kablunk/Core/KablunkAPI.h"
#include "Kablunk/Core/CoreTypes.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>

#define KB_LIVE_REFERENCES 0
#define KB_REF_MOVE_DEFINED 1


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
    RefCounted() = default;
	// #NOTE guarantee that RefCounted has a vtable, so that memory does not become misaligned (by 8 bytes) when downcasting
	virtual ~RefCounted() = default;

	KB_FORCE_INLINE auto inc_ref() const -> void { m_ref_count.fetch_add(1, std::memory_order_relaxed); }
	KB_FORCE_INLINE auto dec_ref() const -> u32 { return m_ref_count.fetch_sub(1, std::memory_order_release) - 1; }
private:
    mutable std::atomic<u32> m_ref_count{ 0 };
};

namespace Internal
{ // start namespace ::Internal
    inline static std::mutex s_ref_move_construct_mutex;


	void AddToLiveReferences(void* instance);
	void RemoveFromLiveReferences(void* instance);
	bool IsLive(void* instance);
} // end namespace ::Internal

namespace concepts
{ // start namespace ::concepts

template <typename T>
concept is_ref_counted = std::is_base_of_v<RefCounted, T>;

} // end namespace ::concepts

template <typename T>
class arc
{
public:
	constexpr arc() : m_ptr{ nullptr } {}
    explicit constexpr arc(std::nullptr_t) : m_ptr { nullptr } {}

    explicit constexpr arc(T* ptr) : m_ptr{ ptr }
	{
		static_assert(std::is_base_of_v<RefCounted, T>, "Class is not RefCounted!");

		IncRef();
	}

    constexpr arc(const arc<T>& other) noexcept
        : m_ptr{ other.m_ptr }
    {
        if (this != &other)
            IncRef();
    }

#if KB_REF_MOVE_DEFINED
    constexpr arc(arc&& p_other) noexcept
        : m_ptr{ p_other.m_ptr }
	{
        static_assert(std::is_base_of_v<RefCounted, T>, "Class is not RefCounted!");

        p_other.m_ptr = nullptr;
	}
#endif

	template <typename T2>
    explicit constexpr arc(const arc<T2>& other) noexcept
	{
		m_ptr = static_cast<T*>(other.m_ptr);

		IncRef();
	}

	template <typename T2>
    explicit constexpr arc(arc<T2>&& other) noexcept
	{
		m_ptr = static_cast<T*>(other.m_ptr);
		other.m_ptr = nullptr;
	}

    constexpr static arc CopyWithoutIncrement(const arc& other) noexcept
	{
		arc new_ref = nullptr;
		new_ref->m_ptr = other->m_ptr;

		return new_ref;
	}

    constexpr ~arc() noexcept
	{
		DecRef();
	}


    constexpr arc& operator=(std::nullptr_t) noexcept
	{
		DecRef();
		m_ptr = nullptr;
		return *this;
	}

    constexpr arc& operator=(const arc& other) noexcept
	{
        if (this == &other)
            return *this;

		other.IncRef();
		DecRef();

		m_ptr = other.m_ptr;
		return *this;
	}

#if KB_REF_MOVE_DEFINED
    constexpr arc& operator=(arc&& p_other) noexcept
	{
        // DecRef();

        m_ptr = p_other.m_ptr;
        p_other.m_ptr = nullptr;

        return *this;
	}
#endif

#if 0
	template <typename T2>
    constexpr arc& operator=(const arc<T2>& other) noexcept
	{
		other.IncRef();
		DecRef();

		m_ptr = static_cast<T*>(other.m_ptr);
		return *this;
	}

	template <typename T2>
    constexpr arc& operator=(arc<T2>&& p_other) noexcept
	{
		DecRef();

		m_ptr = static_cast<T*>(p_other.m_ptr);
        p_other.m_ptr = nullptr;

		return *this;
	}
#endif

	constexpr operator bool() noexcept { return m_ptr != nullptr; }
	constexpr operator bool() const noexcept { return m_ptr != nullptr; }

	T* operator->() noexcept { return m_ptr; }
    constexpr const T* operator->() const noexcept { return m_ptr; }

	T& operator*() noexcept { return *m_ptr; }
    constexpr const T& operator*() const noexcept { return *m_ptr; }

	T* get() noexcept { return m_ptr; }
    constexpr const T* get() const noexcept { return m_ptr; }

    constexpr void reset(T* ptr = nullptr) noexcept
	{
		DecRef();
		m_ptr = ptr;
	}

	template <typename T2>
    constexpr arc<T2> As() const noexcept
	{
		return arc<T2>(*this);
	}

	template <typename... Args>
    constexpr static arc Create(Args&&... args) noexcept
	{
        return arc{ new T(std::forward<Args>(args)...) };
	}

	// ptr comparison, not value
    constexpr bool operator==(const arc other) const noexcept
	{
		return m_ptr == other.m_ptr;
	}

    constexpr bool operator!=(const arc other) const noexcept
	{
		return !(*this == other);
	}

    constexpr bool Equals(const arc& other) const noexcept
	{
		if (!m_ptr || !other.m_ptr)
			return false;

		return *m_ptr == *other.m_ptr;
	}

private:
    constexpr KB_FORCE_INLINE auto IncRef() const noexcept -> void
	{
        if (!m_ptr)
            return;

        m_ptr->inc_ref();
#if KB_LIVE_REFERENCES
        Internal::AddToLiveReferences((void*)m_ptr);
#endif
	}

    constexpr KB_FORCE_INLINE auto DecRef() const noexcept -> void
	{
		if (!m_ptr)
			return;

		if (!m_ptr->dec_ref())
		{
            std::atomic_thread_fence(std::memory_order_acquire);
			delete m_ptr;
#if KB_LIVE_REFERENCES
			Internal::RemoveFromLiveReferences((void*)m_ptr);
#endif
		}

        m_ptr = nullptr;
	}

	template <typename T2>
	friend class arc;

	template <typename T2>
	friend class WeakRef;

	mutable T* m_ptr;
};

template <typename T>
class WeakRef
{
public:
	WeakRef() = default;
	//WeakRef(IntrusiveRef<T> arc) : m_ptr{ arc.get() } { }
	WeakRef(const arc<T>& ref) : m_ptr{ ref.m_ptr } { }
	WeakRef(arc<T>& ref) : m_ptr{ ref.m_ptr } { }
	WeakRef(T* ptr) : m_ptr{ ptr } { }

	~WeakRef() = default;

#if KB_LIVE_REFERENCES
	bool Valid() const { return m_ptr ? Internal::IsLive(m_ptr) : false; }
#else
    auto Valid() const -> bool { return m_ptr; }
#endif

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

// view for a intrusive arc counted pointer
template <typename T>
using weak_ref = WeakRef<T>;

// ==============
}

#endif

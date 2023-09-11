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

class RefCounted
{
public:
	// #NOTE guarantee that RefCounted has a vtable, so that memory does not become misaligned (by 8 bytes) when downcasting
	virtual ~RefCounted() = default;

	void IncRefCount() const { m_ref_count++; }
	void DecRefCount() const { m_ref_count--; }
	uint32_t GetRefCount() const { return m_ref_count.load(); }
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
	ref() : m_ptr{ nullptr } {}
	ref(std::nullptr_t n) : m_ptr { nullptr } {}
	ref(T* ptr) : m_ptr{ ptr }
	{
		static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

		IncRef();
	}

	template <typename T2>
	ref(const ref<T2>& other)
	{
		m_ptr = (T*)other.m_ptr;

		IncRef();
	}

	template <typename T2>
	ref(ref<T2>&& other)
	{
		m_ptr = (T*)other.m_ptr;
		other.m_ptr = nullptr;
	}

	static ref<T> CopyWithoutIncrement(const ref<T>& other)
	{
		ref<T> new_ref = nullptr;
		new_ref->m_ptr = other->m_ptr;

		return new_ref;
	}

	~ref()
	{
		DecRef();
	}

	ref(const ref<T>& other) : m_ptr{ other.m_ptr }
	{
		IncRef();
	}

	ref& operator=(std::nullptr_t)
	{
		DecRef();
		m_ptr = nullptr;
		return *this;
	}

	ref& operator=(const ref<T>& other)
	{
		other.IncRef();
		DecRef();

		m_ptr = other.m_ptr;
		return *this;
	}

	template <typename T2>
	ref& operator=(const ref<T2>& other)
	{
		other.IncRef();
		DecRef();

		m_ptr = static_cast<T*>(other.m_ptr);
		return *this;
	}

	template <typename T2>
	ref& operator=(ref<T2>&& other)
	{
		DecRef();

		m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
		return *this;
	}

	operator bool() { return m_ptr != nullptr; }
	operator bool() const { return m_ptr != nullptr; }

	T* operator->() { return m_ptr; }
	const T* operator->() const { return m_ptr; }

	T& operator*() { return *m_ptr; }
	const T& operator*() const { return *m_ptr; }

	T* get() { return m_ptr; }
	const T* get() const { return m_ptr; }

	void reset(T* ptr = nullptr)
	{
		DecRef();
		m_ptr = ptr;
	}

	template <typename T2>
	ref<T2> As() const
	{
		return ref<T2>(*this);
	}

	template <typename... Args>
	static ref<T> Create(Args&&... args)
	{
		return ref<T>(new T(std::forward<Args>(args)...));
	}

	// ptr comparison, not value
	bool operator==(const ref<T> other) const
	{
		return m_ptr == other.m_ptr;
	}

	bool operator!=(const ref<T> other) const
	{
		return !(*this == other);
	}

	bool Equals(const ref<T>& other) const
	{
		if (!m_ptr || !other.m_ptr)
			return false;

		return *m_ptr == *other.m_ptr;
	}

private:
	void IncRef() const
	{
		if (m_ptr)
		{
			m_ptr->IncRefCount();
			Internal::AddToLiveReferences((void*)m_ptr);
		}
	}

	void DecRef() const
	{
		if (!m_ptr)
			return;
			
		m_ptr->DecRefCount();
			
		if (!m_ptr->GetRefCount())
		{
			delete m_ptr;
			Internal::RemoveFromLiveReferences((void*)m_ptr);
			m_ptr = nullptr;
		}
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

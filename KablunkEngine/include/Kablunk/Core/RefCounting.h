#ifndef KABLUNK_CORE_REF_COUNTING_H
#define KABLUNK_CORE_REF_COUNTING_H

#include <atomic>
#include <memory>
#include <cassert>
#include <iostream>

namespace Kablunk
{

	// #TODO deprecate
	
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// ================

	class RefCounted
	{
	public:
		void IncRefCount() const { m_ref_count++; }
		void DecRefCount() const { m_ref_count--; }
		uint32_t GetRefCount() const { return m_ref_count.load(); }
	private:
		mutable std::atomic<uint32_t> m_ref_count = 0;
	};

	namespace Internal
	{
		void AddToLiveReferences(void* instance);
		void RemoveFromLiveReferences(void* instance);
		bool IsLive(void* instance);
	}

	template <typename T>
	class IntrusiveRef
	{
	public:
		IntrusiveRef() : m_ptr{ nullptr } {}
		IntrusiveRef(std::nullptr_t n) : m_ptr { nullptr } {}
		IntrusiveRef(T* ptr) : m_ptr{ ptr }
		{
			static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

			IncRef();
		}

		template <typename T2>
		IntrusiveRef(const IntrusiveRef<T2>& other)
		{
			m_ptr = (T*)other.m_ptr;

			IncRef();
		}

		template <typename T2>
		IntrusiveRef(IntrusiveRef<T2>&& other)
		{
			m_ptr = (T*)other.m_ptr;
			other.m_ptr = nullptr;
		}

		static IntrusiveRef<T> CopyWithoutIncrement(const IntrusiveRef<T>& other)
		{
			IntrusiveRef<T> new_ref = nullptr;
			new_ref->m_ptr = other->m_ptr;

			return new_ref;
		}

		~IntrusiveRef()
		{
			DecRef();
		}

		IntrusiveRef(const IntrusiveRef<T>& other) : m_ptr{ other.m_ptr }
		{
			IncRef();
		}

		IntrusiveRef& operator=(std::nullptr_t)
		{
			DecRef();
			m_ptr = nullptr;
			return *this;
		}

		IntrusiveRef& operator=(const IntrusiveRef<T>& other)
		{
			other.IncRef();
			DecRef();

			m_ptr = other.m_ptr;
			return *this;
		}

		template <typename T2>
		IntrusiveRef& operator=(const IntrusiveRef<T2>& other)
		{
			other.IncRef();
			DecRef();

			m_ptr = other.m_ptr;
			return *this;
		}

		template <typename T2>
		IntrusiveRef& operator=(IntrusiveRef<T2>&& other)
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
		IntrusiveRef<T2> As() const
		{
			return IntrusiveRef<T2>(*this);
		}

		template <typename... Args>
		static IntrusiveRef<T> Create(Args&&... args)
		{
			return IntrusiveRef<T>(new T(std::forward<Args>(args)...));
		}

		// ptr comparison, not value
		bool operator==(const IntrusiveRef<T> other) const
		{
			return m_ptr == other.m_ptr;
		}

		bool operator!=(const IntrusiveRef<T> other) const
		{
			return !(*this == other);
		}

		bool Equals(const IntrusiveRef<T>& other) const
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
			if (m_ptr)
			{
				m_ptr->DecRefCount();
				if (m_ptr->GetRefCount() == 0)
				{
					delete m_ptr;
					Internal::RemoveFromLiveReferences((void*)m_ptr);
					m_ptr = nullptr;
				}
			}
		}

		template <class T2>
		friend class IntrusiveRef;

		mutable T* m_ptr;
	};

	template <typename T>
	class WeakRef
	{
	public:
		WeakRef() = default;
		//WeakRef(IntrusiveRef<T> ref) : m_ptr{ ref.get() } { }
		WeakRef(const IntrusiveRef<T>& ref) : m_ptr{ ref.get() } { }
		WeakRef(IntrusiveRef<T>& ref) : m_ptr{ ref.get() } { }
		WeakRef(T* ptr) : m_ptr{ ptr } { }

		~WeakRef() = default;

		// #TODO make sure pointer is valid in the live reference map
		bool Valid() const { return m_ptr ? Internal::IsLive(m_ptr) : false; }

		T* operator->() { return m_ptr; }
		T& operator*() { return *m_ptr; }
		operator bool() const { return m_ptr != nullptr; }

		T* get() { return m_ptr; }
	private:
		T* m_ptr = nullptr;
	};
}

#endif

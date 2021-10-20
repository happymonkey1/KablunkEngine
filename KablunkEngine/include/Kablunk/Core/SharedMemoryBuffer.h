#ifndef KABLUNK_CORE_SHARED_MEMORY_BUFFER_H
#define KABLUNK_CORE_SHARED_MEMORY_BUFFER_H

#include <Kablunk/Core/Core.h>

#include <string>
#include <vector>
namespace Kablunk
{
	// #TODO move
	struct SharedMemoryBufferLayoutElement
	{
		std::string Name = "";
		size_t Offset = 0;
		size_t Size = 0;

		SharedMemoryBufferLayoutElement() = default;
		SharedMemoryBufferLayoutElement(const std::string& name, size_t size) : Name{ name }, Offset{ 0 }, Size{ size } {}
	};

	class SharedMemoryBufferLayout
	{
	public:
		SharedMemoryBufferLayout(const std::initializer_list<SharedMemoryBufferLayoutElement>& elements);
		~SharedMemoryBufferLayout() = default;

		const std::vector<SharedMemoryBufferLayoutElement>& GetLayout() const { return m_elements; }
		const SharedMemoryBufferLayoutElement& Find(const std::string& name) const;
		size_t GetSize() const;

	private:
		void CalculateOffsets();

		std::vector<SharedMemoryBufferLayoutElement> m_elements;
	};


	// #TODO move
	class SharedMemoryBuffer
	{
	public:
		explicit SharedMemoryBuffer(const std::string& name, bool create,
			const std::initializer_list<SharedMemoryBufferLayoutElement>& elements);
		~SharedMemoryBuffer();

		template <typename T>
		T* Get(const std::string& name)
		{
			size_t offset = m_buffer_layout.Find(name).Offset;
			void* cur = static_cast<uint8_t*>(m_buffer) + offset;

			return (T*)cur;
		}

		template <typename T>
		T* Set(const std::string& name, T* data)
		{
			auto layout_element = m_buffer_layout.Find(name);
			void* cur = static_cast<uint8_t*>(m_buffer) + layout_element.Offset;

			memcpy(cur, data, layout_element.Size);

			return (T*)cur;
		}

		template <typename T, typename... Args>
		T* Create(const std::string& name, Args&&... args)
		{
			auto layout_element = m_buffer_layout.Find(name);
			void* cur = static_cast<uint8_t*>(m_buffer) + layout_element.Offset;
			T* casted_cur = (T*)cur;

			T* data = new (casted_cur) T{ std::forward<Args>(args)... };

			return data;
		}

	private:
		std::string m_name = "";
		void* m_handle = nullptr;
		void* m_buffer = nullptr;
		size_t m_buffer_size = 0;

		size_t m_offset = 0;
		SharedMemoryBufferLayout m_buffer_layout;
	};
}

#endif

#include "kablunkpch.h"

#include "Kablunk/Core/SharedMemoryBuffer.h"

#ifdef KB_PLATFORM_WINDOWS
#	include <windows.h>
#else
#	error "Shared Memory Buffers are only supported on Windows!"
#endif

namespace kb
{

	SharedMemoryBufferLayout::SharedMemoryBufferLayout(const std::initializer_list<SharedMemoryBufferLayoutElement>& elements)
		: m_elements{ elements }
	{
		CalculateOffsets();
	}

	const SharedMemoryBufferLayoutElement& SharedMemoryBufferLayout::Find(const std::string& name) const
	{
		auto it = std::find_if(m_elements.begin(), m_elements.end(), [&name](const auto& el) { return el.Name == name; });
		if (it == m_elements.end())
		{
			assert(false);
			//KB_CORE_ASSERT(false, "Trying to retrieve memory that does not exist in buffer!");
			return {};
		}

		return *it;
	}

	void SharedMemoryBufferLayout::CalculateOffsets()
	{
		size_t offset = 0;
		for (auto& el : m_elements)
		{
			el.Offset = offset;
			offset += el.Size;
		}
	}

	size_t SharedMemoryBufferLayout::GetSize() const
	{
		size_t size = 0;
		for (const auto& el : m_elements)
			size += el.Size;

		return size;
	}

	SharedMemoryBuffer::SharedMemoryBuffer(const std::string& name, bool create,
		const std::initializer_list<SharedMemoryBufferLayoutElement>& elements)
		: m_name{ name }, m_buffer_size{ 0 }, m_buffer_layout{ elements }
	{
		if (create)
		{
			m_buffer_size = m_buffer_layout.GetSize();

			m_handle = CreateFileMappingA(
				INVALID_HANDLE_VALUE,
				NULL,
				PAGE_READWRITE,
				0,
				m_buffer_size,
				m_name.c_str()
			);

			//KB_CORE_TRACE("Shared Memory '{0}' and handle '{1}' created!", m_name, m_handle);
		}
		else
		{
			m_handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
			//KB_CORE_TRACE("Shared Memory '{0}' and handle '{1}' opened!", m_name, m_handle);
		}

		if (m_handle == nullptr)
		{
			//KB_CORE_ERROR("Failed to map shared memory!");
			return;
		}

		m_buffer = MapViewOfFile(m_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	}

	SharedMemoryBuffer::~SharedMemoryBuffer()
	{
		KB_CORE_TRACE("Shared Memory '{0}' destroyed!", m_name);

		UnmapViewOfFile(m_buffer);

		CloseHandle(m_handle);

		m_buffer = nullptr;
		m_buffer_size = 0;
	}
}

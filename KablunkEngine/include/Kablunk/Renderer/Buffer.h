#ifndef KABLUNK_RENDERER_BUFFER_H
#define KABLUNK_RENDERER_BUFFER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/RendererTypes.h"

namespace Kablunk 
{

	enum class VertexBufferUsage
	{
		Static = 0, Dynamic
	};

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 2 * 4;
		case ShaderDataType::Float3:   return 3 * 4;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 3*3*3;
		case ShaderDataType::Mat4:     return 4*4*4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 2 * 4;
		case ShaderDataType::Int3:     return 3 * 4;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		default:     KB_CORE_ERROR("Unknown ShaderDataType!"); return 0;
		}
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		size_t Offset;
		bool Normalized;
		
		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name{ name }, Type{ type }, Size{ ShaderDataTypeSize(type) }, Offset{ 0 }, Normalized{ normalized }
		{

		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float:    return 1;
			case ShaderDataType::Float2:   return 2;
			case ShaderDataType::Float3:   return 3;
			case ShaderDataType::Float4:   return 4;
			case ShaderDataType::Mat3:     return 3 * 3;
			case ShaderDataType::Mat4:     return 4 * 4;
			case ShaderDataType::Int:      return 1;
			case ShaderDataType::Int2:     return 2;
			case ShaderDataType::Int3:     return 3;
			case ShaderDataType::Int4:     return 4;
			case ShaderDataType::Bool:     return 1;
			default:     KB_CORE_ERROR("Unknown ShaderDataType!"); return 0;
			}
		}
	};

	class BufferLayout
	{
	public:

		BufferLayout() { KB_CORE_WARN("Default BufferLayout constructor not implemented!"); }
		BufferLayout(const std::initializer_list<BufferElement>& elements) 
			: m_Elements{ elements }
		{
			CalculateOffsetsAndStride();
		}

		

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer : public RefCounted
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void RT_SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;


		static ref<VertexBuffer> Create(uint32_t size);
		static ref<VertexBuffer> Create(const void* data, uint32_t size);

		virtual RendererID GetRendererID() const = 0;
	};

	class IndexBuffer : public RefCounted
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* buffer, uint32_t size, uint32_t offset = 0) = 0;

		virtual const uint32_t GetCount() const = 0;
		virtual uint32_t GetSize() const = 0;

		virtual RendererID GetRendererID() const = 0;

		static ref<IndexBuffer> Create(uint32_t count);
		static ref<IndexBuffer> Create(const void* data, uint32_t count);
	};
}

#endif 

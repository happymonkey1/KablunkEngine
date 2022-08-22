#ifndef KABLUNK_RENDERER_TEXTURE_H
#define KABLUNK_RENDERER_TEXTURE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RendererTypes.h"

#include <string>

namespace Kablunk 
{
	class Texture : public RefCounted
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual RendererID GetRendererID() const = 0;
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual uint64_t GetHash() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual IntrusiveRef<Image2D> GetImage() const = 0;

		virtual ImageFormat GetFormat() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual Buffer& GetWriteableBuffer() = 0;
		virtual bool operator==(const Texture2D& other) const = 0;

		static IntrusiveRef<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static IntrusiveRef<Texture2D> Create(const std::string& path);
	};
}

#endif

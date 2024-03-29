#ifndef KABLUNK_RENDERER_TEXTURE_H
#define KABLUNK_RENDERER_TEXTURE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RendererTypes.h"

#include "Kablunk/Asset/Asset.h"

#include <filesystem>
#include <string>

namespace kb 
{
    struct texture_properties
    {
        // flag to set whether a local buffer of the image data is stored in ram
        bool m_keep_local_buffer = false;
    };

	class Texture : public asset::Asset
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
		virtual ref<Image2D> GetImage() const = 0;

		virtual ImageFormat GetFormat() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual owning_buffer& GetWriteableBuffer() = 0;
        virtual const owning_buffer& get_buffer() const = 0;
		virtual bool operator==(const Texture2D& other) const = 0;

		virtual bool loaded() const = 0;

		// static method to get the asset type of the class
		static asset::AssetType get_static_type() { return asset::AssetType::Texture; }

		static ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
        static ref<Texture2D> Create(const std::string& path);
	private:
		virtual void Invalidate() = 0;
	};
}

#endif

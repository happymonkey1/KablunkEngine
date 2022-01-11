#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_TEXTURE_H
#define KABLUNK_PLATFORM_VULKAN_TEXTURE_H

#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Core/Buffer.h"
#include "Kablunk/Renderer/Texture.h"
#include "Platform/Vulkan/VulkanContext.h"


#include <vulkan/vulkan.h>

namespace Kablunk
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data);
		VulkanTexture2D(const std::string& path);
		virtual ~VulkanTexture2D();

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual IntrusiveRef<Image2D> GetImage() const override { return m_image; }

		virtual ImageFormat GetFormat() const { return m_format; }

		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual RendererID GetRendererID() const override { return 0; }


		virtual Buffer GetWriteableBuffer() override;

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot) const override;

		virtual bool operator==(const Texture& other) const override;
	private:
		void Invalidate();
		bool LoadImage(const std::string& filepath);
	private:
		std::string m_filepath;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_size;

		IntrusiveRef<Image2D> m_image;

		ImageFormat m_format;

		Buffer m_image_data;
	};

}

#endif

#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_TEXTURE_H
#define KABLUNK_PLATFORM_VULKAN_TEXTURE_H

#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Core/Buffer.h"
#include "Kablunk/Renderer/Texture.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "Platform/Vulkan/VulkanImage.h"

#include <vulkan/vulkan.h>

namespace kb
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(ImageFormat format, uint32_t width, uint32_t height, const void* data);
		VulkanTexture2D(const std::string& path);
		virtual ~VulkanTexture2D();

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual ref<Image2D> GetImage() const override { return m_image; }

		virtual ImageFormat GetFormat() const { return m_format; }

		virtual uint32_t GetWidth() const override { return m_width; }
		virtual uint32_t GetHeight() const override { return m_height; }
		virtual RendererID GetRendererID() const override { return 0; }
		virtual uint64_t GetHash() const override { return m_hash; }

		const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const { return m_image.As<VulkanImage2D>()->GetDescriptor(); }

		virtual Buffer& GetWriteableBuffer() override;
        virtual const Buffer& get_buffer() const override { return m_image_data; }

		virtual bool loaded() const override { return m_loaded; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot) const override;
		virtual bool operator==(const Texture2D& other) const override;
	private:
		virtual void Invalidate() override;
		bool LoadImage(const std::string& filepath);
	private:
		std::string m_filepath;
        u64 m_hash = 0ull;
		uint32_t m_width;
		uint32_t m_height;

		ref<Image2D> m_image;

		ImageFormat m_format;

        // buffer of image data stored on cpu
		Buffer m_image_data;

		bool m_loaded = false;
	};

}

#endif

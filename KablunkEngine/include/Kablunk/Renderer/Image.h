#pragma once
#ifndef KABLUNK_RENDERER_IMAGE_H
#define KABLUNK_RENDERER_IMAGE_H


#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Buffer.h"
#include <glm/glm.hpp>

namespace Kablunk
{
	enum class ImageFormat
	{
		None = 0,
		RED32I,
		RED32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,
		RG16F,
		RG32F,

		SRGB,

		DEPTH32F,
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	enum class ImageUsage
	{
		None = 0,
		Texture,
		Attachment,
		Storage
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp,
		Repeat
	};

	enum class TextureFilter
	{
		None = 0,
		Linear,
		Nearest
	};

	enum class TextureType
	{
		None = 0,
		Texture2D,
		TextureCube
	};

	struct TextureProperties
	{
		TextureWrap sampler_wrap = TextureWrap::Repeat;
		TextureFilter sampler_filter = TextureFilter::Linear;
		bool generate_mips = true;
		bool SRGB = false;
		bool Storage = false;

		std::string debug_name;
	};

	struct ImageSpecification
	{
		ImageFormat format = ImageFormat::RGBA;
		ImageUsage usage = ImageUsage::Texture;
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t mips = 1;
		uint32_t layers = 1;
		bool deinterleaved = false;

		std::string debug_name;
	};

	class Image : public RefCounted
	{
	public:
		virtual ~Image() {}

		virtual void Invalidate() = 0;
		virtual void Release() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual float GetAspectRatio() const = 0;

		virtual ImageSpecification& GetSpecification() = 0;
		virtual const ImageSpecification& GetSpecification() const = 0;

		virtual const Buffer& GetBuffer() const = 0;
		virtual Buffer& GetBuffer() = 0;

		virtual void CreatePerLayerImageViews() = 0;

		virtual uint64_t GetHash() const = 0;

		// TODO: usage (eg. shader read)
	};

	class Image2D : public Image
	{
	public:
		static IntrusiveRef<Image2D> Create(ImageSpecification specification, Buffer buffer);
		static IntrusiveRef<Image2D> Create(ImageSpecification specification, const void* data = nullptr);
	};

	namespace Utils {

		inline uint32_t GetImageFormatBPP(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RED32I:   return 4;
			case ImageFormat::RED32F:   return 4;
			case ImageFormat::RGB:
			case ImageFormat::SRGB:     return 3;
			case ImageFormat::RGBA:     return 4;
			case ImageFormat::RGBA16F:  return 2 * 4;
			case ImageFormat::RGBA32F:  return 4 * 4;
			}
			KB_CORE_ASSERT(false, "Unknown ImageFormat!");
			return 0;
		}

		inline uint32_t CalculateMipCount(uint32_t width, uint32_t height)
		{
			return (uint32_t)std::floor(std::log2(glm::min(width, height))) + 1;
		}

		inline uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			return width * height * GetImageFormatBPP(format);
		}

		inline bool IsDepthFormat(ImageFormat format)
		{
			if (format == ImageFormat::DEPTH24STENCIL8 || format == ImageFormat::DEPTH32F)
				return true;

			return false;
		}

	}

}

#endif

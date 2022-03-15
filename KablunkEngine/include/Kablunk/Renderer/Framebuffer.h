#ifndef KABLUNK_RENDERER_FRAMEBUFFER_H
#define KABLUNK_RENDERER_FRAMEBUFFER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/Texture.h"
#include <map>

namespace Kablunk
{
	class Framebuffer;

	enum class FramebufferBlendMode
	{
		None = 0,
		OneZero,
		SrcAlphaOneMinusSrcAlpha,
		Additive,
		Zero_SrcColor
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(ImageFormat f)
			: format{ f } { }

		ImageFormat format;
		bool blend = true;
		FramebufferBlendMode blend_mode = FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha;
		// #TODO filtering and wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments{ attachments } {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	// Properties struct
	struct FramebufferSpecification
	{
		float scale = 1.0f;
		uint32_t width = 0, height = 0;
		FramebufferAttachmentSpecification Attachments;
		uint32_t samples = 1;
		glm::vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		bool clear_on_load = true;

		bool no_resize = false;

		bool blend = true;

		FramebufferBlendMode blend_mode = FramebufferBlendMode::None;

		bool swap_chain_target = false; 

		IntrusiveRef<Image2D> existing_image;
		std::vector<uint32_t> existing_image_layers;

		std::map<uint32_t, IntrusiveRef<Image2D>> existing_images;

		IntrusiveRef<Framebuffer> existing_framebuffer;

		std::string debug_name;
	};

	class Framebuffer : public RefCounted
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Resize(uint32_t width, uint32_t height, bool force_recreate = false) = 0;
		virtual void AddResizeCallback(const std::function<void(IntrusiveRef<Framebuffer>)>& func) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void BindTexture(uint32_t attachment_index = 0, uint32_t slot = 0) const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual RendererID GetRendererID() const = 0;

		virtual IntrusiveRef<Image2D> GetImage(uint32_t attachment_index = 0) const = 0;
		virtual IntrusiveRef<Image2D> GetDepthImage() const = 0;
		// #TODO clean up api because this is currently hard coded for reading an int from the buffer
		virtual int ReadPixel(uint32_t attachment_index, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachment_index, int value) = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static IntrusiveRef<Framebuffer> Create(const FramebufferSpecification& specs);
	};

	class FramebufferPool final
	{
	public:
		FramebufferPool(uint32_t max_framebuffers = 32);
		~FramebufferPool();

		std::weak_ptr<Framebuffer> AllocateBuffer();
		void Add(const IntrusiveRef<Framebuffer>& framebuffer);

		std::vector<IntrusiveRef<Framebuffer>>& GetAll() { return m_pool; }
		const std::vector<IntrusiveRef<Framebuffer>> &GetAll() const { return m_pool; }

		inline static FramebufferPool* Get() { return s_instance; }
	private:
		std::vector<IntrusiveRef<Framebuffer>> m_pool;

		static FramebufferPool* s_instance;
	};
}

#endif

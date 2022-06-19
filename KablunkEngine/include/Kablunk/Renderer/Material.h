#ifndef KABLUNK_RENDERER_MATERIAL_H
#define KABLUNK_RENDERER_MATERIAL_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"

namespace Kablunk
{
	enum class MaterialFlag
	{
		None		= BIT(0),
		DepthTest	= BIT(1),
		Blend		= BIT(2),
		TwoSided	= BIT(3)
	};


	class Material : public RefCounted
	{
	public:
		virtual ~Material() = default;

		virtual void Invalidate() = 0;
		virtual void Bind() = 0;

		virtual void Set(const std::string& name, float value) = 0;
		virtual void Set(const std::string& name, int value) = 0;
		virtual void Set(const std::string& name, uint32_t value) = 0;
		virtual void Set(const std::string& name, bool value) = 0;
		virtual void Set(const std::string& name, const glm::vec2& value) = 0;
		virtual void Set(const std::string& name, const glm::vec3& value) = 0;
		virtual void Set(const std::string& name, const glm::vec4& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec2& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec3& value) = 0;
		virtual void Set(const std::string& name, const glm::ivec4& value) = 0;
		virtual void Set(const std::string& name, const glm::mat3& value) = 0;
		virtual void Set(const std::string& name, const glm::mat4& value) = 0;
		virtual void Set(const std::string& name, const IntrusiveRef<Texture2D>& texture) = 0;
		virtual void Set(const std::string& name, const IntrusiveRef<Texture2D>& texture, uint32_t array_index) = 0;
		virtual void Set(const std::string& name, const IntrusiveRef<Image2D>& texture) = 0;

		virtual bool& GetBool(const std::string& name) = 0;
		virtual float& GetFloat(const std::string& name) = 0;
		virtual int& GetInt(const std::string& name) = 0;
		virtual uint32_t& GetUInt(const std::string& name) = 0;
		virtual glm::vec2& GetVec2(const std::string& name) = 0;
		virtual glm::vec3& GetVec3(const std::string& name) = 0;
		virtual glm::vec4& GetVec4(const std::string& name) = 0;
		virtual glm::mat3& GetMat3(const std::string& name) = 0;
		virtual glm::mat4& GetMat4(const std::string& name) = 0;
		virtual IntrusiveRef<Texture2D> GetTexture2D(const std::string& name) = 0;
		virtual IntrusiveRef<Texture2D> TryGetTexture2D(const std::string& name) = 0;

		virtual IntrusiveRef<Shader> GetShader() = 0;
		virtual const std::string& GetName() const = 0;

		virtual uint32_t GetFlags() const = 0;
		virtual bool GetFlag(MaterialFlag flag) const = 0;
		virtual void SetFlag(MaterialFlag flag, bool value = true) = 0;

		static IntrusiveRef<Material> Create(const IntrusiveRef<Shader>& shader, const std::string& name = "");
		static IntrusiveRef<Material> Copy(const IntrusiveRef<Material>& other, const std::string& name = "");
	};
}

#endif

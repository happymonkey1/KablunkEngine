#ifndef KABLUNK_RENDERER_MATERIAL_H
#define KABLUNK_RENDERER_MATERIAL_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Texture.h"

namespace Kablunk
{
	class Material
	{
	public:

		virtual ~Material() = default;

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
		virtual void Set(const std::string& name, const Ref<Texture2D>& texture) = 0;

		virtual float& GetFloat(const std::string& name) = 0;
		virtual int& GetInt(const std::string& name) = 0;
		virtual uint32_t& GetUInt(const std::string& name) = 0;
		virtual glm::vec2& GetVec2(const std::string& name) = 0;
		virtual glm::vec3& GetVec3(const std::string& name) = 0;
		virtual glm::vec4& GetVec4(const std::string& name) = 0;
		virtual glm::mat3& GetMat3(const std::string& name) = 0;
		virtual glm::mat4& GetMat4(const std::string& name) = 0;
		virtual Ref<Texture2D> GetTexture2D(const std::string& name) = 0;

		virtual Ref<Shader> GetShader() = 0;
		virtual const std::string& GetName() const = 0;

		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");
	};
}

#endif

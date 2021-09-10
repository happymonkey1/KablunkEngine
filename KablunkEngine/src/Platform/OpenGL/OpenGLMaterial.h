#ifndef KABLUNK_PLATFORM_OPENGL_MATERIAL_H
#define KABLUNK_PLATFORM_OPENGL_MATERIAL_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Material.h"

namespace Kablunk
{
	class OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial() = default;
		OpenGLMaterial(const Ref<Shader>& shader, const std::string& name = "");
		virtual ~OpenGLMaterial() = default;

		virtual void Invalidate() override;

		virtual void Set(const std::string& name, float value) override;
		virtual void Set(const std::string& name, int value) override;
		virtual void Set(const std::string& name, uint32_t value) override;
		virtual void Set(const std::string& name, bool value) override;
		virtual void Set(const std::string& name, const glm::vec2& value) override;
		virtual void Set(const std::string& name, const glm::vec3& value) override;
		virtual void Set(const std::string& name, const glm::vec4& value) override;
		virtual void Set(const std::string& name, const glm::ivec2& value) override;
		virtual void Set(const std::string& name, const glm::ivec3& value) override;
		virtual void Set(const std::string& name, const glm::ivec4& value) override;
		virtual void Set(const std::string& name, const glm::mat3& value) override;
		virtual void Set(const std::string& name, const glm::mat4& value) override;
		virtual void Set(const std::string& name, const Ref<Texture2D>& texture) override;

		virtual float& GetFloat(const std::string& name) override;
		virtual int& GetInt(const std::string& name) override;
		virtual uint32_t& GetUInt(const std::string& name) override;
		virtual glm::vec2& GetVec2(const std::string& name) override;
		virtual glm::vec3& GetVec3(const std::string& name) override;
		virtual glm::vec4& GetVec4(const std::string& name) override;
		virtual glm::mat3& GetMat3(const std::string& name) override;
		virtual glm::mat4& GetMat4(const std::string& name) override;
		virtual Ref<Texture2D> GetTexture2D(const std::string& name) override;

		virtual Ref<Shader> GetShader() override;
		virtual const std::string& GetName() const override;
	private:
		Ref<Shader> m_shader;
		std::string m_name;

		std::vector<Ref<Texture2D>> m_texture;
	};
}

#endif

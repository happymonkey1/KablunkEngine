#include "kablunkpch.h"

#include "Platform/OpenGL/OpenGLMaterial.h"

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

namespace Kablunk
{

	OpenGLMaterial::OpenGLMaterial(const IntrusiveRef<Shader>& shader, const std::string& name /*= ""*/)
		: m_shader{ shader }, m_name{ name }, m_textures{ std::vector<IntrusiveRef<Texture2D>>{} }
	{

	}

	void OpenGLMaterial::Bind()
	{
		m_shader->Bind();

		// #TODO set uniforms

		for (size_t i = 0; i < m_textures.size(); ++i)
		{
			m_textures[i]->Bind();
		}
	}

	int32_t OpenGLMaterial::GetGLLocation(const std::string& name)
	{
		return glGetUniformLocation(m_shader->GetRendererID(), name.c_str());
	}

	void OpenGLMaterial::Set(const std::string& name, float value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform1f(loc, value);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, int value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform1i(loc, value);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, uint32_t value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform1ui(loc, value);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, bool value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform1i(loc, value);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::vec2& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform2f(loc, value.x, value.y);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::vec3& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform3f(loc, value.x, value.y, value.z);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::vec4& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform4f(loc, value.x, value.y, value.z, value.w);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::ivec2& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform2i(loc, value.x, value.y);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::ivec3& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform3i(loc, value.x, value.y, value.z);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::ivec4& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniform4i(loc, value.x, value.y, value.z, value.w);
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::mat3& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(value));
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const glm::mat4& value)
	{
		Set(name, [value](GLint loc)
			{
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
			});
	}

	void OpenGLMaterial::Set(const std::string& name, const IntrusiveRef<Texture2D>& texture)
	{
		// #TODO
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void OpenGLMaterial::Set(const std::string& name, const IntrusiveRef<Texture2D>& texture, uint32_t array_index)
	{
		// #TODO
		KB_CORE_ASSERT(false, "not implemented!");
	}

	void OpenGLMaterial::Set(const std::string& name, const IntrusiveRef<Image2D>& image)
	{
		// #TODO
		KB_CORE_ASSERT(false, "not implemented!");
	}

	float& OpenGLMaterial::GetFloat(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		float t = 0;
		return t;
	}

	int& OpenGLMaterial::GetInt(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		int t = 0;
		return t;
	}

	uint32_t& OpenGLMaterial::GetUInt(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		uint32_t t = 0;
		return t;
	}

	glm::vec2& OpenGLMaterial::GetVec2(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return glm::vec2{0, 0};
	}

	glm::vec3& OpenGLMaterial::GetVec3(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return glm::vec3{0, 0, 0};
	}

	glm::vec4& OpenGLMaterial::GetVec4(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return glm::vec4{0, 0, 0, 0};
	}

	glm::mat3& OpenGLMaterial::GetMat3(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return glm::mat3{1.0f};
	}

	glm::mat4& OpenGLMaterial::GetMat4(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return glm::mat4{1.0f};
	}

	IntrusiveRef<Texture2D> OpenGLMaterial::GetTexture2D(const std::string& name)
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return nullptr;
	}

}

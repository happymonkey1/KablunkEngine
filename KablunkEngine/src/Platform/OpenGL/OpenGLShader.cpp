#include "kablunkpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace kb
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		else if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		else
		{
			KB_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}
	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
        KB_PROFILE_FUNC();

		std::string shaderSrc = ReadFile(filePath);
		auto shaderSources = PreProcess(shaderSrc);
		Compile(shaderSources);

		// Get name of file from filepath
		auto lastSlash = filePath.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
		auto lastDot = filePath.rfind('.');
		auto count = (lastDot == std::string::npos) ? filePath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filePath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name{name}
	{
        KB_PROFILE_FUNC();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		Compile(sources);
	}

	OpenGLShader::~OpenGLShader()
	{
        KB_PROFILE_FUNC();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
        KB_PROFILE_FUNC();

		std::string result;
		std::ifstream in{ filePath, std::ios::in | std::ios::binary };
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();

			return result;
		}
		else
		{
			KB_CORE_ERROR("Shader file could not be opened '{0}'", filePath);
			return "";
		}
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
        KB_PROFILE_FUNC();

		std::unordered_map<GLenum, std::string> shaderSources;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			KB_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			KB_CORE_ASSERT(ShaderTypeFromString(type), "Invalid/Unsupported shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		/*
		* FROM https://www.khronos.org/opengl/wiki/Shader_Compilation
		*/

        KB_PROFILE_FUNC();

		GLuint program = glCreateProgram();
		KB_CORE_ASSERT(shaderSources.size() <= 2, "Only 2 shaders are supported!");
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string source = kv.second;
			
			GLuint shader = glCreateShader(type);

			const GLchar* sourceCstr = source.c_str();
			glShaderSource(shader, 1, &sourceCstr, 0);

			// Compile the vertex shader
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				KB_CORE_ERROR("{0}", infoLog.data());
				KB_CORE_ASSERT(false, "  Shader failed to compile!");
				
				break;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}


		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			for (auto& id : glShaderIDs)
				glDeleteShader(id);

			KB_CORE_ERROR("{0}", infoLog.data());
			KB_CORE_ASSERT(false, "  Shader(s) failed to link!");
			

			return;
		}

		// Always detach shaders after a successful link.
		for (auto& id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
		m_RendererID = program;
	}

	void OpenGLShader::Bind() const
	{
        KB_PROFILE_FUNC();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
        KB_PROFILE_FUNC();

		glUseProgram(0);
	}

	void OpenGLShader::Reload(bool force_compile /*= false*/)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	size_t OpenGLShader::GetHash() const
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return 0;
	}

	void OpenGLShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
	{
		KB_CORE_ASSERT(false, "not implemented!");
	}

	const std::unordered_map<std::string, ShaderBuffer>& OpenGLShader::GetShaderBuffers() const
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return {};
	}

	const std::unordered_map<std::string, ShaderResourceDeclaration>& OpenGLShader::GetResources() const
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return {};
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
        KB_PROFILE_FUNC();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
        KB_PROFILE_FUNC();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
        KB_PROFILE_FUNC();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
        KB_PROFILE_FUNC();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
        KB_PROFILE_FUNC();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
        KB_PROFILE_FUNC();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
        KB_PROFILE_FUNC();

		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetUniformBuffer(const ShaderUniformBuffer& buffer, const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(buffer.renderer_ID, offset, size, data);
	}

	void OpenGLShader::SetUniformBuffer(const std::string& name, const void* data, uint32_t size)
	{
		GLint loc = glGetUniformLocation(m_RendererID, name.c_str());
		if (loc == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glNamedBufferSubData(loc, 0, size, data);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformUInt(const std::string& name, uint32_t value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform1ui(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			KB_CORE_ERROR("Could not find uniform '{0}'!", name);
			return;
		}
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}


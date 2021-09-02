#ifndef OPENGLSHADER_H
#define OPENGLSHADER_H
#include "Kablunk/Renderer/Shader.h"
#include "Kablunk/Renderer/Renderer.h"



// Forward declaration
// TODO: REMOVE
typedef unsigned int GLenum;


namespace Kablunk
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const;
		virtual void Unbind() const;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;

		virtual const std::string& GetName() const override { return m_Name; }

		void SetUniformBuffer(const ShaderUniformBuffer& buffer, const void* data, uint32_t size, uint32_t offset);
		void SetUniformBuffer(const std::string& name, const void* data, uint32_t size);

		// #TODO should probably rename to SetUniform...
		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformUInt(const std::string& name, uint32_t value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);
		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		RendererID m_RendererID;
		std::string m_Name;
	};
}


#endif

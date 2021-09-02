#pragma once
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Kablunk {

	enum class ShaderUniformType
	{
		None = 0,
		Bool,
		Int,
		UInt,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat3, 
		Mat4,
		IVec2,
		IVec3,
		IVec4
	};

	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offest);

		const std::string& GetName() const { return m_name; }
		ShaderUniformType GetType() const { return m_type; }
		uint32_t GetSize() const { return m_size; }
		uint32_t GetOffset() const { return m_offset; }
	private:
		std::string m_name;
		ShaderUniformType m_type;
		uint32_t m_size = 0;
		uint32_t m_offset = 0;
	};

	struct ShaderUniformBuffer
	{
		std::string Name;
		uint32_t Index;
		uint32_t Binding;
		uint32_t Size;
		uint32_t RendererID;
		std::vector<ShaderUniform> Uniforms;
	};

	class Shader 
	{
	public:
		virtual ~Shader() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::string& filePath);
		// #REMOVE deprecated
		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public: 
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name);
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}

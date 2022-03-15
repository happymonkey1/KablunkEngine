#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/ShaderUniform.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <string>

namespace Kablunk {

	enum class ShaderUniformType
	{
		None = 0, Bool, Int, UInt, Float, Vec2, Vec3, Vec4, Mat3, Mat4,
		IVec2, IVec3, IVec4
	};

	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset);

		const std::string& GetName() const { return m_name; }
		ShaderUniformType GetType() const { return m_type; }
		uint32_t GetSize() const { return m_size; }
		uint32_t GetOffset() const { return m_offset; }

		static std::string UniformTypeToString(ShaderUniformType type);
	private:
		std::string m_name;
		ShaderUniformType m_type = ShaderUniformType::None;
		uint32_t m_size = 0;
		uint32_t m_offset = 0;
	};

	struct ShaderUniformBuffer
	{
		std::string name;
		uint32_t index;
		uint32_t binding_point;
		uint32_t size;
		uint32_t renderer_ID;
		std::vector<ShaderUniform> uniforms;
	};

	struct ShaderStorageBuffer
	{
		std::string name;
		uint32_t index;
		uint32_t binding_point;
		uint32_t size;
		uint32_t renderer_ID;
		//std::vector<ShaderUniform> Uniforms;
	};

	struct ShaderBuffer
	{
		std::string name;
		uint32_t size = 0;
		std::unordered_map<std::string, ShaderUniform> uniforms;
	};


	class Shader : public RefCounted
	{
	public:
		
		using ShaderReloadedCallback = std::function<void()>;
		virtual ~Shader() {}

		virtual void Reload(bool force_compile = false) = 0;
		virtual size_t GetHash() const = 0;

		virtual void AddShaderReloadedCallback(const ShaderReloadedCallback& callback) = 0;

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
		virtual RendererID GetRendererID() const = 0;

		virtual const std::unordered_map<std::string, ShaderBuffer>& GetShaderBuffers() const = 0;
		virtual const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResources() const = 0;

		static IntrusiveRef<Shader> Create(const std::string& filePath, bool force_compile = false);
	private:
		inline static std::vector<IntrusiveRef<Shader>> s_all_shaders; // Temporary while we don't have an asset system
	};

	class ShaderLibrary : public RefCounted
	{
	public: 
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

		void Add(const IntrusiveRef<Shader>& shader);
		void Add(const std::string& name, const IntrusiveRef<Shader>& shader);
		IntrusiveRef<Shader> Load(const std::string& filepath);
		IntrusiveRef<Shader> Load(const std::string& name, const std::string& filepath);

		IntrusiveRef<Shader> Get(const std::string& name);

		bool Exists(const std::string& name);
	private:
		std::unordered_map<std::string, IntrusiveRef<Shader>> m_shaders;
	};
}

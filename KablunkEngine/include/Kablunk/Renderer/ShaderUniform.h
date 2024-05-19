#pragma once
#ifndef KABLUNK_RENDERER_SHADER_UNIFORM_H
#define KABLUNK_RENDERER_SHADER_UNIFORM_H

#include <string>
#include <vector>

namespace kb
{
	enum class ShaderDomain
	{
		None = 0, Vertex = 0, Pixel = 1 // unused
	};

	class ShaderResourceDeclaration
	{
	public:
		ShaderResourceDeclaration() = default;
        ~ShaderResourceDeclaration() = default;

		ShaderResourceDeclaration(
            const std::string& name,
            u32 p_set,
            uint32_t resourceRegister,
            uint32_t count
        )
            : m_name{ name }, m_descriptor_set{ p_set }, m_register{ resourceRegister }, m_count{ count }
        {
        }

		virtual const std::string& GetName() const noexcept { return m_name; }
		virtual u32 GetRegister() const noexcept { return m_register; }
		virtual u32 GetCount() const noexcept { return m_count; }
        virtual u32 get_set() const noexcept { return m_descriptor_set; }

	private:
		std::string m_name;
        u32 m_descriptor_set = 0;
		u32 m_register = 0;
		u32 m_count = 0;
	};

	typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;
}

#endif

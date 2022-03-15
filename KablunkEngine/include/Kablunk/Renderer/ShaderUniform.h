#pragma once
#ifndef KABLUNK_RENDERER_SHADER_UNIFORM_H
#define KABLUNK_RENDERER_SHADER_UNIFORM_H

#include <string>
#include <vector>

namespace Kablunk
{
	enum class ShaderDomain
	{
		None = 0, Vertex = 0, Pixel = 1 // unused
	};

	class ShaderResourceDeclaration
	{
	public:
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(const std::string& name, uint32_t resourceRegister, uint32_t count)
			: m_name{ name }, m_register{ resourceRegister }, m_count{ count } { }

		virtual const std::string& GetName() const { return m_name; }
		virtual uint32_t GetRegister() const { return m_register; }
		virtual uint32_t GetCount() const { return m_count; }
	private:
		std::string m_name;
		uint32_t m_register = 0;
		uint32_t m_count = 0;
	};

	typedef std::vector<ShaderResourceDeclaration*> ShaderResourceList;
}

#endif

#ifndef KABLUNK_UTILITIES_REFLECTION_KABLUNK_TYPES_H
#define KABLUNK_UTILITIES_REFLECTION_KABLUNK_TYPES_H

#include "Kablunk/Utilities/Reflection/TypeTraits.h"

namespace kb::Reflect
{
	using SerializeFunc = void(*)();

	struct TypeData
	{
		void* Data = nullptr;
		TypeTraits* Type = nullptr;

		TypeData() = default;
		TypeData(void* data, TypeTraits* type) : Data{ data }, Type{ type } { }

		template <typename T>
		static TypeData Bind(T* other_ptr);

		template <class T>
		T* Cast() const;

		static TypeData Create(TypeTraits* type_traits);

		bool operator==(const TypeData& other) const
		{
			return Data == other.Data && Type == other.Type;
		}
	};

	struct TypeKey
	{



	};

	struct ValidTypes
	{
		static constexpr const char* INT = "int";
		static constexpr const char* CSTRING = "const char*";
		static constexpr const char* STRING = "std::string";
		static constexpr const char* FLOAT = "float";
		static constexpr const char* DOUBLE = "double";
		static constexpr const char* VEC2 = "glm::vec2";
		static constexpr const char* IVEC2 = "glm::ivec2";
		static constexpr const char* VEC3 = "glm::vec3";
		static constexpr const char* IVEC3 = "glm::ivec3";

		static bool is_valid_type(const std::string& type)
		{
			if (type == INT)
				return true;
			else if (type == CSTRING)
				return true;
			else if (type == FLOAT)
				return true;
			else if (type == DOUBLE)
				return true;
			else if (type == VEC2)
				return true;
			else if (type == IVEC2)
				return true;
			else if (type == VEC3)
				return true;
			else if (type == IVEC3)
				return true;
			else
				return false;
		}
	};

}

#endif

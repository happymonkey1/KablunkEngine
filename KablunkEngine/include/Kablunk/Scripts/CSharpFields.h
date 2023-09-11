#pragma once
#ifndef KABLUNK_SCRIPTS_CSHARP_FIELDS_H
#define KABLUNK_SCRIPTS_CSHARP_FIELDS_H

#include "Kablunk/Core/Core.h"


#include <string>

namespace kb
{
	enum class FieldType
	{
		None = 0,
		Bool,
		Float,
		Int,
		UnsignedInt,
		String,
		Vec2,
		Vec3,
		Vec4,
		ClassReference,
		Entity
	};

	inline const char* FieldTypeToCStr(FieldType type)
	{
		switch (type)
		{
		case FieldType::None:			KB_CORE_ASSERT(false, "Unreachable code"); return "";
		case FieldType::Bool:			return "Bool";
		case FieldType::Float:			return "Float";
		case FieldType::Int:			return "Int";
		case FieldType::UnsignedInt:	return "UnsignedInt";
		case FieldType::String:			return "String";
		case FieldType::Vec2:			return "Vec2";
		case FieldType::Vec3:			return "Vec3";
		case FieldType::Vec4:			return "Vec4";
		case FieldType::Entity:			return "Entity";
		}

		return "Unknown";
	}

	struct PublicField
	{
		std::string Name;
		std::string TypeName;
		FieldType Type;
		bool Is_read_only;

		PublicField() = default;
		PublicField(const std::string& name, const std::string& type_name, FieldType type, bool is_read_only = false);

		PublicField(const PublicField& other);
		PublicField(PublicField&& other);
		~PublicField();

		PublicField& operator=(const PublicField& other);
		PublicField& operator=(PublicField&& other) = default;

	};
}
#endif

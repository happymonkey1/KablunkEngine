#ifndef KABLUNK_UTILITIES_REFLECTION_INTEGRAL_TYPE_TRAITS_H
#define KABLUNK_UTILITIES_REFLECTION_INTEGRAL_TYPE_TRAITS_H

#include "Kablunk/Core/Logger.h"
#include "Kablunk/Utilities/Reflection/TypeTraits.h"

namespace Kablunk::Reflect
{
	// Integral type traits must have specialized templates to compile. While this is generally not good practice,
	// because each type must be defined to work in the reflection system as of right now, this is
	// fine. Not having a type defined is an error and the compiler should warn us if the definition
	// cannot be found
	template <typename T>
	static TypeTraits* GetIntegralTypeTraits()
	{
		static_assert(false, "Integral type template specialization not defined");
		return nullptr;
	}

	// ========
	//   bool
	// ========

	struct bool_TypeTraits : public TypeTraits
	{
		bool_TypeTraits()
			: TypeTraits{ "bool", sizeof(bool) }
		{

		}
	};

	template <>
	static TypeTraits* GetIntegralTypeTraits<bool>()
	{
		static bool_TypeTraits type_description;
		return &type_description;
	}

	// =======
	//   int
	// =======

	struct int_TypeTraits : public TypeTraits
	{
		int_TypeTraits()
			: TypeTraits{ "int", sizeof(int) }
		{

		}
	};

	template <>
	static TypeTraits* GetIntegralTypeTraits<int>()
	{
		static int_TypeTraits type_description;
		return &type_description;
	}

	// =======
	//   char
	// =======

	struct char_TypeTraits : public TypeTraits
	{
		char_TypeTraits()
			: TypeTraits{ "char", sizeof(char) }
		{

		}
	};

	template <>
	static TypeTraits* GetIntegralTypeTraits<char>()
	{
		static char_TypeTraits type_description;
		return &type_description;
	}

	// ============
	//   uint32_t
	// ============

	struct uint32_TypeTraits : public TypeTraits
	{
		uint32_TypeTraits()
			: TypeTraits{ "uint32_t", sizeof(uint32_t) }
		{

		}
	};

	template <>
	static TypeTraits* GetIntegralTypeTraits<uint32_t>()
	{
		static uint32_TypeTraits type_description;
		return &type_description;
	}

	// ============
	//   uint64_t
	// ============

	struct uint64_TypeTraits : public TypeTraits
	{
		uint64_TypeTraits()
			: TypeTraits{ "uint64_t", sizeof(uint64_t) }
		{

		}
	};

	template <>
	static TypeTraits* GetIntegralTypeTraits<uint64_t>()
	{
		static uint64_TypeTraits type_description;
		return &type_description;
	}
}

#endif

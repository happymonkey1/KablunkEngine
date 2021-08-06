#include "kablunkpch.h"
#include "Kablunk/Utilities/Reflection/IntegralTypeTraits.h"

namespace Kablunk::Reflect
{

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

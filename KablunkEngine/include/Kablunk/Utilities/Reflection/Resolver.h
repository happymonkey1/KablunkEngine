#ifndef KABLUNK_UTILITIES_REFLECTION_RESOLVER_H
#define KABLUNK_UTILITIES_REFLECTION_RESOLVER_H

#include "Kablunk/Utilities/Reflection/TypeTraits.h"
#include "Kablunk/Utilities/Reflection/IntegralTypeTraits.h"

namespace kb::Reflect
{
	class BaseResolver
	{
	public:
		// Use SFINAE to determine if T::Reflection is a member
		// https://en.cppreference.com/w/cpp/language/sfinae

		using Defined = uint16_t;
		using NotDefined = uint8_t;

		template <typename T>
		static Defined f(decltype(&T::Reflection));

		// fallback template 
		template <typename T>
		static NotDefined f(...);

		template <typename T>
		struct IsReflectionDefined
		{
			enum 
			{ 
				Value = sizeof(f<T>(nullptr)) == sizeof(Defined)
			};
		};

	public:

		template <typename T, typename std::enable_if<IsReflectionDefined<T>::Value, int>::type = 0>
		static TypeTraits* Get()
		{
			return &T::Reflection;
		}

		// SFINAE Backup for integral type
		template <typename T, typename std::enable_if<!IsReflectionDefined<T>::Value, int>::type = 0>
		static TypeTraits* Get()
		{
			return GetIntegralTypeTraits<T>();
		}
	};

	template <typename T>
	class TypeResolver
	{
	public:
		static TypeTraits* Get()
		{
			return BaseResolver::Get<T>();
		}
	};
}

#endif

#ifndef KABLUNK_UTILITIES_REFLECTION_REFLECTION_CORE_H
#define KABLUNK_UTILITIES_REFLECTION_REFLECTION_CORE_H

#include "Kablunk/Utilities/Reflection/TypeTraits.h"
#include "Kablunk/Utilities/Reflection/IntegralTypeTraits.h"
#include "Kablunk/Utilities/Reflection/Resolver.h"

// Use to declare reflection for a user defined struct
#define KB_REFLECT() \
	friend class kb::Reflect::BaseResolver; \
	static kb::Reflect::struct_TypeTraits Reflection; \
	static void InitReflection(kb::Reflect::struct_TypeTraits*);

#define KB_REFLECT_STRUCT_BEGIN(type) \
	kb::Reflect::struct_TypeTraits type::Reflection{ type::InitReflection }; \
	void type::InitReflection(kb::Reflect::struct_TypeTraits* reflection) \
	{ \
		using T = type; \
		reflection->Name = #type; \
		reflection->Size = sizeof(T); \
		reflection->Members = { 

#define KB_REFLECT_STRUCT_DEFINE_MEMBER(name) \
		{ #name, offsetof(T, name), kb::Reflect::TypeResolver<decltype(T::name)>::Get() },

#define KB_REFLECT_STRUCT_END() \
		}; \
	}

#endif

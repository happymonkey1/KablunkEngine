#ifndef KABLUNK_UTILITIES_REFLECTION_INTEGRAL_TYPE_TRAITS_H
#define KABLUNK_UTILITIES_REFLECTION_INTEGRAL_TYPE_TRAITS_H

#include "Kablunk/Core/Log.h"
#include "Kablunk/Utilities/Reflection/TypeTraits.h"

namespace Kablunk::Reflect
{
	// Integral type traits are defined in the cpp file. While this is generally not good practice,
	// because each type must be defined to work in the reflection system as of right now, this is
	// fine. Not having a type defined is an error and the compiler should warn us if the definition
	// cannot be found
	template <typename T>
	static TypeTraits* GetIntegralTypeTraits()
	{
		KB_CORE_ASSERT(false, "Integral type ({0}) not defined", typeid(T));
		return nullptr;
	}
}

#endif

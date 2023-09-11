#include "kablunkpch.h"
#include "Kablunk/Utilities/Reflection/TypeTraits.h"

namespace kb::Reflect
{
	struct_TypeTraits::struct_TypeTraits(IntantiateReflectionFunc init)
		: TypeTraits{ nullptr, 0 }
	{
		init(this);
	}
	
	struct_TypeTraits::struct_TypeTraits(const char* name, size_t size, const std::initializer_list<Member>& members)
		: TypeTraits{ name, size }, Members{ members }
	{

	}
}

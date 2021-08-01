#ifndef KABLUNK_UTILITIES_REFLECTION_TYPE_TRAITS_H
#define KABLUNK_UTILITIES_REFLECTION_TYPE_TRAITS_H

#include <string>

namespace Kablunk::Reflect
{
	struct TypeTraits
	{
		const char* Name;
		size_t Size;
		
		TypeTraits() = default;
		TypeTraits(const char*, size_t) = default;
		virtual ~TypeTraits() = default;
		
		virtual std::string GetName() const { return Name; }
	};

}

#endif

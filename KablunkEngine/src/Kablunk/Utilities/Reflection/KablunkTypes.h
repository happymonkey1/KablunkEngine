#ifndef KABLUNK_UTILITIES_REFLECTION_KABLUNK_TYPES_H
#define KABLUNK_UTILITIES_REFLECTION_KABLUNK_TYPES_H

#include "Kablunk/Utilities/Reflection/TypeTraits.h"

namespace Kablunk::Reflect
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

}

#endif

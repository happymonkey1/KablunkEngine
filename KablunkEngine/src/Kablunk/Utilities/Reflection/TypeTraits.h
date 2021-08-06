#ifndef KABLUNK_UTILITIES_REFLECTION_TYPE_TRAITS_H
#define KABLUNK_UTILITIES_REFLECTION_TYPE_TRAITS_H

#include <string>
#include <vector>

namespace Kablunk::Reflect
{
	struct TypeTraits
	{
		const char* Name;
		size_t Size;

		TypeTraits() = delete;
		TypeTraits(const char* name , size_t size)
			: Name{ name }, Size{ size } { }

		//TypeTraits(const TypeTraits&) = delete;
		//TypeTraits& operator=(const TypeTraits&) = delete;

		virtual ~TypeTraits() noexcept = default;
		
		virtual std::string GetName() const noexcept { return Name; }
	};

	struct struct_TypeTraits : public TypeTraits
	{
		using IntantiateReflectionFunc = void(*)(struct_TypeTraits*);

		struct Member
		{
			const char* Name;
			size_t Offset;
			TypeTraits* Type;

			std::string GetName() const noexcept { return Name; }
			std::string GetTypeName() const noexcept { return Type->GetName(); }
		};

		std::vector<Member> Members;

		// Takes a function pointer to initialization function defined by macro in reflected struct
		struct_TypeTraits(IntantiateReflectionFunc init);

		struct_TypeTraits(const char* name, size_t size, const std::initializer_list<Member>& members);
	};
}



#endif

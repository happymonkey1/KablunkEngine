#ifndef KABLUNK_UTILITIES_STATIC_REFLECTION_H
#define KABLUNK_UTILITIES_STATIC_REFLECTION_H

#include <string>
#include <array>

namespace Kablunk
{
	// Large number of possible fields
	static constexpr uint32_t k_max_number_of_fields = 256;

	enum class TypeName
	{
		TYPE = 0
	};

	struct TypeTraits
	{
		std::string Name;
		TypeName Identifier;
		size_t Size;
	};

	struct Field
	{
		std::string Name;
		TypeTraits* Type;
		size_t Offset;
	};

	struct Class
	{
		std::array<Field, k_max_number_of_fields> fields;
	};

	// ==========
	//   Macros
	// ==========

	// instantiate new type
	#define DEFINE_TYPE(TYPE) \
	template<> \
	TypeTraits* GetType<TYPE>() \
	{ \
		static TypeTraits type; \
		type.Name = #TYPE; \
		type.Identifier = TypeName::TYPE \
		type.Size = sizeof(TYPE); \
		return &type; \
	} \

	// instantiate new class
	#define BEGIN_ATTRIBUTES_FOR(CLASS) \
	template<> \
	Class* GetClass<CLASS>() \
	{ \
		using ClassType = CLASS; \
		static Class local_class; \
		enum { BASE = __COUNTER__ }; \

		#define DEFINE_MEMBER(NAME) \
			enum { NAME##Index = __COUNTER__ - BASE - 1 }; \
			local_class.fields[NAME##Index].Name = { #NAME }; \
			local_class.fields[NAME##Index].Type = GetType<decltype>(ClassType::NAME)>(); \
			local_class.fields[NAME##INDEX].Offset = offsetof(ClassType, NAME); \

		#define END_ATTRIBUTES \
			return &local_class; \
	} \

}


#endif

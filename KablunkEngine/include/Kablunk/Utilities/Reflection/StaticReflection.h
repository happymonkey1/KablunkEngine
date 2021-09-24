#if 0
#define KABLUNK_UTILITIES_STATIC_REFLECTION_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Log.h"

#include <vector>
#include <string>

namespace Kablunk
{
	namespace Reflect
	{
		struct TypeDescription
		{
			const char* Name;
			size_t Size;

			TypeDescription(const char* name, size_t size) 
				: Name{ name }, Size{ size }
			{

			}

			virtual ~TypeDescription() = default;
			virtual std::string GetName() const { return Name; }
		};

		struct ReflectionContainer : TypeDescription
		{
			struct Member
			{
				const char* Name;
				size_t Offset;
				TypeDescription* Type;
			};

			std::vector<Member> Members;

			ReflectionContainer()
				: TypeDescription{ nullptr, 0 }
			{

			}

			ReflectionContainer(void (*init)(ReflectionContainer*))
				: TypeDescription{ nullptr, 0 }
			{
				init(this);
			}
			ReflectionContainer(const char* name, size_t size, const std::initializer_list<Member>& members)
				: TypeDescription{ nullptr, 0 }, Members{ members }
			{

			}
		};

		// Forward declaration
		template <typename T>
		TypeDescription* GetPrimitiveTypeDescription();


		class Resolver
		{
		public:
			// Use SFINAE to determine if T::Reflection is a member
			// https://en.cppreference.com/w/cpp/language/sfinae

			using Defined = uint16_t;
			using NotDefined = uint8_t;

			template <typename T>
			static Defined f(typename T::Reflection*);

			// fallback template 
			template <typename T>
			static NotDefined f(T*);

			template <typename T>
			struct IsReflectionDefined
			{
				enum { Defined = sizeof(f<T>(nullptr)) == sizeof(Defined); };
			};

		public:
			// Reflection defined for struct
			template <typename T, typename std::enable_if<IsReflectionDefined<T>::Defined, int>::type = 0>
			static TypeDescription* Get()
			{
				return &T::Reflection;
			}

			// Reflection for types like int, char, std::string, etc
			template <typename T, typename std::enable_if<!IsReflectionDefined<T>::Defined, int>::type = 0>
			static TypeDescription* Get()
			{
				return GetPrimitiveTypeDescription();
			}
		};

		// ==========
		//   Macros
		// ==========
		
		// Use to declare Reflection for a user defined struct
		#define REFLECT() \
			friend struct reflect::Resolver; \ 
			static Reflect::ReflectionContainer Reflection; \
			static void InitReflection(Reflect::ReflectionContainer*);

		#define REFLECT_BEGIN_STRUCT(type) \
			Reflect::ReflectionContainer type::Reflection{ type::InitReflection }; \
			void type::InitReflection(Reflect::ReflectionContainer* type_description) \
			{ \
				using T = type; \
				type_description->Name = #type; \
				type_description->Size = sizeof(T); \
				type_description->Members = { 

		#define REFLECT_DEFINE_MEMBER(name) \
					{ #name, offsetof(T, name), Reflect::Resolver::Get<T>(); },

		#define REFLECT_END_STRUCT() \
				}; \
			}
	}
}


#endif

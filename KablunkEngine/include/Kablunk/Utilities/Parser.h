#ifndef KABLUNK_UTILITIES_PARSER_CPP_H
#define KABLUNK_UTILITIES_PARSER_CPP_H

#include "Kablunk/Core/Logger.h"

#include "Kablunk/Utilities/ParserToken.h"
#include "Kablunk/Utilities/Reflection/KablunkTypes.h"

#include <string>
#include <vector>
#include <sstream>
#include <string_view>

namespace kb::Parser
{
	namespace String
	{
		// #FIXME very bad performance wise. Probably alot of unnecessary dynamic allocations through strings.
		template <typename T>
		static std::string DemangleTypeIDName()
		{
			std::string mangled_name = typeid(T).name();
			std::string last_name = "";
			
			std::stringstream ss{ mangled_name };
			while (ss)
			{
				ss >> last_name;
			}

			size_t substring_index = last_name.rfind(':');

			return substring_index != std::string::npos ? last_name.substr(substring_index + 1) : last_name;
		}

		// #TODO review and implement instead of the above code
		/*
		namespace internal
		{
		  static const unsigned int FRONT_SIZE = sizeof("internal::GetTypeNameHelper<") - 1u;
		  static const unsigned int BACK_SIZE = sizeof(">::GetTypeName") - 1u;

		  template <typename T>
		  struct GetTypeNameHelper
		  {
			static const char* GetTypeName(void)
			{
			  static const size_t size = sizeof(__FUNCTION__) - FRONT_SIZE - BACK_SIZE;
			  static char typeName[size] = {};
			  memcpy(typeName, __FUNCTION__ + FRONT_SIZE, size - 1u);

			  return typeName;
			}
		  };
		}


		template <typename T>
		const char* GetTypeName(void)
		{
		  return internal::GetTypeNameHelper<T>::GetTypeName();
		}
		*/
	}

	namespace CPP
	{
		struct FindIdentifier
		{
			static constexpr const char* CLASS = "class";
			static constexpr const char* STRUCT = "struct";
		};


		template <typename T>
		static T ConvertTo(const std::string& type_as_str)
		{

		}

		static std::string strip_namespace(const std::string& str_with_namespace)
		{
			size_t index = str_with_namespace.find_last_of(':', str_with_namespace.size());
			if (index == std::string::npos)
				return str_with_namespace;
			else
				return str_with_namespace.substr(index + 1);
		}

		static std::vector<std::string> find_all_of(const char* identifier, const std::string& filepath, uint32_t num_structs_to_find = 0)
		{
			std::string text;
			std::ifstream in{ filepath, std::ios::in | std::ios::binary };
			if (in)
			{
				// Read filetext into string
				in.seekg(0, std::ios::end);
				text.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&text[0], text.size());
				in.close();


				auto struct_names = std::vector<std::string>{ };
				auto ss = std::stringstream{ text };
				auto count = uint32_t{ 0 };

				std::string word;
				while (ss)
				{
					ss >> word;
					if (word == identifier)
					{
						ss >> word;
						struct_names.emplace_back(word);

						count++;

						if (num_structs_to_find != 0)
							if (count == num_structs_to_find)
								return struct_names;
					}
				}


				return struct_names;
			}
			else
			{
				KB_CORE_ERROR("Source file could not be opened '{0}'", filepath);
				return {};
			}
		}

		static std::vector<std::string> FindStructNames(const std::string& filepath, uint32_t num_structs_to_find = 0)
		{
			return find_all_of(FindIdentifier::STRUCT, filepath, num_structs_to_find);
		}

		static std::vector<std::string> FindClassNames(const std::string& filepath, uint32_t num_structs_to_find = 0)
		{
			return find_all_of(FindIdentifier::CLASS, filepath, num_structs_to_find);
		}

		static std::vector<std::string> FindClassAndStructNames(const std::string& filepath, uint32_t num_to_find = 0)
		{
			auto structs = find_all_of(FindIdentifier::STRUCT, filepath, num_to_find);
			auto cclass = find_all_of(FindIdentifier::CLASS, filepath, num_to_find);
			structs.insert(structs.end(), cclass.begin(), cclass.end());
			return structs;
		}

		static std::string parse_type_and_field_name(const std::string& text)
		{
			std::stringstream ss{ text };
			std::string sanitized_string;

			std::string word;
			while (ss)
			{
				ss >> word;
				if (Reflect::ValidTypes::is_valid_type(word))
				{
					sanitized_string += word + " ";

					// get variable name
					ss >> word;
					sanitized_string += word;

					return sanitized_string;
				}
			}

			KB_CORE_ERROR("failed to parse type and field name from '{}'", text);
			return "INV_TYPE_AND_FIELD_DATA";
		}

		static std::vector<ParserToken> FindParserTokens(const std::string& filepath)
		{
			std::vector<ParserToken> parser_tokens = {};
			std::string text;
			std::ifstream in{ filepath, std::ios::in | std::ios::binary };
			if (in)
			{
				// Read filetext into string
				in.seekg(0, std::ios::end);
				text.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&text[0], text.size());
				in.close();


				auto struct_names = std::vector<std::string>{ };
				auto ss = std::stringstream{ text };
				auto count = uint32_t{ 0 };

				std::string word;
				while (ss)
				{
					ss >> word;
					if (Annotations::is_annotation_valid(word))
					{
						std::string next_line;
						std::getline(ss, next_line, '\n');
						if (next_line.size() == 1)
							std::getline(ss, next_line, '\n');
						
						parser_tokens.emplace_back(ParserToken{ ParserTokenDescriptor::Annotation, parse_type_and_field_name(next_line) });
					}
				}

				return parser_tokens;
			}
			else
			{
				KB_CORE_ERROR("Source file could not be opened '{0}'", filepath);
				return {};
			}
		}
	}



}

#endif

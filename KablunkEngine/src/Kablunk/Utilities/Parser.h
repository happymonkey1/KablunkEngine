#ifndef KABLUNK_UTILITIES_PARSER_CPP_H
#define KABLUNK_UTILITIES_PARSER_CPP_H

#include "Kablunk/Core/Log.h"
#include <string>
#include <vector>
#include <sstream>

namespace Kablunk::Parser
{
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
					KB_CORE_TRACE("{0}", word);
					if (word == "struct")
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
	}



}

#endif

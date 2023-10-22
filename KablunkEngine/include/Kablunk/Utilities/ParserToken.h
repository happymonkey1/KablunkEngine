#pragma once

#include "Kablunk/Core/Core.h"

#include <stdint.h>
#include <unordered_map>


namespace kb::Parser
{
	enum class ParserTokenDescriptor : uint8_t
	{
		Annotation = 0,

		NONE
	};

	const char* parser_token_descriptor_to_c_str(ParserTokenDescriptor token_type);

	struct ParserToken
	{
		ParserTokenDescriptor Type = ParserTokenDescriptor::NONE;
		std::string Data = "";

		std::string ToString() const;
	};

	namespace CPP
	{
		class Annotations
		{
		public:
			static constexpr const char* SERIALIZE_FIELD = "[[SerializeField]]";

			static bool is_annotation_valid(const std::string& potential_annotation) 
			{ 
				return s_annotation_map.find(potential_annotation) != s_annotation_map.end(); 
			}

		private:
			inline static const kb::unordered_flat_map<std::string, bool> s_annotation_map = kb::unordered_flat_map<std::string, bool>{ 
				{ std::string{ SERIALIZE_FIELD }, false }
			};
		};
	}
}

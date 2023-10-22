#include "kablunkpch.h"
#include "Kablunk/Utilities/ParserToken.h"

#include "Kablunk/Core/Core.h"

namespace kb
{

	const char* Parser::parser_token_descriptor_to_c_str(ParserTokenDescriptor token_type)
	{
		switch (token_type)
		{
			case ParserTokenDescriptor::Annotation:		return "Annotation";
			case ParserTokenDescriptor::NONE:			return "INV_PARSER_TOKEN_DESCRIPTOR";
			default:		
			{
				KB_CORE_ASSERT(false, "not implemented ParserTokenDescriptor");
				return "INV_PARSER_TOKEN_DESCRIPTOR";
			}
		}
	}

	std::string Parser::ParserToken::ToString() const
	{
		return fmt::format("ParserToken(type='{}', data='{}')", parser_token_descriptor_to_c_str(Type), Data);
	}

}

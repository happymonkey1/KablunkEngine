#include "kablunkpch.h"
#include "Kablunk/Scripts/NativeScriptCompilerLogger.h"

#include <stdarg.h>

namespace Kablunk
{

	std::string ConverToKablunkFormat(const char* format)
	{
		std::string return_format = "";
		std::string data = { format };
		size_t begin = 0;
		size_t pos = data.find("%", begin);
		size_t count = 0;
		while (pos != std::string::npos)
		{
			return_format += data.substr(begin, pos - begin);
			begin = pos + 2;

			return_format += '{';
			return_format += '0' + count++;
			return_format += "}";

			pos = data.find("%", begin);
		}

		return_format += data.substr(begin, strlen(format) - begin);

		return return_format;
	}

	void NativeScriptCompilerLogger::LogError(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		
		//KB_CORE_ERROR(ConverToKablunkFormat(format).c_str(), args);
		kbLog(LogLevel::ERR, format, args);
		va_end(args);
	}

	void NativeScriptCompilerLogger::LogWarning(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		//KB_CORE_WARN(ConverToKablunkFormat(format).c_str(), args);
		kbLog(LogLevel::WARN, format, args);
		va_end(args);
	}

	void NativeScriptCompilerLogger::LogInfo(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		//KB_CORE_INFO(ConverToKablunkFormat(format).c_str(), args);
		kbLog(LogLevel::INFO,format, args);
		va_end(args);
	}

}

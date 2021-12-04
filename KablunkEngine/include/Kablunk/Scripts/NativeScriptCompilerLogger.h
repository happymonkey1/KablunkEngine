#ifndef KABLUNK_SCRIPTS_NATIVE_SCRIPT_COMPILER_LOGGER_H
#define KABLUNK_SCRIPTS_NATIVE_SCRIPT_COMPILER_LOGGER_H

#if KB_NATIVE_SCRIPTING

#include "RCCPP/RuntimeCompiler/ICompilerLogger.h"

namespace Kablunk
{

	class NativeScriptCompilerLogger : public ICompilerLogger
	{
		
		enum class LogLevel
		{
			INFO = 0,
			WARN,
			ERR
		};

		void LogError(const char* format, ...) override;
		void LogWarning(const char* format, ...) override;
		void LogInfo(const char* format, ...) override;
	public:
		NativeScriptCompilerLogger() { }

	private:
		template <typename... Args>
		void kbLog(LogLevel lvl, const char* format, Args&&... args)
		{
			// #TODO figure out how to use va_list with kablunk logging
			switch (lvl)
			{
			case LogLevel::INFO: vprintf(format, std::forward<Args>(args)...); break;//KB_CORE_INFO(format, std::forward<Args&&>(args)...); break;
				case LogLevel::WARN: vprintf(format, std::forward<Args>(args)...); break;//KB_CORE_WARN(format, std::forward<Args&&>(args)...); break;
				case LogLevel::ERR: vprintf(format, std::forward<Args>(args)...); break;//KB_CORE_ERROR(format, std::forward<Args&&>(args)...); break;
			}
		}
	};

}

#endif

#endif

//
// Basic instrumentation profiler by Cherno

// Usage: include this header file somewhere in your code (eg. precompiled header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session 
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use things like __FUNCSIG__ for the profile name.
//
#ifndef KABLUNK_DEBUG_INSTRUMENTOR_H
#define KABLUNK_DEBUG_INSTRUMENTOR_H

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

namespace Kablunk
{
	struct ProfileResult
	{
		std::string name;
		long long start, end;
		size_t thread_id;
	};

	struct InstrumentationSession
	{
		std::string name;
	};

	class Instrumentor
	{
	private:
		InstrumentationSession* m_current_session;
		std::ofstream m_output_stream;
		int m_profile_count;
	public:
		Instrumentor()
			: m_current_session{ nullptr }, m_profile_count{ 0 }
		{
		}

		Instrumentor(const Instrumentor&)	= delete;
		Instrumentor(Instrumentor&&)		= delete;

		~Instrumentor()
		{
			EndSession();
		}

		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			m_output_stream.open(filepath);
			WriteHeader();
			m_current_session = new InstrumentationSession{ name };
		}

		void EndSession()
		{
			WriteFooter();
			m_output_stream.close();
			delete m_current_session;
			m_current_session = nullptr;
			m_profile_count = 0;
		}

		void WriteProfile(const ProfileResult& result)
		{
			if (m_profile_count++ > 0)
				m_output_stream << ",";

			std::string name = result.name;
			std::replace(name.begin(), name.end(), '"', '\'');

			m_output_stream << "{";
			m_output_stream << "\"cat\":\"function\",";
			m_output_stream << "\"dur\":" << (result.end - result.start) << ',';
			m_output_stream << "\"name\":\"" << name << "\",";
			m_output_stream << "\"ph\":\"X\",";
			m_output_stream << "\"pid\":0,";
			m_output_stream << "\"tid\":" << result.thread_id << ",";
			m_output_stream << "\"ts\":" << result.start;
			m_output_stream << "}";

			m_output_stream.flush();
		}

		void WriteHeader()
		{
			m_output_stream << "{\"otherData\": {},\"traceEvents\":[";
			m_output_stream.flush();
		}

		void WriteFooter()
		{
			m_output_stream << "]}";
			m_output_stream.flush();
		}

		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}
	private:
	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name)
			: m_Name(name), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~InstrumentationTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
			Instrumentor::Get().WriteProfile({ m_Name, start, end, thread_id });

			m_Stopped = true;
		}
	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
		bool m_Stopped;
	};
}

#if 0
#if KB_PROFILE
	#define KB_BEGIN_SESSION(name, filepath) ::Kablunk::Instrumentor::Get().BeginSession(name, filepath)
	#define KB_END_SESSION()				 ::Kablunk::Instrumentor::Get().EndSession();
	#define KB_PROFILE_SCOPE_OLD(name)			 ::Kablunk::InstrumentationTimer timer##__LINE__(name);	
	#define KB_PROFILE_FUNCTION_OLD()			 KB_PROFILE_SCOPE_OLD(__FUNCSIG__)  
#else
	#define KB_BEGIN_SESSION(name, filepath)
	#define KB_END_SESSION()
	#define KB_PROFILE_FUNCTION()
	#define KB_PROFILE_SCOPE(name)
#endif
#endif

#endif

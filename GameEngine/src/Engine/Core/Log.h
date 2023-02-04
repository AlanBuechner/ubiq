#pragma once

#include "Core.h"
#include "iostream"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include <ctime>
#include <memory>
#include <utility>
#include <charconv>

namespace Engine
{
	class Logger
	{
	public:
		enum Level
		{
			Trace, Info, Warn, Error
		};

		Logger(const std::string& format) :
			m_Format(format)
		{}

		void SetLevel(Level level)
		{
			m_Level = level;
		}

		template<typename... Args>
		void Log(Level level, const char* msg, const Args&&... args)
		{
			if (level >= m_Level)
			{
				switch (level)
				{
				case Engine::Logger::Trace:
					std::cout << "\x1B[32m";
					break;
				case Engine::Logger::Info:
					std::cout << "\x1B[37m";
					break;
				case Engine::Logger::Warn:
					std::cout << "\x1B[33m";
					break;
				case Engine::Logger::Error:
					std::cout << "\x1B[31m";
					break;
				default:
					break;
				}
				uint32 findex = 0;
				while (m_Format[findex] != 0)
				{
					uint32 sindex = findex;
					while (m_Format[findex] != 0 && !CheckSpecial(findex))
						findex++;

					std::string_view s(&m_Format[sindex], findex - sindex);
					std::cout << s;

					findex++; // skip the %

					if (m_Format[findex] == 't')
					{
						time_t now = time(0);
						tm* ltm = localtime(&now);
						std::cout << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec;
					}
					else if (m_Format[findex] == 'm')
					{
						uint32 mfi = 0;
						while (msg[mfi] != 0)
						{
							uint32 msi = mfi;
							while (msg[mfi] != 0 && !checkFormat(msg, mfi))
								mfi++;

							std::string_view ms(&msg[msi], mfi - msi);
							std::cout << ms;

							if (msg[mfi] == '{')
							{
								mfi++;

								// get index
								uint32 ifi = mfi;
								while (msg[mfi] != 0 && isdigit(msg[mfi]))
									mfi++;

								std::string_view ds(&msg[ifi], mfi - ifi);
								uint32 ai = 0;
								std::from_chars(ds.data(), ds.data() + ds.size(), ai);

								mfi++;

								// print arg

								//std::cout << args[ai];
							}

						}
					}

					findex++;
				}

				std::cout << std::endl;

			}
		}

	private:
		bool CheckSpecial(uint32 findex)
		{
			uint32 lastIndex = findex-1;
			if (lastIndex < 0) lastIndex = 0;
			return (m_Format[findex] == '%' && m_Format[lastIndex] != '\\');
		}

		bool checkFormat(const char* msg, uint32 findex)
		{
			uint32 lastIndex = findex - 1;
			if (lastIndex < 0) lastIndex = 0;
			return (msg[findex] == '{' && msg[lastIndex] != '\\');
		}

	private:
		std::string m_Format;
		Level m_Level = Trace;
	};

	class ENGINE_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger;  }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;

	};
}

#define CORE_TRACE(...)		::Engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CORE_INFO(...)		::Engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_WARN(...)		::Engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_ERROR(...)		::Engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CORE_FATAL(...)		::Engine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define DEBUG_TRACE(...)	::Engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define DEBUG_INFO(...)		::Engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define DEBUG_WARN(...)		::Engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DEBUG_ERROR(...)	::Engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define DEBUG_FATAL(...)	::Engine::Log::GetClientLogger()->fatal(__VA_ARGS__)

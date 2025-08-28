#pragma once

#include "Utils/Common.h"
#include "Utils/Types.h"
#include "Utils/Vector.h"
#include <iostream>
#include <ctime>
#include <memory>
#include <utility>


#include <sstream>
#include "Utils/StringFormat.h"

namespace Engine
{
	class Logger
	{
	public:
		enum Level
		{
			Trace, Info, Warn, Error
		};

		struct PatternToken
		{
			enum TokenType
			{
				String, Time, Message
			};

			PatternToken(TokenType type, std::string_view string = "") :
				m_Type(type), m_String(string)
			{}

			TokenType m_Type;
			std::string_view m_String;

		};

		Logger() = default;
		Logger(const std::string& pattern);

		void SetPattern(const std::string& pattern);

		void SetLevel(Level level);

		template<typename... Args>
		void Log(Level level, const char* msg, const Args&... args)
		{
			if (level >= m_Level)
			{
				std::stringstream ss;
				for (PatternToken token : m_PatternTokens)
				{
					switch (token.m_Type)
					{
					case PatternToken::String:
						ss << token.m_String;
						break;
					case PatternToken::Message:
						Utils::StringFormat::Format(ss, msg, args...);
						break;
					case PatternToken::Time:
						time_t now = time(0);
						tm* ltm = localtime(&now);
						ss << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec;
						break;
					}
				}
				SetConsoleColor(level);
				std::string msg = ss.str();
				std::cout << msg << std::endl;

				uint32 tracyColors[] = { 0xffffff, 0x34eb40, 0xdeeb34, 0xeb3434 };
				TracyMessageC(msg.c_str(), msg.size(), tracyColors[(uint32)level]);
				SetConsoleColor(Level::Trace); // reset console window color

			}
		}

	private:
		bool CheckPatternSpecial(uint32 findex);


		void ParsePattern();

		

		void SetConsoleColor(Level level);

	private:
		std::string m_Pattern;
		Utils::Vector<PatternToken> m_PatternTokens;
		Level m_Level = Trace;
	};

	class  Log
	{
	public:
		static void Init();

		inline static Logger* GetCoreLogger() { return s_CoreLogger; }
		inline static Logger* GetClientLogger() { return s_ClientLogger; }

	private:
		static Logger* s_CoreLogger;
		static Logger* s_ClientLogger;

	};
}

#define CORE_TRACE(...)		::Engine::Log::GetCoreLogger()->Log(::Engine::Logger::Trace, __VA_ARGS__)
#define CORE_INFO(...)		::Engine::Log::GetCoreLogger()->Log(::Engine::Logger::Info, __VA_ARGS__)
#define CORE_WARN(...)		::Engine::Log::GetCoreLogger()->Log(::Engine::Logger::Warn, __VA_ARGS__)
#define CORE_ERROR(...)		::Engine::Log::GetCoreLogger()->Log(::Engine::Logger::Error, __VA_ARGS__)

#define DEBUG_TRACE(...)	::Engine::Log::GetClientLogger()->Log(::Engine::Logger::Trace, __VA_ARGS__)
#define DEBUG_INFO(...)		::Engine::Log::GetClientLogger()->Log(::Engine::Logger::Info, __VA_ARGS__)
#define DEBUG_WARN(...)		::Engine::Log::GetClientLogger()->Log(::Engine::Logger::Warn, __VA_ARGS__)
#define DEBUG_ERROR(...)	::Engine::Log::GetClientLogger()->Log(::Engine::Logger::Error, __VA_ARGS__)

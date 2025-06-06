#include "pch.h"
#include "Log.h"

#include "Engine/Core/Core.h"

Engine::Logger* Engine::Log::s_CoreLogger;
Engine::Logger* Engine::Log::s_ClientLogger;

namespace Engine
{

	Logger::Logger(const std::string& pattern) :
		m_Pattern(pattern)
	{
		ParsePattern();
	}

	void Logger::SetPattern(const std::string& pattern)
	{
		m_Pattern = pattern;
		ParsePattern();
	}

	void Logger::SetLevel(Level level)
	{
		m_Level = level;
	}

	bool Logger::CheckPatternSpecial(uint32 findex)
	{
		int lastIndex = findex - 1;
		if (lastIndex < 0) lastIndex = 0;
		return (m_Pattern[findex] == '%' && m_Pattern[lastIndex] != '\\');
	}

	bool Logger::CheckMessageSpecial(const char* msg, uint32 findex)
	{
		int lastIndex = findex - 1;
		if (lastIndex < 0) lastIndex = 0;
		return (msg[findex] == '{' && msg[lastIndex] != '\\');
	}

	void Logger::ParsePattern()
	{
		uint32 findex = 0;
		while (m_Pattern[findex] != 0)
		{
			uint32 sindex = findex;
			while (m_Pattern[findex] != 0 && !CheckPatternSpecial(findex))
				findex++;

			std::string_view s(&m_Pattern[sindex], findex - sindex);
			m_PatternTokens.Push(PatternToken(PatternToken::String, s));

			findex++; // skip the %

			if (m_Pattern[findex] == 't')
				m_PatternTokens.Push(PatternToken(PatternToken::Time));
			else if (m_Pattern[findex] == 'm')
				m_PatternTokens.Push(PatternToken(PatternToken::Message));

			findex++;
		}
	}

	bool Logger::ParseMessage(const char* msg, Utils::Vector<MessageToken>& tokens)
	{
		uint32 findex = 0;
		while (msg[findex] != 0)
		{
			uint32 sindex = findex;
			while (msg[findex] != 0 && !CheckMessageSpecial(msg, findex))
				findex++;

			std::string_view s(&msg[sindex], findex - sindex);
			tokens.Push(MessageToken(s));

			if (msg[findex] != 0)
			{
				findex++; // skip the {
				uint32 sindex = findex;
				while (msg[findex] != 0 && msg[findex] != '}')
					findex++;

				if (msg[findex] == 0)
				{
					std::cout << "closing \"}\" was not found" << std::endl;
					return false;
				}

				std::string_view indexString(&msg[sindex], findex - sindex);
				uint32 index;
				auto result = std::from_chars(indexString.data(), indexString.data() + indexString.size(), index);
				if (result.ec == std::errc::invalid_argument)
				{
					std::cout << "Could not convert to index \"" << indexString << "\"" << std::endl;
					return false;
				}

				tokens.Push(MessageToken(index));

				findex++; // skip the }
			}
		}
		return true;
	}

	void Logger::SetConsoleColor(Level level)
	{
#if defined(PLATFORM_WINDOWS)
		// map log levels to colors [white, green, yellow, red]
		uint32 colorCodes[] = { 15, 10, 14, 12 };
		static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, colorCodes[(uint32)level]);
#endif
	}

	void Log::Init()
	{
		s_CoreLogger = new Logger("[%t] ENGINE: %m");
		s_ClientLogger = new Logger("[%t] APP: %m");
	}
}

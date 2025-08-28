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

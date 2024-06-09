#pragma once

#include "Engine/Core/Core.h"
#include "iostream"
#include <ctime>
#include <memory>
#include <utility>
#include <charconv>
#include <type_traits>


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

		struct MessageToken
		{
			enum TokenType
			{
				String, Param
			};

			MessageToken(std::string_view string) :
				m_Type(String), m_String(string)
			{}
			MessageToken(uint32_t index) :
				m_Type(Param), m_ParamIndex(index)
			{}

			TokenType m_Type;
			std::string_view m_String;
			uint32_t m_ParamIndex;
		};

		Logger() = default;
		Logger(const std::string& pattern);

		void SetPattern(const std::string& pattern);

		void SetLevel(Level level);

		struct TemplateElement
		{
			enum Type {
				UINT8, UINT16, UINT32, UINT64,
				INT8, INT16, INT32, INT64,
				FLOAT32, FLOAT64,
				CHAR, CHARPTR, STRING,
			} type;

			void* data;

			TemplateElement(Type t, void* d) :
				type(t), data(d)
			{}

			template <class T>
			struct is_char_ptr
				: std::integral_constant<bool,
				std::is_pointer_v<T>
				&& std::is_same_v<char,
				typename std::remove_cv_t<std::remove_pointer_t<T>>>>
			{};

			template <class T>
			struct is_char_array
				: std::integral_constant<bool,
				std::is_array_v<T>
				&& std::is_same_v<char,
				typename std::remove_cv_t<std::remove_extent_t<T>>>>
			{};

			template<typename T>
			static TemplateElement GenTemplateElement(const T& data)
			{


#define CHECK_TYPE(t1, t2) if constexpr (std::is_same_v<T, t1>) return TemplateElement(t2, (void*)&data);
				CHECK_TYPE(uint8, UINT8);
				CHECK_TYPE(uint16, UINT16);
				CHECK_TYPE(uint32, UINT32);
				CHECK_TYPE(uint64, UINT64);

				CHECK_TYPE(int8, INT8);
				CHECK_TYPE(int16, INT16);
				CHECK_TYPE(int32, INT32);
				CHECK_TYPE(int64, INT64);

				CHECK_TYPE(float, FLOAT32);
				CHECK_TYPE(double, FLOAT64);

				CHECK_TYPE(char, CHAR);
				CHECK_TYPE(std::string, STRING);

				if constexpr (is_char_ptr<T>::value)
					return TemplateElement(CHARPTR, (void*)data);
				if constexpr (is_char_array<T>::value)
					return TemplateElement(CHARPTR, (void*)&data[0]);
#undef CHECK_TYPE

				return TemplateElement(INT32, (void*)&data);
			}

			void PrintValue()
			{
#define PRINT_NUMBER(t1, t2) case Engine::Logger::TemplateElement::t1: std::cout << std::to_string(*(t2*)data);break;
				switch (type)
				{
					PRINT_NUMBER(UINT8, uint8);
					PRINT_NUMBER(UINT16, uint16);
					PRINT_NUMBER(UINT32, uint32);
					PRINT_NUMBER(UINT64, uint64);

					PRINT_NUMBER(INT8, int8);
					PRINT_NUMBER(INT16, int16);
					PRINT_NUMBER(INT32, int32);
					PRINT_NUMBER(INT64, int64);

					PRINT_NUMBER(FLOAT32, float);
					PRINT_NUMBER(FLOAT64, double);

				case Engine::Logger::TemplateElement::CHAR:
					std::cout << *(char*)data; break;
				case Engine::Logger::TemplateElement::CHARPTR:
					std::cout << (char*)data; break;
				case Engine::Logger::TemplateElement::STRING:
					std::cout << *(std::string*)data; break;
				}
#undef PRINT_NUMBER
			}
		};

		template<typename... Args>
		void LogMessage(const char* msg, const Args&... args)
		{
			Utils::Vector<TemplateElement> templateElements;
			templateElements.Reserve(sizeof...(args));
			(templateElements.Push(TemplateElement::GenTemplateElement(args)), ...);

			Utils::Vector<MessageToken> tokens;
			if (ParseMessage(msg, tokens))
			{
				for (MessageToken token : tokens)
				{
					if (token.m_Type == MessageToken::String)
						std::cout << token.m_String;
					else if (token.m_Type == MessageToken::Param)
					{
						if (token.m_ParamIndex >= templateElements.Count())
						{
							std::cout << "invalid param index \"" << std::to_string(token.m_ParamIndex) << "\"" << std::endl;
							return;
						}
						templateElements[token.m_ParamIndex].PrintValue();
					}
				}
			}
		}

		template<typename... Args>
		void Log(Level level, const char* msg, const Args&... args)
		{
			if (level >= m_Level)
			{
				SetConsoleColor(level);

				for (PatternToken token : m_PatternTokens)
				{
					switch (token.m_Type)
					{
					case PatternToken::String:
						std::cout << token.m_String;
						break;
					case PatternToken::Message:
						LogMessage(msg, args...);
						break;
					case PatternToken::Time:
						time_t now = time(0);
						tm* ltm = localtime(&now);
						std::cout << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec;
						break;
					}
				}
				std::cout << std::endl;

			}
		}

	private:
		bool CheckPatternSpecial(uint32 findex);

		bool CheckMessageSpecial(const char* msg, uint32 findex);

		void ParsePattern();

		bool ParseMessage(const char* msg, Utils::Vector<MessageToken>& tokens);

		void SetConsoleColor(Level level);

	private:
		std::string m_Pattern;
		Utils::Vector<PatternToken> m_PatternTokens;
		Level m_Level = Trace;
	};

	class ENGINE_API Log
	{
	public:
		static void Init();

		inline static Ref<Logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static Ref<Logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static Ref<Logger> s_CoreLogger;
		static Ref<Logger> s_ClientLogger;

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

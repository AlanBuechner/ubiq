#pragma once
#include "UString.h"
#include "UFileIO.h"

namespace Engine
{
	class IFStream
	{
	public:
		IFStream();
		~IFStream();


		void Open(const UString& path);
		void Close();

		void ReadLine(UString& line);

		bool EndOfFile();

	private:
		UFileIO m_File;
		UString m_Data;

		uint32_t m_Offset = 0;
	};

	class OFStream
	{
	public:
		OFStream();
		~OFStream();

		void Open(const UString& path);
		void Close();

		void Write(const UString& data);
		void WriteLine(const UString& data);

		void Flush();

	private:
		UFileIO m_File;
		UString m_Data;
	};
}
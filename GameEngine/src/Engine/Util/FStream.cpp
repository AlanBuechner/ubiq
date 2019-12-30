#include "pch.h"
#include "FStream.h"

namespace Engine
{
	IFStream::IFStream()
	{
	}

	IFStream::~IFStream()
	{
	}

	void IFStream::Open(const UString& path)
	{
		m_File.Open(path);
		m_Data = m_File.ReadFromFile();
		DEBUG_INFO(m_Data);
	}

	void IFStream::Close()
	{
		m_File.Close();
	}

	void IFStream::ReadLine(UString& line)
	{
		int location = m_Data.Find('\n');
		if (location == -1)
		{
			location = m_Data.Size();
			line = m_Data.SubString(0, location);
			m_Data = m_Data.SubString(location, m_Data.Size());
		}
		else
		{
			line = m_Data.SubString(0, location);
			m_Data = m_Data.SubString(location + 1, m_Data.Size());
		}
	}

	bool IFStream::EndOfFile()
	{
		return m_Data.Size() == 0;
	}

}
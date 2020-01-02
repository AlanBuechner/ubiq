#include "pch.h"
#include "FStream.h"

namespace Engine
{
	IFStream::IFStream()
	{
	}

	IFStream::~IFStream()
	{
		Close();
	}

	void IFStream::Open(const UString& path)
	{
		m_File.Open(path);
		m_Data = m_File.ReadFromFile();
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
			location = (int)m_Data.Size();
			line = m_Data.SubString(0, location);
			m_Data = m_Data.SubString(location, (unsigned int)m_Data.Size());
		}
		else
		{
			line = m_Data.SubString(0, location);
			m_Data = m_Data.SubString(location + 1, (unsigned int)m_Data.Size());
		}
	}

	bool IFStream::EndOfFile()
	{
		return m_Data.Size() == 0;
	}

	/*------------------ OFStream --------------------*/

	OFStream::OFStream()
	{
	}

	OFStream::~OFStream()
	{
		Close();
	}

	void OFStream::Open(const UString& path)
	{
		m_File.Open(path, true);
	}

	void OFStream::Close()
	{
		if (m_File.IsOpen()) 
		{
			Flush();
			m_File.Close();
		}
	}

	void OFStream::Write(const UString& data)
	{
		m_Data += data;
	}

	void OFStream::WriteLine(const UString& data)
	{
		m_Data += data + '\n';
	}

	void OFStream::Flush()
	{
		m_File.WriteToFile(m_Data);
		m_Data = "";
	}

}
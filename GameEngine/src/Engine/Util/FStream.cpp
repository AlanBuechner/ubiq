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
		int location = m_Data.Find('\n', m_Offset+1);
		bool eof = false;
		if (location == -1)
		{
			location = (int)m_Data.Size();
			eof = true;
		}
		line = m_Data.SubString(m_Offset, location);
		location += (eof ? 0 : 1);
		m_Offset = location;
	}

	bool IFStream::EndOfFile()
	{
		return m_Offset >= m_Data.Size();
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
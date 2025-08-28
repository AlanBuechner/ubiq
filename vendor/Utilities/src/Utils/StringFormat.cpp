#include "StringFormat.h"
#include <charconv>


namespace Utils
{

	bool StringFormat::CheckMessageSpecial(const char* msg, uint32 findex)
	{
		int lastIndex = findex - 1;
		if (lastIndex < 0) lastIndex = 0;
		return (msg[findex] == '{' && msg[lastIndex] != '\\');
	}

	bool StringFormat::ParseMessage(const char* msg, Vector<MessageToken>& tokens)
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
					return false;

				std::string_view indexString(&msg[sindex], findex - sindex);
				uint32 index;
				std::from_chars_result result = std::from_chars(indexString.data(), indexString.data() + indexString.size(), index);
				if (result.ec == std::errc::invalid_argument)
					return false;

				tokens.Push(MessageToken(index));

				findex++; // skip the }
			}
		}
		return true;
	}

}


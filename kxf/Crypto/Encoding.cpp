#include "kxf-pch.h"
#include "Encoding.h"
#include "kxf/Core/String.h"
#include "kxf/Core/UniChar.h"
#include "kxf/Core/DataSize.h"
#include "kxf/IO/IStream.h"

#include <wx/base64.h>
#include "kxf/Win32/UndefMacros.h"

namespace
{
	constexpr size_t g_StreamBlockSize = kxf::DataSize::FromKB(64).ToBytes();
}

namespace kxf::Crypto
{
	size_t Rot13(String& data) noexcept
	{
		size_t count = 0;
		for (XChar& c: data)
		{
			if (UniChar(c).IsASCII())
			{
				if ((c >= 'A' && c <= 'M') || (c >= 'a' && c <= 'm'))
				{
					c += 13;
					count++;
				}
				else if ((c >= 'N' && c <= 'Z') || (c >= 'n' && c <= 'z'))
				{
					c -= 13;
					count++;
				}
			}
		}
		return count;
	}
	size_t UwUize(String& data) noexcept
	{
		size_t count = 0;
		for (XChar& c: data)
		{
			if (c == 'l' || c == 'r')
			{
				c = 'w';
				count++;
			}
			else if (c == 'L' || c == 'R')
			{
				c = 'W';
				count++;
			}
		}
		return count;
	}

	bool Base64Encode(IInputStream& inputStream, IOutputStream& outputStream)
	{
		const size_t size = wxBase64EncodedSize(g_StreamBlockSize);
		wxMemoryBuffer buffer(size);

		uint8_t readBuffer[g_StreamBlockSize] = {};
		while (inputStream.CanRead())
		{
			if (inputStream.Read(readBuffer, std::size(readBuffer)).LastRead() != 0)
			{
				buffer.AppendData(readBuffer, inputStream.LastRead().ToBytes());
			}
			else
			{
				break;
			}

			if (buffer.GetDataLen() == size)
			{
				auto utf8 = wxBase64Encode(buffer).ToUTF8();
				if (outputStream.WriteAll(utf8.data(), utf8.length()))
				{
					return true;
				}
				else
				{
					break;
				}
			}
		}
		return false;
	}
	bool Base64Decode(IInputStream& inputStream, IOutputStream& outputStream)
	{
		const size_t size = wxBase64DecodedSize(g_StreamBlockSize);
		wxMemoryBuffer buffer(size);

		uint8_t readBuffer[g_StreamBlockSize] = {};
		while (inputStream.CanRead())
		{
			if (inputStream.Read(readBuffer, std::size(readBuffer)).LastRead() != 0)
			{
				buffer.AppendData(readBuffer, inputStream.LastRead().ToBytes());
			}
			else
			{
				break;
			}

			if (buffer.GetDataLen() == size)
			{
				wxMemoryBuffer outBuffer = wxBase64Decode(reinterpret_cast<const char*>(buffer.GetData()), buffer.GetDataLen(), wxBase64DecodeMode_SkipWS);
				if (outputStream.WriteAll(outBuffer.GetData(), outBuffer.GetDataLen()))
				{
					return true;
				}
				else
				{
					break;
				}
			}
		}
		return false;
	}
}

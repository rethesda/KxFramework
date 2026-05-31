#include "kxf-pch.h"
#include "IWebResponse.h"
#include "WebRequestHeader.h"
#include "kxf/Core/RegEx.h"
#include "kxf/IO/NullStream.h"
#include "kxf/IO/StreamReaderWriter.h"
#include "kxf/FileSystem/FSPath.h"
#include "kxf/Serialization/XML.h"
#include "kxf/Serialization/JSON.h"

namespace kxf
{
	FSPath IWebResponse::GetSuggestedFilePath() const
	{
		if (String contentDisposition = GetHeader("Content-Disposition"); !contentDisposition.IsEmpty())
		{
			if (RegEx regEx("filename=\"(.+)\"", RegExFlag::IgnoreCase); regEx.Matches(contentDisposition))
			{
				return regEx.GetMatch(contentDisposition, 1);
			}
		}

		if (URI uri = GetURI())
		{
			if (uri.HasPath())
			{
				return uri.GetPath();
			}
			return uri.GetServer();
		}
		return {};
	}

	String IWebResponse::GetAsString() const
	{
		if (auto stream = GetStream())
		{
			IO::InputStreamReader reader(*stream);
			return reader.ReadStringUTF8(stream->GetSize().ToBytes());
		}
		return {};
	}
	JSONDocument IWebResponse::GetAsJSON() const
	{
		if (auto stream = GetStream())
		{
			JSONDocument document;
			if (document.LoadDocument(*stream))
			{
				return document;
			}
		}
		return {};
	}
	XMLDocument IWebResponse::GetAsXML() const
	{
		if (auto stream = GetStream())
		{
			XMLDocument document;
			if (document.LoadDocument(*stream))
			{
				return document;
			}
		}
		return {};
	}
}

namespace kxf
{
	FSPath NullWebResponse::GetSuggestedFilePath() const
	{
		return {};
	}
	std::shared_ptr<IInputStream> NullWebResponse::GetStream() const
	{
		return nullptr;
	}
}

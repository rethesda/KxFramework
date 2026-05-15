#include "kxf-pch.h"
#include "XDocument.h"

namespace kxf::XDocument
{
	std::pair<StringView, int> ExtractIndexFromElementName(StringView elementName, XChar indexSeparator)
	{
		XChar sep[] = {indexSeparator, 0};
		return ExtractIndexFromElementName(elementName, sep);
	}
	std::pair<StringView, int> ExtractIndexFromElementName(StringView elementName, StringView indexSeparator)
	{
		int index = 0;

		size_t indexStart = elementName.find(indexSeparator);
		if (indexStart != String::npos)
		{
			if (auto value = String(elementName.substr(indexStart + indexSeparator.length())).ParseInteger<int>())
			{
				index = std::clamp(*value, 0, std::numeric_limits<int>::max());
				elementName = elementName.substr(0, indexStart);
			}
		}
		return {elementName, index};
	}
}

#include "kxf-pch.h"
#include "ShellFileTypeInfo.h"
#include "ShellFileTypeManager.h"
#include <wx/mimetype.h>
#include <wx/iconloc.h>

namespace kxf
{
	ShellFileTypeInfo::ShellFileTypeInfo(const String& mimeType)
		:m_FileTypeInfo(std::make_unique<wxFileTypeInfo>(mimeType))
	{
	}
	ShellFileTypeInfo::ShellFileTypeInfo(const String& mimeType, const String& openCommand, const String& printCommand, const String& description, const String& extension)
		:m_FileTypeInfo(std::make_unique<wxFileTypeInfo>(mimeType))
	{
		m_FileTypeInfo->SetOpenCommand(openCommand);
		m_FileTypeInfo->SetPrintCommand(printCommand);
		m_FileTypeInfo->SetDescription(description);

		AddExtension(extension);
	}
	ShellFileTypeInfo::ShellFileTypeInfo(const wxFileTypeInfo& other)
		:m_FileTypeInfo(std::make_unique<wxFileTypeInfo>(other))
	{
	}
	ShellFileTypeInfo::~ShellFileTypeInfo() = default;

	String ShellFileTypeInfo::GetDescription() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetDescription();
		}
		return {};
	}
	ShellFileTypeInfo& ShellFileTypeInfo::SetDescription(const String& value)
	{
		if (m_FileTypeInfo)
		{
			m_FileTypeInfo->SetDescription(value);
		}
		return *this;
	}

	String ShellFileTypeInfo::GetShortDescription() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetShortDesc();
		}
		return {};
	}
	ShellFileTypeInfo& ShellFileTypeInfo::SetShortDescription(const String& value)
	{
		if (m_FileTypeInfo)
		{
			m_FileTypeInfo->SetShortDesc(value);
		}
		return *this;
	}

	String ShellFileTypeInfo::GetMimeType() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetMimeType();
		}
		return {};
	}

	String ShellFileTypeInfo::GetOpenCommand() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetOpenCommand();
		}
		return {};
	}
	ShellFileTypeInfo& ShellFileTypeInfo::SetOpenCommand(const String& value)
	{
		if (m_FileTypeInfo)
		{
			m_FileTypeInfo->SetOpenCommand(value);
		}
		return *this;
	}

	String ShellFileTypeInfo::GetPrintCommand() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetPrintCommand();
		}
		return {};
	}
	ShellFileTypeInfo& ShellFileTypeInfo::SetPrintCommand(const String& value)
	{
		if (m_FileTypeInfo)
		{
			m_FileTypeInfo->SetPrintCommand(value);
		}
		return *this;
	}

	size_t ShellFileTypeInfo::GetExtensionsCount() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetExtensionsCount();
		}
		return 0;
	}
	CallbackResult<size_t> ShellFileTypeInfo::EnumExtensions(CallbackFunction<String> func) const
	{
		if (m_FileTypeInfo)
		{
			for (const auto& value: m_FileTypeInfo->GetExtensions())
			{
				if (func.Invoke(value).ShouldTerminate())
				{
					break;
				}
			}
			return func.Finalize(m_FileTypeInfo->GetExtensionsCount());
		}
		return {};
	}
	ShellFileTypeInfo& ShellFileTypeInfo::AddExtension(const String& extension, bool isURLProtocol)
	{
		if (m_FileTypeInfo)
		{
			m_FileTypeInfo->AddExtension(extension);
			m_URLProtocolMap.insert_or_assign(extension, isURLProtocol);
		}
		return *this;
	}

	int ShellFileTypeInfo::GetIconIndex() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetIconIndex();
		}
		return -1;
	}
	String ShellFileTypeInfo::GetIconFile() const
	{
		if (m_FileTypeInfo)
		{
			return m_FileTypeInfo->GetIconFile();
		}
		return {};
	}
	ShellFileTypeInfo& ShellFileTypeInfo::SetIcon(const String& filePath, int index)
	{
		if (m_FileTypeInfo)
		{
			m_FileTypeInfo->SetIcon(filePath, index);
		}
		return *this;
	}
	bool ShellFileTypeInfo::IsURLProtocol(const String& extension) const
	{
		if (m_FileTypeInfo)
		{
			auto it = m_URLProtocolMap.find(extension);
			if (it != m_URLProtocolMap.end())
			{
				return it->second;
			}
		}
		return false;
	}
	ShellFileTypeInfo& ShellFileTypeInfo::SetURLProtocol(const String& extension, bool protocol)
	{
		if (m_FileTypeInfo)
		{
			m_URLProtocolMap.insert_or_assign(extension, protocol);
		}
		return *this;
	}
}

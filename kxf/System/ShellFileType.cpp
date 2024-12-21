#include "kxf-pch.h"
#include "ShellFileType.h"
#include "ShellFileTypeManager.h"
#include "Registry.h"
#include "kxf/Core/RegEx.h"
#include <wx/mimetype.h>
#include <wx/iconloc.h>

namespace
{
	wxFileType::MessageParameters ToWXMessageParameters(const kxf::ShellFileType::MessageParameters& parameters)
	{
		return wxFileType::MessageParameters(parameters.GetFileName(), parameters.GetMIMEType());
	}
	std::vector<kxf::String> ToVector(const wxArrayString& items)
	{
		return {items.begin(), items.end()};
	}
}

namespace kxf
{
	String ShellFileType::ExpandCommand(const String& command, const MessageParameters& parameters)
	{
		return wxFileType::ExpandCommand(command, ToWXMessageParameters(parameters));
	}

	ShellFileType::ShellFileType(wxFileType* fileType)
		:m_FileType(fileType)
	{
	}
	ShellFileType::ShellFileType(const ShellFileTypeInfo& typeInfo)
	{
		if (typeInfo)
		{
			m_FileType = std::make_unique<wxFileType>(typeInfo.AsWXFileTypeInfo());
		}
	}
	ShellFileType::~ShellFileType() = default;

	String ShellFileType::GetDescription() const
	{
		if (m_FileType)
		{
			wxString value;
			m_FileType->GetDescription(&value);

			return value;
		}
		return {};
	}
	String ShellFileType::GetExtension() const
	{
		if (m_FileType)
		{
			wxArrayString extensions;
			if (m_FileType->GetExtensions(extensions))
			{
				return extensions.front();
			}
		}
		return {};
	}
	std::vector<String> ShellFileType::GetAllExtensions() const
	{
		if (m_FileType)
		{
			wxArrayString extensions;
			m_FileType->GetExtensions(extensions);

			return ToVector(extensions);
		}
		return {};
	}

	String ShellFileType::GetMimeType() const
	{
		if (m_FileType)
		{
			wxString value;
			m_FileType->GetMimeType(&value);

			return value;
		}
		return {};
	}
	std::vector<String> ShellFileType::GetAllMimeTypes() const
	{
		if (m_FileType)
		{
			wxArrayString mimeTypes;
			m_FileType->GetMimeTypes(mimeTypes);

			return ToVector(mimeTypes);
		}
		return {};
	}

	ShellFileType::IconLocation ShellFileType::GetIcon() const
	{
		if (m_FileType)
		{
			wxIconLocation icon;
			if (m_FileType->GetIcon(&icon))
			{
				return {icon.GetFileName(), icon.GetIndex()};
			}
		}
		return {};
	}
	ShellFileType::IconLocation ShellFileType::GetIcon(const MessageParameters& parameters) const
	{
		if (m_FileType)
		{
			wxIconLocation icon;
			if (m_FileType->GetIcon(&icon, ToWXMessageParameters(parameters)))
			{
				return {icon.GetFileName(), icon.GetIndex()};
			}
		}
		return {};
	}

	String ShellFileType::GetCommand(const String& action, const String& filePath) const
	{
		if (m_FileType)
		{
			String value;
			m_FileType->GetExpandedCommand(action, wxFileType::MessageParameters(filePath));

			return value;
		}
		return {};
	}
	String ShellFileType::GetCommand(const String& action, const MessageParameters& parameters) const
	{
		if (m_FileType)
		{
			String value;
			m_FileType->GetExpandedCommand(action, ToWXMessageParameters(parameters));

			return value;
		}
		return {};
	}

	String ShellFileType::GetOpenCommand(const String& filePath) const
	{
		if (m_FileType)
		{
			wxString value;
			m_FileType->GetOpenCommand(&value, wxFileType::MessageParameters(filePath));

			return value;
		}
		return {};
	}
	String ShellFileType::GetOpenCommand(const MessageParameters& parameters) const
	{
		if (m_FileType)
		{
			wxString value;
			m_FileType->GetOpenCommand(&value, ToWXMessageParameters(parameters));

			return value;
		}
		return {};
	}

	String ShellFileType::GetOpenExecutable() const
	{
		if (m_FileType)
		{
			String openCommand = GetOpenCommand(NullString);
			if (RegEx regEx(u8R"(\"(.+?)\")", RegExFlag::IgnoreCase); regEx.Matches(openCommand))
			{
				return regEx.GetMatch(openCommand, 1);
			}
		}
		return {};
	}

	String ShellFileType::GetPrintCommand(const String& filePath) const
	{
		if (m_FileType)
		{
			wxString value;
			m_FileType->GetPrintCommand(&value, wxFileType::MessageParameters(filePath));

			return value;
		}
		return {};
	}
	String ShellFileType::GetPrintCommand(const MessageParameters& parameters) const
	{
		if (m_FileType)
		{
			wxString value;
			m_FileType->GetPrintCommand(&value, ToWXMessageParameters(parameters));

			return value;
		}
		return {};
	}

	size_t ShellFileType::GetAllCommands(std::vector<String>& verbs, std::vector<String>& commands, const MessageParameters& parameters) const
	{
		if (m_FileType)
		{
			wxArrayString verbsWX;
			wxArrayString commandsWX;
			size_t count = m_FileType->GetAllCommands(&verbsWX, &commandsWX, ToWXMessageParameters(parameters));

			if (count != 0)
			{
				verbs = ToVector(verbsWX);
				commands = ToVector(commandsWX);

				return count;
			}
		}
		return 0;
	}
	bool ShellFileType::IsURLProtocol(const String& extension) const
	{
		if (m_FileType && !extension.IsEmpty())
		{
			for (const String& ext: GetAllExtensions())
			{
				if (ext == extension)
				{
					RegistryKey key(RegistryRootKey::ClassesRoot, extension, RegistryAccess::Read);
					return key.DoesValueExist("URL Protocol");
				}
			}
		}
		return false;
	}
}

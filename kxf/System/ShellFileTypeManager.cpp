#include "kxf-pch.h"
#include "ShellFileTypeManager.h"
#include "Registry.h"
#include "kxf/Utility/Common.h"
#include "kxf/Utility/String.h"
#include <wx/mimetype.h>
#include <wx/iconloc.h>

namespace kxf
{
	bool ShellFileTypeManager::IsOfType(const String& mimeType, const String& wildcard)
	{
		return wxMimeTypesManager::IsOfType(mimeType, wildcard);
	}

	ShellFileTypeManager::ShellFileTypeManager()
		:m_Manager(std::make_unique<wxMimeTypesManager>())
	{
	}
	ShellFileTypeManager::~ShellFileTypeManager() = default;

	ShellFileType ShellFileTypeManager::FileTypeFromExtension(const String& extension) const
	{
		return m_Manager->GetFileTypeFromExtension(extension);
	}
	ShellFileType ShellFileTypeManager::FileTypeFromMimeType(const String& mimeType) const
	{
		return m_Manager->GetFileTypeFromMimeType(mimeType);
	}

	ShellFileType ShellFileTypeManager::Associate(const ShellFileTypeInfo& fileTypeInfo)
	{
		if (!fileTypeInfo)
		{
			return {};
		}

		if (wxFileType* fileType = m_Manager->Associate(fileTypeInfo.AsWXFileTypeInfo()))
		{
			fileTypeInfo.EnumExtensions([&](String extension)
			{
				if (fileTypeInfo.IsURLProtocol(extension))
				{
					RegistryKey classesRoot(RegistryRootKey::ClassesRoot, {}, RegistryAccess::Create);
					if (classesRoot)
					{
						String ext = FSPath(std::move(extension)).GetExtension();

						RegistryKey key = classesRoot.CreateKey(ext, RegistryAccess::Write);
						key.SetStringValue({}, Format("URL:{} Protocol", ext.MakeUpper()));
						key.SetStringValue("URL Protocol", {});
					}
				}
			});
			return fileType;
		}
		return nullptr;
	}
	bool ShellFileTypeManager::IsAssociatedWith(const ShellFileType& fileType, const String& executablePath) const
	{
		if (fileType)
		{
			return fileType.GetOpenExecutable() == executablePath;
		}
		return false;
	}
	bool ShellFileTypeManager::Unassociate(ShellFileType& fileType)
	{
		if (fileType)
		{
			return m_Manager->Unassociate(&fileType.AsWXFileType());
		}
		return false;
	}
	void ShellFileTypeManager::AddFallback(const ShellFileTypeInfo& fileTypeInfo)
	{
		m_Manager->AddFallback(fileTypeInfo.AsWXFileTypeInfo());
	}

	CallbackResult<size_t> ShellFileTypeManager::EnumFileTypes(CallbackFunction<String> func) const
	{
		wxArrayString mimeTypes;
		size_t count = m_Manager->EnumAllFileTypes(mimeTypes);

		for (auto& mime: mimeTypes)
		{
			if (func.Invoke(mime).ShouldTerminate())
			{
				break;
			}
		}
		return func.Finalize(count);
	}
}
